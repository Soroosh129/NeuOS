#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <sstream>
#include "hdf5.h"
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>


#include "caffe/common.hpp"
#include "caffe/layer.hpp"
#include "caffe/net.hpp"
#include "caffe/parallel.hpp"
#include "caffe/proto/caffe.pb.h"
#include "caffe/util/hdf5.hpp"
#include "caffe/util/insert_splits.hpp"
#include "caffe/util/math_functions.hpp"
#include "caffe/util/upgrade_proto.hpp"
#include <energymon/energymon-odroid.h>
#include <energymon/energymon-time-util.h>

// DVFS includes
#include "triangle/schedule.h"
#include "DVFS/setconfig.h"
#include <time.h>       /* time */

#define SHM_SIZE 10
#define DEADLINE 30

//#define MAX_ACC
//#define MIN_ENG

using namespace std;
namespace caffe {

	template <typename Dtype>
		Net<Dtype>::Net(const NetParameter& param) {
			Init(param);
		}

	template <typename Dtype>
		Net<Dtype>::Net(const string& param_file, Phase phase,
				const int level, const vector<string>* stages) {
			NetParameter param;
			ReadNetParamsFromTextFileOrDie(param_file, &param);
			// Set phase, stages and level
			param.mutable_state()->set_phase(phase);
			if (stages != NULL) {
				for (int i = 0; i < stages->size(); i++) {
					param.mutable_state()->add_stage((*stages)[i]);
				}
			}
			param.mutable_state()->set_level(level);
			Init(param);
		}

	template <typename Dtype>
		void Net<Dtype>::Init(const NetParameter& in_param) {
			// Set phase from the state.
			phase_ = in_param.state().phase();
			// Filter layers based on their include/exclude rules and
			// the current NetState.
			NetParameter filtered_param;
			FilterNet(in_param, &filtered_param);
			LOG_IF(INFO, Caffe::root_solver())
				<< "Initializing net from parameters: " << std::endl
				<< filtered_param.DebugString();
			// Create a copy of filtered_param with splits added where necessary.
			NetParameter param;
			InsertSplits(filtered_param, &param);
			// Basically, build all the layers and set up their connections.
			name_ = param.name();
			map<string, int> blob_name_to_idx;
			set<string> available_blobs;
			memory_used_ = 0;
			// For each layer, set up its input and output
			bottom_vecs_.resize(param.layer_size());
			top_vecs_.resize(param.layer_size());
			bottom_id_vecs_.resize(param.layer_size());
			param_id_vecs_.resize(param.layer_size());
			top_id_vecs_.resize(param.layer_size());
			bottom_need_backward_.resize(param.layer_size());
			for (int layer_id = 0; layer_id < param.layer_size(); ++layer_id) {
				// Inherit phase from net if unset.
				if (!param.layer(layer_id).has_phase()) {
					param.mutable_layer(layer_id)->set_phase(phase_);
				}
				// Setup layer.
				const LayerParameter& layer_param = param.layer(layer_id);
				if (layer_param.propagate_down_size() > 0) {
					CHECK_EQ(layer_param.propagate_down_size(),
							layer_param.bottom_size())
						<< "propagate_down param must be specified "
						<< "either 0 or bottom_size times ";
				}
				layers_.push_back(LayerRegistry<Dtype>::CreateLayer(layer_param));
				layer_names_.push_back(layer_param.name());
				LOG_IF(INFO, Caffe::root_solver())
					<< "Creating Layer " << layer_param.name();
				bool need_backward = false;

				// Figure out this layer's input and output
				for (int bottom_id = 0; bottom_id < layer_param.bottom_size();
						++bottom_id) {
					const int blob_id = AppendBottom(param, layer_id, bottom_id,
							&available_blobs, &blob_name_to_idx);
					// If a blob needs backward, this layer should provide it.
					need_backward |= blob_need_backward_[blob_id];
				}
				int num_top = layer_param.top_size();
				for (int top_id = 0; top_id < num_top; ++top_id) {
					AppendTop(param, layer_id, top_id, &available_blobs, &blob_name_to_idx);
					// Collect Input layer tops as Net inputs.
					if (layer_param.type() == "Input") {
						const int blob_id = blobs_.size() - 1;
						net_input_blob_indices_.push_back(blob_id);
						net_input_blobs_.push_back(blobs_[blob_id].get());
					}
				}
				// If the layer specifies that AutoTopBlobs() -> true and the LayerParameter
				// specified fewer than the required number (as specified by
				// ExactNumTopBlobs() or MinTopBlobs()), allocate them here.
				Layer<Dtype>* layer = layers_[layer_id].get();
				if (layer->AutoTopBlobs()) {
					const int needed_num_top =
						std::max(layer->MinTopBlobs(), layer->ExactNumTopBlobs());
					for (; num_top < needed_num_top; ++num_top) {
						// Add "anonymous" top blobs -- do not modify available_blobs or
						// blob_name_to_idx as we don't want these blobs to be usable as input
						// to other layers.
						AppendTop(param, layer_id, num_top, NULL, NULL);
					}
				}
				// After this layer is connected, set it up.
				layers_[layer_id]->SetUp(bottom_vecs_[layer_id], top_vecs_[layer_id]);
				LOG_IF(INFO, Caffe::root_solver())
					<< "Setting up " << layer_names_[layer_id];
				for (int top_id = 0; top_id < top_vecs_[layer_id].size(); ++top_id) {
					if (blob_loss_weights_.size() <= top_id_vecs_[layer_id][top_id]) {
						blob_loss_weights_.resize(top_id_vecs_[layer_id][top_id] + 1, Dtype(0));
					}
					blob_loss_weights_[top_id_vecs_[layer_id][top_id]] = layer->loss(top_id);
					LOG_IF(INFO, Caffe::root_solver())
						<< "Top shape: " << top_vecs_[layer_id][top_id]->shape_string();
					if (layer->loss(top_id)) {
						LOG_IF(INFO, Caffe::root_solver())
							<< "    with loss weight " << layer->loss(top_id);
					}
					memory_used_ += top_vecs_[layer_id][top_id]->count();
				}
				LOG_IF(INFO, Caffe::root_solver())
					<< "Memory required for data: " << memory_used_ * sizeof(Dtype);
				const int param_size = layer_param.param_size();
				const int num_param_blobs = layers_[layer_id]->blobs().size();
				CHECK_LE(param_size, num_param_blobs)
					<< "Too many params specified for layer " << layer_param.name();
				ParamSpec default_param_spec;
				for (int param_id = 0; param_id < num_param_blobs; ++param_id) {
					const ParamSpec* param_spec = (param_id < param_size) ?
						&layer_param.param(param_id) : &default_param_spec;
					const bool param_need_backward = param_spec->lr_mult() != 0;
					need_backward |= param_need_backward;
					layers_[layer_id]->set_param_propagate_down(param_id,
							param_need_backward);
				}
				for (int param_id = 0; param_id < num_param_blobs; ++param_id) {
					AppendParam(param, layer_id, param_id);
				}
				// Finally, set the backward flag
				layer_need_backward_.push_back(need_backward);
				if (need_backward) {
					for (int top_id = 0; top_id < top_id_vecs_[layer_id].size(); ++top_id) {
						blob_need_backward_[top_id_vecs_[layer_id][top_id]] = true;
					}
				}
			}
			// Go through the net backwards to determine which blobs contribute to the
			// loss.  We can skip backward computation for blobs that don't contribute
			// to the loss.
			// Also checks if all bottom blobs don't need backward computation (possible
			// because the skip_propagate_down param) and so we can skip bacward
			// computation for the entire layer
			set<string> blobs_under_loss;
			set<string> blobs_skip_backp;
			for (int layer_id = layers_.size() - 1; layer_id >= 0; --layer_id) {
				bool layer_contributes_loss = false;
				bool layer_skip_propagate_down = true;
				for (int top_id = 0; top_id < top_vecs_[layer_id].size(); ++top_id) {
					const string& blob_name = blob_names_[top_id_vecs_[layer_id][top_id]];
					if (layers_[layer_id]->loss(top_id) ||
							(blobs_under_loss.find(blob_name) != blobs_under_loss.end())) {
						layer_contributes_loss = true;
					}
					if (blobs_skip_backp.find(blob_name) == blobs_skip_backp.end()) {
						layer_skip_propagate_down = false;
					}
					if (layer_contributes_loss && !layer_skip_propagate_down)
						break;
				}
				// If this layer can skip backward computation, also all his bottom blobs
				// don't need backpropagation
				if (layer_need_backward_[layer_id] && layer_skip_propagate_down) {
					layer_need_backward_[layer_id] = false;
					for (int bottom_id = 0; bottom_id < bottom_vecs_[layer_id].size();
							++bottom_id) {
						bottom_need_backward_[layer_id][bottom_id] = false;
					}
				}
				if (!layer_contributes_loss) { layer_need_backward_[layer_id] = false; }
				if (Caffe::root_solver()) {
					if (layer_need_backward_[layer_id]) {
						LOG(INFO) << layer_names_[layer_id] << " needs backward computation.";
					} else {
						LOG(INFO) << layer_names_[layer_id]
							<< " does not need backward computation.";
					}
				}
				for (int bottom_id = 0; bottom_id < bottom_vecs_[layer_id].size();
						++bottom_id) {
					if (layer_contributes_loss) {
						const string& blob_name =
							blob_names_[bottom_id_vecs_[layer_id][bottom_id]];
						blobs_under_loss.insert(blob_name);
					} else {
						bottom_need_backward_[layer_id][bottom_id] = false;
					}
					if (!bottom_need_backward_[layer_id][bottom_id]) {
						const string& blob_name =
							blob_names_[bottom_id_vecs_[layer_id][bottom_id]];
						blobs_skip_backp.insert(blob_name);
					}
				}
			}
			// Handle force_backward if needed.
			if (param.force_backward()) {
				for (int layer_id = 0; layer_id < layers_.size(); ++layer_id) {
					layer_need_backward_[layer_id] = true;
					for (int bottom_id = 0;
							bottom_id < bottom_need_backward_[layer_id].size(); ++bottom_id) {
						bottom_need_backward_[layer_id][bottom_id] =
							bottom_need_backward_[layer_id][bottom_id] ||
							layers_[layer_id]->AllowForceBackward(bottom_id);
						blob_need_backward_[bottom_id_vecs_[layer_id][bottom_id]] =
							blob_need_backward_[bottom_id_vecs_[layer_id][bottom_id]] ||
							bottom_need_backward_[layer_id][bottom_id];
					}
					for (int param_id = 0; param_id < layers_[layer_id]->blobs().size();
							++param_id) {
						layers_[layer_id]->set_param_propagate_down(param_id, true);
					}
				}
			}
			// In the end, all remaining blobs are considered output blobs.
			for (set<string>::iterator it = available_blobs.begin();
					it != available_blobs.end(); ++it) {
				LOG_IF(INFO, Caffe::root_solver())
					<< "This network produces output " << *it;
				net_output_blobs_.push_back(blobs_[blob_name_to_idx[*it]].get());
				net_output_blob_indices_.push_back(blob_name_to_idx[*it]);
			}
			for (size_t blob_id = 0; blob_id < blob_names_.size(); ++blob_id) {
				blob_names_index_[blob_names_[blob_id]] = blob_id;
			}
			for (size_t layer_id = 0; layer_id < layer_names_.size(); ++layer_id) {
				layer_names_index_[layer_names_[layer_id]] = layer_id;
			}
			ShareWeights();
			debug_info_ = param.debug_info();
			LOG_IF(INFO, Caffe::root_solver()) << "Network initialization done.";
		}

	template <typename Dtype>
		void Net<Dtype>::FilterNet(const NetParameter& param,
				NetParameter* param_filtered) {
			NetState net_state(param.state());
			param_filtered->CopyFrom(param);
			param_filtered->clear_layer();
			for (int i = 0; i < param.layer_size(); ++i) {
				const LayerParameter& layer_param = param.layer(i);
				const string& layer_name = layer_param.name();
				CHECK(layer_param.include_size() == 0 || layer_param.exclude_size() == 0)
					<< "Specify either include rules or exclude rules; not both.";
				// If no include rules are specified, the layer is included by default and
				// only excluded if it meets one of the exclude rules.
				bool layer_included = (layer_param.include_size() == 0);
				for (int j = 0; layer_included && j < layer_param.exclude_size(); ++j) {
					if (StateMeetsRule(net_state, layer_param.exclude(j), layer_name)) {
						layer_included = false;
					}
				}
				for (int j = 0; !layer_included && j < layer_param.include_size(); ++j) {
					if (StateMeetsRule(net_state, layer_param.include(j), layer_name)) {
						layer_included = true;
					}
				}
				if (layer_included) {
					param_filtered->add_layer()->CopyFrom(layer_param);
				}
			}
		}

	template <typename Dtype>
		bool Net<Dtype>::StateMeetsRule(const NetState& state,
				const NetStateRule& rule, const string& layer_name) {
			// Check whether the rule is broken due to phase.
			if (rule.has_phase()) {
				if (rule.phase() != state.phase()) {
					LOG_IF(INFO, Caffe::root_solver())
						<< "The NetState phase (" << state.phase()
						<< ") differed from the phase (" << rule.phase()
						<< ") specified by a rule in layer " << layer_name;
					return false;
				}
			}
			// Check whether the rule is broken due to min level.
			if (rule.has_min_level()) {
				if (state.level() < rule.min_level()) {
					LOG_IF(INFO, Caffe::root_solver())
						<< "The NetState level (" << state.level()
						<< ") is above the min_level (" << rule.min_level()
						<< ") specified by a rule in layer " << layer_name;
					return false;
				}
			}
			// Check whether the rule is broken due to max level.
			if (rule.has_max_level()) {
				if (state.level() > rule.max_level()) {
					LOG_IF(INFO, Caffe::root_solver())
						<< "The NetState level (" << state.level()
						<< ") is above the max_level (" << rule.max_level()
						<< ") specified by a rule in layer " << layer_name;
					return false;
				}
			}
			// Check whether the rule is broken due to stage. The NetState must
			// contain ALL of the rule's stages to meet it.
			for (int i = 0; i < rule.stage_size(); ++i) {
				// Check that the NetState contains the rule's ith stage.
				bool has_stage = false;
				for (int j = 0; !has_stage && j < state.stage_size(); ++j) {
					if (rule.stage(i) == state.stage(j)) { has_stage = true; }
				}
				if (!has_stage) {
					LOG_IF(INFO, Caffe::root_solver())
						<< "The NetState did not contain stage '" << rule.stage(i)
						<< "' specified by a rule in layer " << layer_name;
					return false;
				}
			}
			// Check whether the rule is broken due to not_stage. The NetState must
			// contain NONE of the rule's not_stages to meet it.
			for (int i = 0; i < rule.not_stage_size(); ++i) {
				// Check that the NetState contains the rule's ith not_stage.
				bool has_stage = false;
				for (int j = 0; !has_stage && j < state.stage_size(); ++j) {
					if (rule.not_stage(i) == state.stage(j)) { has_stage = true; }
				}
				if (has_stage) {
					LOG_IF(INFO, Caffe::root_solver())
						<< "The NetState contained a not_stage '" << rule.not_stage(i)
						<< "' specified by a rule in layer " << layer_name;
					return false;
				}
			}
			return true;
		}

	// Helper for Net::Init: add a new top blob to the net.
	template <typename Dtype>
		void Net<Dtype>::AppendTop(const NetParameter& param, const int layer_id,
				const int top_id, set<string>* available_blobs,
				map<string, int>* blob_name_to_idx) {
			shared_ptr<LayerParameter> layer_param(
					new LayerParameter(param.layer(layer_id)));
			const string& blob_name = (layer_param->top_size() > top_id) ?
				layer_param->top(top_id) : "(automatic)";
			// Check if we are doing in-place computation
			if (blob_name_to_idx && layer_param->bottom_size() > top_id &&
					blob_name == layer_param->bottom(top_id)) {
				// In-place computation
				LOG_IF(INFO, Caffe::root_solver())
					<< layer_param->name() << " -> " << blob_name << " (in-place)";
				top_vecs_[layer_id].push_back(blobs_[(*blob_name_to_idx)[blob_name]].get());
				top_id_vecs_[layer_id].push_back((*blob_name_to_idx)[blob_name]);
			} else if (blob_name_to_idx &&
					blob_name_to_idx->find(blob_name) != blob_name_to_idx->end()) {
				// If we are not doing in-place computation but have duplicated blobs,
				// raise an error.
				LOG(FATAL) << "Top blob '" << blob_name
					<< "' produced by multiple sources.";
			} else {
				// Normal output.
				if (Caffe::root_solver()) {
					LOG(INFO) << layer_param->name() << " -> " << blob_name;
				}
				shared_ptr<Blob<Dtype> > blob_pointer(new Blob<Dtype>());
				const int blob_id = blobs_.size();
				blobs_.push_back(blob_pointer);
				blob_names_.push_back(blob_name);
				blob_need_backward_.push_back(false);
				if (blob_name_to_idx) { (*blob_name_to_idx)[blob_name] = blob_id; }
				top_id_vecs_[layer_id].push_back(blob_id);
				top_vecs_[layer_id].push_back(blob_pointer.get());
			}
			if (available_blobs) { available_blobs->insert(blob_name); }
		}

	// Helper for Net::Init: add a new bottom blob to the net.
	template <typename Dtype>
		int Net<Dtype>::AppendBottom(const NetParameter& param, const int layer_id,
				const int bottom_id, set<string>* available_blobs,
				map<string, int>* blob_name_to_idx) {
			const LayerParameter& layer_param = param.layer(layer_id);
			const string& blob_name = layer_param.bottom(bottom_id);
			if (available_blobs->find(blob_name) == available_blobs->end()) {
				LOG(FATAL) << "Unknown bottom blob '" << blob_name << "' (layer '"
					<< layer_param.name() << "', bottom index " << bottom_id << ")";
			}
			const int blob_id = (*blob_name_to_idx)[blob_name];
			LOG_IF(INFO, Caffe::root_solver())
				<< layer_names_[layer_id] << " <- " << blob_name;
			bottom_vecs_[layer_id].push_back(blobs_[blob_id].get());
			bottom_id_vecs_[layer_id].push_back(blob_id);
			available_blobs->erase(blob_name);
			bool need_backward = blob_need_backward_[blob_id];
			// Check if the backpropagation on bottom_id should be skipped
			if (layer_param.propagate_down_size() > 0) {
				need_backward = layer_param.propagate_down(bottom_id);
			}
			bottom_need_backward_[layer_id].push_back(need_backward);
			return blob_id;
		}

	template <typename Dtype>
		void Net<Dtype>::AppendParam(const NetParameter& param, const int layer_id,
				const int param_id) {
			const LayerParameter& layer_param = layers_[layer_id]->layer_param();
			const int param_size = layer_param.param_size();
			string param_name =
				(param_size > param_id) ? layer_param.param(param_id).name() : "";
			if (param_name.size()) {
				param_display_names_.push_back(param_name);
			} else {
				ostringstream param_display_name;
				param_display_name << param_id;
				param_display_names_.push_back(param_display_name.str());
			}
			const int net_param_id = params_.size();
			params_.push_back(layers_[layer_id]->blobs()[param_id]);
			param_id_vecs_[layer_id].push_back(net_param_id);
			param_layer_indices_.push_back(make_pair(layer_id, param_id));
			ParamSpec default_param_spec;
			const ParamSpec* param_spec = (layer_param.param_size() > param_id) ?
				&layer_param.param(param_id) : &default_param_spec;
			if (!param_size || !param_name.size() || (param_name.size() &&
						param_names_index_.find(param_name) == param_names_index_.end())) {
				// This layer "owns" this parameter blob -- it is either anonymous
				// (i.e., not given a param_name) or explicitly given a name that we
				// haven't already seen.
				param_owners_.push_back(-1);
				if (param_name.size()) {
					param_names_index_[param_name] = net_param_id;
				}
				const int learnable_param_id = learnable_params_.size();
				learnable_params_.push_back(params_[net_param_id].get());
				learnable_param_ids_.push_back(learnable_param_id);
				has_params_lr_.push_back(param_spec->has_lr_mult());
				has_params_decay_.push_back(param_spec->has_decay_mult());
				params_lr_.push_back(param_spec->lr_mult());
				params_weight_decay_.push_back(param_spec->decay_mult());
			} else {
				// Named param blob with name we've seen before: share params
				const int owner_net_param_id = param_names_index_[param_name];
				param_owners_.push_back(owner_net_param_id);
				const pair<int, int>& owner_index =
					param_layer_indices_[owner_net_param_id];
				const int owner_layer_id = owner_index.first;
				const int owner_param_id = owner_index.second;
				LOG_IF(INFO, Caffe::root_solver()) << "Sharing parameters '" << param_name
					<< "' owned by "
					<< "layer '" << layer_names_[owner_layer_id] << "', param "
					<< "index " << owner_param_id;
				Blob<Dtype>* this_blob = layers_[layer_id]->blobs()[param_id].get();
				Blob<Dtype>* owner_blob =
					layers_[owner_layer_id]->blobs()[owner_param_id].get();
				const int param_size = layer_param.param_size();
				if (param_size > param_id && (layer_param.param(param_id).share_mode() ==
							ParamSpec_DimCheckMode_PERMISSIVE)) {
					// Permissive dimension checking -- only check counts are the same.
					CHECK_EQ(this_blob->count(), owner_blob->count())
						<< "Cannot share param '" << param_name << "' owned by layer '"
						<< layer_names_[owner_layer_id] << "' with layer '"
						<< layer_names_[layer_id] << "'; count mismatch.  Owner layer param "
						<< "shape is " << owner_blob->shape_string() << "; sharing layer "
						<< "shape is " << this_blob->shape_string();
				} else {
					// Strict dimension checking -- all dims must be the same.
					CHECK(this_blob->shape() == owner_blob->shape())
						<< "Cannot share param '" << param_name << "' owned by layer '"
						<< layer_names_[owner_layer_id] << "' with layer '"
						<< layer_names_[layer_id] << "'; shape mismatch.  Owner layer param "
						<< "shape is " << owner_blob->shape_string() << "; sharing layer "
						<< "expects shape " << this_blob->shape_string();
				}
				const int learnable_param_id = learnable_param_ids_[owner_net_param_id];
				learnable_param_ids_.push_back(learnable_param_id);
				if (param_spec->has_lr_mult()) {
					if (has_params_lr_[learnable_param_id]) {
						CHECK_EQ(param_spec->lr_mult(), params_lr_[learnable_param_id])
							<< "Shared param '" << param_name << "' has mismatched lr_mult.";
					} else {
						has_params_lr_[learnable_param_id] = true;
						params_lr_[learnable_param_id] = param_spec->lr_mult();
					}
				}
				if (param_spec->has_decay_mult()) {
					if (has_params_decay_[learnable_param_id]) {
						CHECK_EQ(param_spec->decay_mult(),
								params_weight_decay_[learnable_param_id])
							<< "Shared param '" << param_name << "' has mismatched decay_mult.";
					} else {
						has_params_decay_[learnable_param_id] = true;
						params_weight_decay_[learnable_param_id] = param_spec->decay_mult();
					}
				}
			}
		}

	//void DVFS_print(config conf)
	//{
	//	std::cout << conf.large_freq << "\t" << conf.small_freq << "\t" << conf.GPU_freq << std::endl;
	//}
	template <typename Dtype>
		int Net<Dtype>::getMask (int* m, int len){
			int res = 0;
			int i=0;
			while(i<len){
				res = res*10 + m[i];
				i++;
			}
			return res;
			//return c.mem_freq*4 - c.small_freq*3 + c.GPU_freq*2 - (c.large_freq * (c.large_mask[0] + c.large_mask[1])) - 100*(c.small_mask[0] + c.small_mask[1] + c.small_mask[2]+ c.small_mask[3]);

		}

	template <typename Dtype>
		int Net<Dtype>::getCores (int* m, int len){
			int res = 0;
			int i=0;
			while(i<len){
				if(m[i]==1)
					res++;
				i++;
			}
			return res;

		}


	template <typename Dtype>
		bool Net<Dtype>::myCompare(config c1, config c2){
			//return orderof(c1) > orderof(c2);
			if(c1.mem_freq < c2.mem_freq)
				return true;
			if(c1.mem_freq == c2.mem_freq && 
					c1.small_freq < c2.small_freq)
				return true;
			if(c1.mem_freq == c2.mem_freq && 
					c1.small_freq == c2.small_freq &&
					c1.GPU_freq < c2.GPU_freq)
				return true;
			if(c1.mem_freq == c2.mem_freq && 
					c1.small_freq == c2.small_freq &&
					c1.GPU_freq == c2.GPU_freq &&
					getCores(c1.large_mask, 2) < getCores(c2.large_mask, 2))
				return true;
			if(c1.mem_freq == c2.mem_freq && 
					c1.small_freq == c2.small_freq &&
					c1.GPU_freq == c2.GPU_freq &&
					getCores(c1.large_mask, 2) == getCores(c2.large_mask, 2) &&
					c1.large_freq < c2.large_freq)
				return true;
			if(c1.mem_freq == c2.mem_freq && 
					c1.small_freq == c2.small_freq &&
					c1.GPU_freq == c2.GPU_freq &&
					getCores(c1.large_mask, 2) == getCores(c2.large_mask, 2) &&
					c1.large_freq == c2.large_freq &&
					getCores(c2.small_mask, 4) < getCores(c2.small_mask, 4))
				return true;

			return false;
		}



	/* DVFS */
	template <typename Dtype>
		void Net<Dtype>::setup_DVFS(float period, const string config_path, const string Uncertainty_path)
		{
			_peroid=period; //ddl in ms

			ifstream fconf(config_path);
			string tmp;

			vector<config> raw; // filled
			while(getline(fconf,tmp))
			{
				istringstream iss(tmp);
				string token;
				config conf;
				vector<string> confstr;
				while(getline(iss, token, ' ')){
					confstr.push_back(token);
				}
				conf.ID = std::stoi(confstr[0]);
				conf.small_freq = std::stoi(confstr[1]);
				conf.small_mask[0] = std::stoi(confstr[2]);
				conf.small_mask[1] = std::stoi(confstr[3]);
				conf.small_mask[2] = std::stoi(confstr[4]);
				conf.small_mask[3] = std::stoi(confstr[5]);
				conf.large_freq = std::stoi(confstr[6]);
				conf.large_mask[0] = std::stoi(confstr[7]);
				conf.large_mask[1] = std::stoi(confstr[8]);
				conf.GPU_freq = std::stoi(confstr[9]);
				conf.mem_freq = std::stoi(confstr[10]);
				raw.push_back(conf);
				//DVFS_print(conf);
				//std::cout << conf.large_freq << "\t" << conf.small_freq << "\t" << conf.GPU_freq << std::endl;
			}

			cout << "=============raw.size: "<<raw.size() << endl;
			sort(raw.begin(), raw.end(), Net<Dtype>::myCompare);

			for(int i=0; i<raw.size(); i++)
				configurations[i]=raw[i];

			ifstream func(Uncertainty_path);
			getline(func,tmp);
			istringstream ass(tmp);
			string token;
			while(getline(ass,token, '\t')){
				_Uncertainty.push_back(stof(token));
				//std::cout<< (std::stof(token)) << std::endl;
			}

			isWarmup=true;	


			//fconf.close();
			//_slack = slack;
			//_Uncertainty = Uncertainty;
			/* load configs from config_path */
		}
	//template <typename Dtype>
	//int Net<Dtype>::setConfig(config & con)
	//{
	//	setBigClusterFreq(con.large_freq);
	//	setLittleClusterFreq(con.small_freq );
	//	int core[5]={(int)con.small_mask[1],(int)con.small_mask[2],(int)con.small_mask[3],(int)con.large_mask[0],(int)con.large_mask[1]};
	//		
	//	setAllCoresOnline(core, 5);
	//	setGpuFreq(con.GPU_freq);
	//	setEmcFreq(con.mem_freq);
	//	return 0;
	//
	//}

	template <typename Dtype>
		void Net<Dtype>::warmup()
		{
			//for(int i=0;i<24;i++){
			//	deadlines.push_back(i*1.0);
			//}
			srand (time(NULL));
			int conf = rand()%configurations.size();
			cout << "*********************Configuration size: " << configurations.size() << endl;
			//setConfig(configurations[conf]);

			config cur = configurations[conf];
			setSmallCoresOnline(cur.small_mask);
			setBigCoresOnline(cur.large_mask);	
			setBigClusterFreq(cur.large_freq);
			setLittleClusterFreq(cur.small_freq);
			setGpuFreq(cur.GPU_freq);
			setEmcFreq(cur.mem_freq);

			for(int i=0; i<100; i++)
			{
				current_configuration[i] = conf;
			}
			cout << "*********************" << endl;

		}

	template <typename Dtype>
		Dtype Net<Dtype>::ForwardFromTo(int start, int end) {
			CHECK_GE(start, 0);
			CHECK_LT(end, layers_.size());
			Dtype loss = 0;

			energymon em; 
			uint64_t start_nj[4], end_nj[4];
			uint64_t start_total, end_total;
			struct timespec ts;
			uint64_t exec_us=0, sched_us=0;
			double xup_exec_us =0.0;
			double xup_energy_mj=0.0;

			int iteration = 20;

			if(isWarmup){
				iteration = 20;
				warmup();
			}




			energymon_get_odroid(&em);
			em.finit(&em);
			//energymon_clock_gettime(&ts);
			//dvfs dv(2);



			//*** Process Sync. ****?
			key_t key,key2,key3;
			int shmid,shmid_conc;
			float *data;
			float *approx;
			int *concurrency_d;
			//char buffer[10];
			//char data_copy[SHM_SIZE] ="";
			float epsilon=0;
			float priority = 1;
			uint64_t total_running_time=0;
			// ftok to generate unique key
			key = ftok("/home/nvidia/datafile", 65);
			key2 = ftok("/home/nvidia/concurrfile", 65);
			key3 = ftok("/home/nvidia/approxfile", 65);



		        energymon_gettime_us(&ts);
			/* Progress */
			shmid = shmget(key, SHM_SIZE*sizeof(float), 0644| IPC_CREAT | IPC_EXCL);
			if ( errno == EEXIST )
			{
				shmid = shmget(key, SHM_SIZE*sizeof(float), 0644 );
			}
			else{
				shmid = shmget(key, SHM_SIZE*sizeof(float), 0644 | IPC_CREAT );
				if (shmid == -1)
				{
					perror("shmget");
					exit(1);
				}
			}
			if (shmid == -1)
			{
				perror("shmget");
				exit(1);
			}


			// std::cout << shmid << std::endl;
			data = (float *)shmat(shmid, NULL, 0);
			if (data == (float *)(-1)) {
				fprintf(stderr,"shmat error on line %d with error %s. \n", __LINE__,strerror(errno));
				exit(1);
			}


			/* Approximation */
			shmid = shmget(key3, SHM_SIZE*sizeof(float), 0644| IPC_CREAT | IPC_EXCL);
			if ( errno == EEXIST )
			{
				shmid = shmget(key3, SHM_SIZE*sizeof(float), 0644 );
			}
			else{
				shmid = shmget(key3, SHM_SIZE*sizeof(float), 0644 | IPC_CREAT );
				if (shmid == -1)
				{
					perror("shmget");
					exit(1);
				}
			}
			if (shmid == -1)
			{
				perror("shmget");
				exit(1);
			}


			// std::cout << shmid << std::endl;
			approx = (float *)shmat(shmid, NULL, 0);
			if (approx == (float *)(-1)) {
				fprintf(stderr,"shmat error on line %d with error %s. \n", __LINE__,strerror(errno));
				exit(1);
			}



			if ((shmid_conc = shmget(key2, sizeof(int), 0644 )) == -1 && errno == ENOENT) {
				if ((shmid_conc = shmget(key2, sizeof(int), 0644 | IPC_CREAT )) == -1)
				{
					perror("shmget");
					exit(1);
				}
				concurrency_d = (int *)shmat(shmid_conc, (void *)0, 0);
				if (concurrency_d == (int *)(-1)) {
					fprintf(stderr,"shmat error on line %d.\n", __LINE__);
					exit(1);
				}
				concurrency_d[0] = 0;
				if (shmdt(concurrency_d) == -1) {
					fprintf(stderr,"shmat error on line %d.\n", __LINE__);
					exit(1);
				}
			}
			__sync_synchronize ();

			concurrency_d = (int *)shmat(shmid_conc, (void *)0, 0);
			if (concurrency_d == (int *)(-1)) {
				fprintf(stderr,"shmat error on line %d.\n", __LINE__);
				exit(1);
			}
			//concurrency_d[0] =0;

			//std::cout << concurrency_d[0] << std::endl;
			int local_index = __sync_fetch_and_add(&concurrency_d[0],1);
			__sync_val_compare_and_swap(&concurrency_d[0],10,0);
			__sync_synchronize ();

		        sched_us = energymon_gettime_us(&ts);
			
			std::cout << "++++ shmem__us/sched_energy: " << sched_us << " "<< (end_total-start_total)/1000000.0 << std::endl;
			std::cout << "Concorrency= " <<  local_index << std::endl; 






			float max_score = end - start;
			float min_score = max_score - (max_score)*0.5; /* FIX ME: this is just for lowrank. Expand. */

			int lowrank_idx =0;
			for (int i = start; i <= end; ++i) {

#ifdef MAX_ACC
				float A_Star = max_score; 
#else
				float A_Star = ceil(priority * min_score + (1-priority)*max_score);
#endif		
				// Send epsilon
				//std::cout << "local_index = " << concurrency_d << " " << concurrency_d[0] << std::endl;
				data[local_index] = epsilon;
				approx[local_index] = max_score;

				for (int c = 0; c < before_forward_.size(); ++c) {
					before_forward_[c]->run(i);
				}

				Dtype layer_loss;



				//Hush

				//cout << isWarmup << endl;

				// DVFS: wait for scheduler
				if(!isWarmup)
				{
					/* Non-History */
					//current_configuration = schedule(_slack, deadlines[i], (float)exec_us/iteration, _Uncertainty[i],current_configuration);

					/* History */
					//cout<<"======Preparing to schedule======"<<endl;
					if(i > 0)
					{

						start_total = em.fread(&em, start_nj);
						energymon_gettime_us(&ts);
						//cout<< "Slack:" <<_slack <<" Deadline:" <<deadlines[i-1] << " History:[" <<  history[i-1][0].speedup << " " << rev_history[i-1][0]  << " " <<  history[i][0].speedup << " " << rev_history[i][0] << "]"<< endl;
						current_configuration[i] = schedule(priority,_slack, deadlines[i],deadlines[i-1], (float)exec_us/iteration + sched_us, _Uncertainty[i],current_configuration[i-1],current_configuration[i], history[i-1], rev_history[i-1], history[i], rev_history[i]);
						//cout << "Done scheduling." << endl;


						sched_us += energymon_gettime_us(&ts);
						end_total = em.fread(&em, start_nj);

						xup_exec_us += sched_us;
						xup_energy_mj += (double)(end_total-start_total)/1000000.0;


						if(current_configuration[i] >= configurations.size()){
							current_configuration[i] = configurations.size()-1;
						}

						// last current_configuration[i-1]
						// current current_configuration[i]
						//


						config last = configurations[current_configuration[i-1]];				
						config cur = configurations[current_configuration[i]];

						bool flag = true;
						for(int c=0;c<4;c++){
							if(last.small_mask[c]!=cur.small_mask[c]){
								flag=false;
								break;
							}
						}
						if(flag==false){
							setSmallCoresOnline(cur.small_mask);
						}


						flag=true;
						for(int c=0;c<2;c++){
							if(last.large_mask[c]!=cur.large_mask[c]){
								flag=false;
								break;
							}
						}
						if(flag==false){
							setBigCoresOnline(cur.large_mask);	
						}


						if(cur.large_freq!=last.large_freq){
							setBigClusterFreq(cur.large_freq);
						}

						if(cur.small_freq!=last.small_freq)
							setLittleClusterFreq(cur.small_freq);

						if(cur.GPU_freq!=last.GPU_freq)
							setGpuFreq(cur.GPU_freq);

						if(cur.mem_freq!=last.mem_freq)
							setEmcFreq(cur.mem_freq);

						//cout <<"############## Configuration:" << current_configuration[i] << " Uncertainty:" << _Uncertainty[i] << endl;
					}
				}	

								// Actual computation
				if(A_Star < max_score && layers_[i]->type() == "Convolution")
				{
					std::cout << "Using lowrank instead." << std::endl;
					//std::cout << "Layer type is convolution" << std::endl;
					// Run approximation
					vector<Blob<Dtype>*> tmp_top_vecs(top_vecs_[i].size());
					for(int kkl=0; kkl<top_vecs_[i].size(); kkl++)
					{
						tmp_top_vecs[kkl] = new Blob<Dtype>();
					}
					LayerParameter tmpParam = lowrank->layers_[lowrank_idx]->layer_param();
					LayerParameter tmpParam2 = lowrank->layers_[lowrank_idx+1]->layer_param();
					std::cout << "Layer names:" <<  tmpParam.name() << " and " <<tmpParam2.name() << std::endl;
					//std::cout << layers_[i]->type();	
					start_total = em.fread(&em, start_nj);
					//prev_configuration[i] = current_configuration;
					energymon_clock_gettime(&ts);

					for(int iter =0; iter < iteration; iter++){
						//std::cout << "\tType: " << layers_[i]->type() << std::endl;
						//layer_loss = layers_[i]->Forward(bottom_vecs_[i], top_vecs_[i]);

						//std::cout << "Vertical:" << std::endl;
						layer_loss = lowrank->layers_[lowrank_idx]->Forward(bottom_vecs_[i], tmp_top_vecs);
						//std::cout << "Horizontal:" << std::endl;
						layer_loss = lowrank->layers_[lowrank_idx+1]->Forward(tmp_top_vecs, top_vecs_[i]);
					}
					// Adjust max_score for next layers
					max_score -= 0.5; /* FIX ME: this is just for lowrank. Expand */
				}		

				else{
					//std::cout << layers_[i]->type();	
					start_total = em.fread(&em, start_nj);
					//prev_configuration[i] = current_configuration;
					energymon_clock_gettime(&ts);
					for(int iter =0; iter < iteration; iter++){
						//std::cout << "\tType: " << layers_[i]->type() << std::endl;
						layer_loss = layers_[i]->Forward(bottom_vecs_[i], top_vecs_[i]);
					}
				}
				exec_us = energymon_gettime_us(&ts);
				end_total = em.fread(&em, end_nj);
				xup_exec_us += exec_us/iteration;
				xup_energy_mj += (double)(end_total-start_total)/iteration/1000000.0;

				//usleep(10000);
				std::cout << "Latency(us)/Energy(mJ): " << exec_us/iteration << " " 
					<< (end_total - start_total)/1000000.0/iteration << " "
					<< std::endl;
				if(isWarmup){
					deadlines.push_back(exec_us/iteration);
				}
				else
				{
					epsilon = _slack;
					//std::cout<< "Cohort={ ";
					int cohort_size = 0;
					float tmp_priority=0;

					energymon_gettime_us(&ts);
#ifdef MAX_ACC
					float max_epsilon = INT_MIN;
					for(int i=0; i< SHM_SIZE; i++)
					{
						if(data[i] > max_epsilon)
						{
							max_epsilon = data[i];
						}
					}
#elif defined MIN_ENG
					float min_epsilon = INT_MAX;
					for(int i=0; i< SHM_SIZE; i++)
					{
						if(data[i] < min_epsilon)
						{
							min_epsilon = data[i];
						}
					}
					std::cout << "Min energy selected." << std::endl;

#endif
					for (int i=0; i< SHM_SIZE; i++)
					{
						//std::cout << data[i] << ",";
						if(data[i]!=0)
						{
							cohort_size++;
#ifdef MAX_ACC
							tmp_priority += 1+ max_epsilon/DEADLINE;
#elif defined MIN_ENG
							tmp_priority += 1+ min_epsilon/DEADLINE;
#else
							tmp_priority += 1+ data[i]/DEADLINE;
#endif
						}
					}
					priority = tmp_priority/cohort_size;


					//std::cout<< "}"<< std::endl;


					sched_us += energymon_gettime_us(&ts);

				}	
				loss += layer_loss;
				//if (debug_info_) { ForwardDebugInfo(i); }
				for (int c = 0; c < after_forward_.size(); ++c) {
					after_forward_[c]->run(i);
				}


				lowrank_idx++;
				if(layers_[i]->type() == "Convolution")
				{
					lowrank_idx++;
				}
			}

			if(isWarmup){
				float sum_deadlines = 0;
				for(int i=0;i<deadlines.size();i++){
					sum_deadlines+=deadlines[i];
				}
				_slack = _peroid*1000 - sum_deadlines;
				std::cout << "_peroid: " << _peroid << std::endl;
				//cout << "########### _slack " << _slack <<  endl;	
			}
			isWarmup=false;
			//cout << "**********" <<  deadlines.size() << endl;
			em.ffinish(&em);	
			std::cout<< "Progress Cohort={ ";
			for (int i=0; i< SHM_SIZE; i++)
			{
				std::cout << data[i] << ",";
			}


			std::cout<< "Accuracy Cohort={ ";
			for (int i=0; i< SHM_SIZE; i++)
			{
				std::cout << approx[i] << ",";
			}
			std::cout<< "}"<< std::endl;

			std::cout<< "}"<< std::endl;

			std::cout << "++++ sched_us/sched_energy: " << sched_us << " "<< (end_total-start_total)/1000000.0 << std::endl;
			std::cout << "++++ xup_exec_us/xup_energy_mj: " << xup_exec_us << " "<< xup_energy_mj << std::endl;
			if (shmdt(data) == -1) {
				perror("shmdt");
				exit(1);
			}

			return loss;
		}

	template <typename Dtype>
		Dtype Net<Dtype>::ForwardFrom(int start) {
			return ForwardFromTo(start, layers_.size() - 1);
		}

	template <typename Dtype>
		Dtype Net<Dtype>::ForwardTo(int end) {
			return ForwardFromTo(0, end);
		}

	template <typename Dtype>
		const vector<Blob<Dtype>*>& Net<Dtype>::Forward(Dtype* loss) {
			if (loss != NULL) {
				*loss = ForwardFromTo(0, layers_.size() - 1);
			} else {
				ForwardFromTo(0, layers_.size() - 1);
			}
			return net_output_blobs_;
		}

	template <typename Dtype>
		const vector<Blob<Dtype>*>& Net<Dtype>::Forward(
				const vector<Blob<Dtype>*> & bottom, Dtype* loss) {
			LOG_EVERY_N(WARNING, 1000) << "DEPRECATED: Forward(bottom, loss) "
				<< "will be removed in a future version. Use Forward(loss).";
			// Copy bottom to net bottoms
			for (int i = 0; i < bottom.size(); ++i) {
				net_input_blobs_[i]->CopyFrom(*bottom[i]);
			}
			return Forward(loss);
		}

	template <typename Dtype>
		void Net<Dtype>::BackwardFromTo(int start, int end) {
			CHECK_GE(end, 0);
			CHECK_LT(start, layers_.size());
			for (int i = start; i >= end; --i) {
				for (int c = 0; c < before_backward_.size(); ++c) {
					before_backward_[c]->run(i);
				}
				if (layer_need_backward_[i]) {
					layers_[i]->Backward(
							top_vecs_[i], bottom_need_backward_[i], bottom_vecs_[i]);
					if (debug_info_) { BackwardDebugInfo(i); }
				}
				for (int c = 0; c < after_backward_.size(); ++c) {
					after_backward_[c]->run(i);
				}
			}
		}

	template <typename Dtype>
		void Net<Dtype>::ForwardDebugInfo(const int layer_id) {
			for (int top_id = 0; top_id < top_vecs_[layer_id].size(); ++top_id) {
				const Blob<Dtype>& blob = *top_vecs_[layer_id][top_id];
				const string& blob_name = blob_names_[top_id_vecs_[layer_id][top_id]];
				const Dtype data_abs_val_mean = blob.asum_data() / blob.count();
				LOG_IF(INFO, Caffe::root_solver())
					<< "    [Forward] "
					<< "Layer " << layer_names_[layer_id]
					<< ", top blob " << blob_name
					<< " data: " << data_abs_val_mean;
			}
			for (int param_id = 0; param_id < layers_[layer_id]->blobs().size();
					++param_id) {
				const Blob<Dtype>& blob = *layers_[layer_id]->blobs()[param_id];
				const int net_param_id = param_id_vecs_[layer_id][param_id];
				const string& blob_name = param_display_names_[net_param_id];
				const Dtype data_abs_val_mean = blob.asum_data() / blob.count();
				LOG_IF(INFO, Caffe::root_solver())
					<< "    [Forward] "
					<< "Layer " << layer_names_[layer_id]
					<< ", param blob " << blob_name
					<< " data: " << data_abs_val_mean;
			}
		}

	template <typename Dtype>
		void Net<Dtype>::BackwardDebugInfo(const int layer_id) {
			const vector<Blob<Dtype>*>& bottom_vec = bottom_vecs_[layer_id];
			for (int bottom_id = 0; bottom_id < bottom_vec.size(); ++bottom_id) {
				if (!bottom_need_backward_[layer_id][bottom_id]) { continue; }
				const Blob<Dtype>& blob = *bottom_vec[bottom_id];
				const string& blob_name = blob_names_[bottom_id_vecs_[layer_id][bottom_id]];
				const Dtype diff_abs_val_mean = blob.asum_diff() / blob.count();
				LOG_IF(INFO, Caffe::root_solver())
					<< "    [Backward] "
					<< "Layer " << layer_names_[layer_id]
					<< ", bottom blob " << blob_name
					<< " diff: " << diff_abs_val_mean;
			}
			for (int param_id = 0; param_id < layers_[layer_id]->blobs().size();
					++param_id) {
				if (!layers_[layer_id]->param_propagate_down(param_id)) { continue; }
				const Blob<Dtype>& blob = *layers_[layer_id]->blobs()[param_id];
				const Dtype diff_abs_val_mean = blob.asum_diff() / blob.count();
				LOG_IF(INFO, Caffe::root_solver())
					<< "    [Backward] "
					<< "Layer " << layer_names_[layer_id]
					<< ", param blob " << param_id
					<< " diff: " << diff_abs_val_mean;
			}
		}

	template <typename Dtype>
		void Net<Dtype>::UpdateDebugInfo(const int param_id) {
			const Blob<Dtype>& blob = *params_[param_id];
			const int param_owner = param_owners_[param_id];
			const string& layer_name = layer_names_[param_layer_indices_[param_id].first];
			const string& param_display_name = param_display_names_[param_id];
			const Dtype diff_abs_val_mean = blob.asum_diff() / blob.count();
			if (param_owner < 0) {
				const Dtype data_abs_val_mean = blob.asum_data() / blob.count();
				LOG_IF(INFO, Caffe::root_solver())
					<< "    [Update] Layer " << layer_name
					<< ", param " << param_display_name
					<< " data: " << data_abs_val_mean
					<< "; diff: " << diff_abs_val_mean;
			} else {
				const string& owner_layer_name =
					layer_names_[param_layer_indices_[param_owner].first];
				LOG_IF(INFO, Caffe::root_solver())
					<< "    [Update] Layer " << layer_name
					<< ", param blob " << param_display_name
					<< " (owned by layer " << owner_layer_name << ", " << "param "
					<< param_display_names_[param_owners_[param_id]] << ")"
					<< " diff: " << diff_abs_val_mean;
			}
		}

	template <typename Dtype>
		void Net<Dtype>::ShareTrainedLayersWith(const Net* other) {
			int num_source_layers = other->layers().size();
			for (int i = 0; i < num_source_layers; ++i) {
				Layer<Dtype>* source_layer = other->layers()[i].get();
				const string& source_layer_name = other->layer_names()[i];
				int target_layer_id = 0;
				while (target_layer_id != layer_names_.size() &&
						layer_names_[target_layer_id] != source_layer_name) {
					++target_layer_id;
				}
				if (target_layer_id == layer_names_.size()) {
					LOG(INFO) << "Ignoring source layer " << source_layer_name;
					continue;
				}
				DLOG(INFO) << "Copying source layer " << source_layer_name;
				vector<shared_ptr<Blob<Dtype> > >& target_blobs =
					layers_[target_layer_id]->blobs();
				CHECK_EQ(target_blobs.size(), source_layer->blobs().size())
					<< "Incompatible number of blobs for layer " << source_layer_name;
				for (int j = 0; j < target_blobs.size(); ++j) {
					Blob<Dtype>* source_blob = source_layer->blobs()[j].get();
					CHECK(target_blobs[j]->shape() == source_blob->shape())
						<< "Cannot share param " << j << " weights from layer '"
						<< source_layer_name << "'; shape mismatch.  Source param shape is "
						<< source_blob->shape_string() << "; target param shape is "
						<< target_blobs[j]->shape_string();
					target_blobs[j]->ShareData(*source_blob);
				}
			}
		}

	template <typename Dtype>
		void Net<Dtype>::BackwardFrom(int start) {
			BackwardFromTo(start, 0);
		}

	template <typename Dtype>
		void Net<Dtype>::BackwardTo(int end) {
			BackwardFromTo(layers_.size() - 1, end);
		}

	template <typename Dtype>
		void Net<Dtype>::Backward() {
			BackwardFromTo(layers_.size() - 1, 0);
			if (debug_info_) {
				Dtype asum_data = 0, asum_diff = 0, sumsq_data = 0, sumsq_diff = 0;
				for (int i = 0; i < learnable_params_.size(); ++i) {
					asum_data += learnable_params_[i]->asum_data();
					asum_diff += learnable_params_[i]->asum_diff();
					sumsq_data += learnable_params_[i]->sumsq_data();
					sumsq_diff += learnable_params_[i]->sumsq_diff();
				}
				const Dtype l2norm_data = std::sqrt(sumsq_data);
				const Dtype l2norm_diff = std::sqrt(sumsq_diff);
				LOG(ERROR) << "    [Backward] All net params (data, diff): "
					<< "L1 norm = (" << asum_data << ", " << asum_diff << "); "
					<< "L2 norm = (" << l2norm_data << ", " << l2norm_diff << ")";
			}
		}

	template <typename Dtype>
		void Net<Dtype>::Reshape() {
			for (int i = 0; i < layers_.size(); ++i) {
				layers_[i]->Reshape(bottom_vecs_[i], top_vecs_[i]);
			}
		}

	template <typename Dtype>
		void Net<Dtype>::CopyTrainedLayersFrom(const NetParameter& param) {
			int num_source_layers = param.layer_size();
			for (int i = 0; i < num_source_layers; ++i) {
				const LayerParameter& source_layer = param.layer(i);
				const string& source_layer_name = source_layer.name();
				int target_layer_id = 0;
				while (target_layer_id != layer_names_.size() &&
						layer_names_[target_layer_id] != source_layer_name) {
					++target_layer_id;
				}
				if (target_layer_id == layer_names_.size()) {
					LOG(INFO) << "Ignoring source layer " << source_layer_name;
					continue;
				}
				DLOG(INFO) << "Copying source layer " << source_layer_name;
				vector<shared_ptr<Blob<Dtype> > >& target_blobs =
					layers_[target_layer_id]->blobs();
				CHECK_EQ(target_blobs.size(), source_layer.blobs_size())
					<< "Incompatible number of blobs for layer " << source_layer_name;
				for (int j = 0; j < target_blobs.size(); ++j) {
					if (!target_blobs[j]->ShapeEquals(source_layer.blobs(j))) {
						Blob<Dtype> source_blob;
						const bool kReshape = true;
						source_blob.FromProto(source_layer.blobs(j), kReshape);
						LOG(FATAL) << "Cannot copy param " << j << " weights from layer '"
							<< source_layer_name << "'; shape mismatch.  Source param shape is "
							<< source_blob.shape_string() << "; target param shape is "
							<< target_blobs[j]->shape_string() << ". "
							<< "To learn this layer's parameters from scratch rather than "
							<< "copying from a saved net, rename the layer.";
					}
					const bool kReshape = false;
					target_blobs[j]->FromProto(source_layer.blobs(j), kReshape);
				}
			}
		}

	template <typename Dtype>
		void Net<Dtype>::CopyTrainedLayersFrom(const string trained_filename) {
			if (H5Fis_hdf5(trained_filename.c_str())) {
				CopyTrainedLayersFromHDF5(trained_filename);
			} else {
				CopyTrainedLayersFromBinaryProto(trained_filename);
			}
		}

	template <typename Dtype>
		void Net<Dtype>::CopyTrainedLayersFromBinaryProto(
				const string trained_filename) {
			NetParameter param;
			ReadNetParamsFromBinaryFileOrDie(trained_filename, &param);
			CopyTrainedLayersFrom(param);
		}

	template <typename Dtype>
		void Net<Dtype>::CopyTrainedLayersFromHDF5(const string trained_filename) {
			hid_t file_hid = H5Fopen(trained_filename.c_str(), H5F_ACC_RDONLY,
					H5P_DEFAULT);
			CHECK_GE(file_hid, 0) << "Couldn't open " << trained_filename;
			hid_t data_hid = H5Gopen2(file_hid, "data", H5P_DEFAULT);
			CHECK_GE(data_hid, 0) << "Error reading weights from " << trained_filename;
			int num_layers = hdf5_get_num_links(data_hid);
			for (int i = 0; i < num_layers; ++i) {
				string source_layer_name = hdf5_get_name_by_idx(data_hid, i);
				if (!layer_names_index_.count(source_layer_name)) {
					LOG(INFO) << "Ignoring source layer " << source_layer_name;
					continue;
				}
				int target_layer_id = layer_names_index_[source_layer_name];
				DLOG(INFO) << "Copying source layer " << source_layer_name;
				vector<shared_ptr<Blob<Dtype> > >& target_blobs =
					layers_[target_layer_id]->blobs();
				hid_t layer_hid = H5Gopen2(data_hid, source_layer_name.c_str(),
						H5P_DEFAULT);
				CHECK_GE(layer_hid, 0)
					<< "Error reading weights from " << trained_filename;
				// Check that source layer doesn't have more params than target layer
				int num_source_params = hdf5_get_num_links(layer_hid);
				CHECK_LE(num_source_params, target_blobs.size())
					<< "Incompatible number of blobs for layer " << source_layer_name;
				for (int j = 0; j < target_blobs.size(); ++j) {
					ostringstream oss;
					oss << j;
					string dataset_name = oss.str();
					int target_net_param_id = param_id_vecs_[target_layer_id][j];
					if (!H5Lexists(layer_hid, dataset_name.c_str(), H5P_DEFAULT)) {
						// Target param doesn't exist in source weights...
						if (param_owners_[target_net_param_id] != -1) {
							// ...but it's weight-shared in target, so that's fine.
							continue;
						} else {
							LOG(FATAL) << "Incompatible number of blobs for layer "
								<< source_layer_name;
						}
					}
					hdf5_load_nd_dataset(layer_hid, dataset_name.c_str(), 0, kMaxBlobAxes,
							target_blobs[j].get());
				}
				H5Gclose(layer_hid);
			}
			H5Gclose(data_hid);
			H5Fclose(file_hid);
		}

	template <typename Dtype>
		void Net<Dtype>::ToProto(NetParameter* param, bool write_diff) const {
			param->Clear();
			param->set_name(name_);
			// Add bottom and top
			DLOG(INFO) << "Serializing " << layers_.size() << " layers";
			for (int i = 0; i < layers_.size(); ++i) {
				LayerParameter* layer_param = param->add_layer();
				layers_[i]->ToProto(layer_param, write_diff);
			}
		}

	template <typename Dtype>
		void Net<Dtype>::ToHDF5(const string& filename, bool write_diff) const {
			hid_t file_hid = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT,
					H5P_DEFAULT);
			CHECK_GE(file_hid, 0)
				<< "Couldn't open " << filename << " to save weights.";
			hid_t data_hid = H5Gcreate2(file_hid, "data", H5P_DEFAULT, H5P_DEFAULT,
					H5P_DEFAULT);
			CHECK_GE(data_hid, 0) << "Error saving weights to " << filename << ".";
			hid_t diff_hid = -1;
			if (write_diff) {
				diff_hid = H5Gcreate2(file_hid, "diff", H5P_DEFAULT, H5P_DEFAULT,
						H5P_DEFAULT);
				CHECK_GE(diff_hid, 0) << "Error saving weights to " << filename << ".";
			}
			for (int layer_id = 0; layer_id < layers_.size(); ++layer_id) {
				const LayerParameter& layer_param = layers_[layer_id]->layer_param();
				string layer_name = layer_param.name();
				hid_t layer_data_hid = H5Gcreate2(data_hid, layer_name.c_str(),
						H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
				CHECK_GE(layer_data_hid, 0)
					<< "Error saving weights to " << filename << ".";
				hid_t layer_diff_hid = -1;
				if (write_diff) {
					layer_diff_hid = H5Gcreate2(diff_hid, layer_name.c_str(),
							H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
					CHECK_GE(layer_diff_hid, 0)
						<< "Error saving weights to " << filename << ".";
				}
				int num_params = layers_[layer_id]->blobs().size();
				for (int param_id = 0; param_id < num_params; ++param_id) {
					ostringstream dataset_name;
					dataset_name << param_id;
					const int net_param_id = param_id_vecs_[layer_id][param_id];
					if (param_owners_[net_param_id] == -1) {
						// Only save params that own themselves
						hdf5_save_nd_dataset<Dtype>(layer_data_hid, dataset_name.str(),
								*params_[net_param_id]);
					}
					if (write_diff) {
						// Write diffs regardless of weight-sharing
						hdf5_save_nd_dataset<Dtype>(layer_diff_hid, dataset_name.str(),
								*params_[net_param_id], true);
					}
				}
				H5Gclose(layer_data_hid);
				if (write_diff) {
					H5Gclose(layer_diff_hid);
				}
			}
			H5Gclose(data_hid);
			if (write_diff) {
				H5Gclose(diff_hid);
			}
			H5Fclose(file_hid);
		}

	template <typename Dtype>
		void Net<Dtype>::Update() {
			for (int i = 0; i < learnable_params_.size(); ++i) {
				learnable_params_[i]->Update();
			}
		}

	template <typename Dtype>
		void Net<Dtype>::ClearParamDiffs() {
			for (int i = 0; i < learnable_params_.size(); ++i) {
				Blob<Dtype>* blob = learnable_params_[i];
				switch (Caffe::mode()) {
					case Caffe::CPU:
						caffe_set(blob->count(), static_cast<Dtype>(0),
								blob->mutable_cpu_diff());
						break;
					case Caffe::GPU:
#ifndef CPU_ONLY
						caffe_gpu_set(blob->count(), static_cast<Dtype>(0),
								blob->mutable_gpu_diff());
#else
						NO_GPU;
#endif
						break;
				}
			}
		}

	template <typename Dtype>
		void Net<Dtype>::ShareWeights() {
			for (int i = 0; i < params_.size(); ++i) {
				if (param_owners_[i] < 0) { continue; }
				params_[i]->ShareData(*params_[param_owners_[i]]);
				params_[i]->ShareDiff(*params_[param_owners_[i]]);
			}
		}

	template <typename Dtype>
		bool Net<Dtype>::has_blob(const string& blob_name) const {
			return blob_names_index_.find(blob_name) != blob_names_index_.end();
		}

	template <typename Dtype>
		const shared_ptr<Blob<Dtype> > Net<Dtype>::blob_by_name(
				const string& blob_name) const {
			shared_ptr<Blob<Dtype> > blob_ptr;
			if (has_blob(blob_name)) {
				blob_ptr = blobs_[blob_names_index_.find(blob_name)->second];
			} else {
				blob_ptr.reset((Blob<Dtype>*)(NULL));
				LOG(WARNING) << "Unknown blob name " << blob_name;
			}
			return blob_ptr;
		}

	template <typename Dtype>
		bool Net<Dtype>::has_layer(const string& layer_name) const {
			return layer_names_index_.find(layer_name) != layer_names_index_.end();
		}

	template <typename Dtype>
		const shared_ptr<Layer<Dtype> > Net<Dtype>::layer_by_name(
				const string& layer_name) const {
			shared_ptr<Layer<Dtype> > layer_ptr;
			if (has_layer(layer_name)) {
				layer_ptr = layers_[layer_names_index_.find(layer_name)->second];
			} else {
				layer_ptr.reset((Layer<Dtype>*)(NULL));
				LOG(WARNING) << "Unknown layer name " << layer_name;
			}
			return layer_ptr;
		}

	INSTANTIATE_CLASS(Net);

}  // namespace caffe
