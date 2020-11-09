#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <dirent.h>
#include <cstdlib>
#include <energymon/energymon-odroid.h>
#include <energymon/energymon-time-util.h>
//#include "setconfig.h"

using namespace caffe;  // NOLINT(build/namespaces)
using std::string;

/* Pair (label, confidence) representing a prediction. */
typedef std::pair<string, float> Prediction;

class Classifier {
	public:
		Classifier(const string& model_file,
				const string& trained_file,
				const string& mean_file,
				const string& label_file);
		Classifier(const string& model_file,
				const string& trained_file,
				const string& mean_file,
				const string& label_file,
				const int batch_size);

		Classifier(const string& model_file,
				const string& trained_file,
				const string& mean_file,
				const string& label_file,
				const string& config_file,
				const string& Uncertainty_file,
				float slack, 
				float period);


		Classifier(const string& model_file,
				const string& trained_file,
				const string& mean_file,
				const string& label_file,
				const string& config_file,
				const string& Uncertainty_file,
				float slack, 
				float period,
				const string& lowrank_model_file,
				const string& lowrank_trained_file
				);

		Classifier(const string& model_file,
				const string& trained_file,
				const string& mean_file,
				const string& label_file,
				const string& config_file,
				const string& Uncertainty_file,
				float slack, 
				float period,
				const string& lowrank_model_file,
				const string& lowrank_trained_file,
				int   config_mode
				);


		std::vector<Prediction> Classify(const cv::Mat& img, int N = 5);
		std::vector< vector<Prediction> > ClassifyBatch(const vector< cv::Mat > imgs, int num_classes = 5);

		void SetMean(const string& mean_file);

		std::vector<float> Predict(const cv::Mat& img);

		void WrapInputLayer(std::vector<cv::Mat>* input_channels);

		void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);

		boost::shared_ptr<Net<float> > net_;
		cv::Size input_geometry_;
		int num_channels_;
		cv::Mat mean_;
		std::vector<string> labels_;
		int batch_size_;
		float _slack;
};


Classifier::Classifier(const string& model_file,
		const string& trained_file,
		const string& mean_file,
		const string& label_file,
		const string& config_file,
		const string& Uncertainty_file,
		float slack,
		float period,
		const string& lowrank_model_file,
		const string& lowrank_trained_file)
 {
	#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
	#else
	Caffe::set_mode(Caffe::GPU);
	#endif


	_slack = slack;

	/* Load the network. */
	net_.reset(new Net<float>(model_file, TEST));
	net_->lowrank.reset(new Net<float>(lowrank_model_file, TEST));
	
	net_->CopyTrainedLayersFrom(trained_file);
	net_->lowrank->CopyTrainedLayersFrom(lowrank_trained_file);

	
	/* For DVFS */
	net_->setup_DVFS(period,config_file,Uncertainty_file);


	CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

	Blob<float>* input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	CHECK(num_channels_ == 3 || num_channels_ == 1)
		<< "Input layer should have 1 or 3 channels.";
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

	/* Load the binaryproto mean file. */
	SetMean(mean_file);

	/* Load labels. */
	std::ifstream labels(label_file.c_str());
	CHECK(labels) << "Unable to open labels file " << label_file;
	string line;
	while (std::getline(labels, line))
		labels_.push_back(string(line));

	Blob<float>* output_layer = net_->output_blobs()[0];
	CHECK_EQ(labels_.size(), output_layer->channels())
		<< "Number of labels is different from the output layer dimension.";
}



Classifier::Classifier(const string& model_file,
		const string& trained_file,
		const string& mean_file,
		const string& label_file,
		const string& config_file,
		const string& Uncertainty_file,
		float slack,
		float period) {
	#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
	#else
	Caffe::set_mode(Caffe::GPU);
	#endif


	_slack = slack;

	/* Load the network. */
	net_.reset(new Net<float>(model_file, TEST));
	net_->CopyTrainedLayersFrom(trained_file);
	
	/* For DVFS */
	net_->setup_DVFS(period,config_file,Uncertainty_file);


	CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

	Blob<float>* input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	CHECK(num_channels_ == 3 || num_channels_ == 1)
		<< "Input layer should have 1 or 3 channels.";
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

	/* Load the binaryproto mean file. */
	SetMean(mean_file);

	/* Load labels. */
	std::ifstream labels(label_file.c_str());
	CHECK(labels) << "Unable to open labels file " << label_file;
	string line;
	while (std::getline(labels, line))
		labels_.push_back(string(line));

	Blob<float>* output_layer = net_->output_blobs()[0];
	CHECK_EQ(labels_.size(), output_layer->channels())
		<< "Number of labels is different from the output layer dimension.";
}

Classifier::Classifier(const string& model_file,
		const string& trained_file,
		const string& mean_file,
		const string& label_file) {
	Caffe::set_mode(Caffe::GPU);

	/* Load the network. */
	net_.reset(new Net<float>(model_file, TEST));
	net_->CopyTrainedLayersFrom(trained_file);

	CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

	Blob<float>* input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	CHECK(num_channels_ == 3 || num_channels_ == 1)
		<< "Input layer should have 1 or 3 channels.";
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

	/* Load the binaryproto mean file. */
	SetMean(mean_file);

	/* Load labels. */
	std::ifstream labels(label_file.c_str());
	CHECK(labels) << "Unable to open labels file " << label_file;
	string line;
	while (std::getline(labels, line))
		labels_.push_back(string(line));

	Blob<float>* output_layer = net_->output_blobs()[0];
	CHECK_EQ(labels_.size(), output_layer->channels())
		<< "Number of labels is different from the output layer dimension.";
}

Classifier::Classifier(const string& model_file,
		const string& trained_file,
		const string& mean_file,
		const string& label_file,
		const int batch_size) {
	Caffe::set_mode(Caffe::GPU);

	/* Set batchsize */
	batch_size_ = batch_size;

	/* Load the network. */
	net_.reset(new Net<float>(model_file, TEST));
	net_->CopyTrainedLayersFrom(trained_file);

	CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

	Blob<float>* input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	CHECK(num_channels_ == 3 || num_channels_ == 1)
		<< "Input layer should have 1 or 3 channels.";
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

	/* Load the binaryproto mean file. */
	SetMean(mean_file);

	/* Load labels. */
	std::ifstream labels(label_file.c_str());
	CHECK(labels) << "Unable to open labels file " << label_file;
	string line;
	while (std::getline(labels, line))
		labels_.push_back(string(line));

	Blob<float>* output_layer = net_->output_blobs()[0];
	CHECK_EQ(labels_.size(), output_layer->channels())
		<< "Number of labels is different from the output layer dimension.";
}


Classifier::Classifier(const string& model_file,
		const string& trained_file,
		const string& mean_file,
		const string& label_file,
		const string& config_file,
		const string& Uncertainty_file,
		float slack,
		float period,
		const string& lowrank_model_file,
		const string& lowrank_trained_file,
		int           config_mode)
 {
	#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
	#else
	Caffe::set_mode(Caffe::GPU);
	#endif


	_slack = slack;

	/* Load the network. */
	net_.reset(new Net<float>(model_file, TEST));
	net_->lowrank.reset(new Net<float>(lowrank_model_file, TEST));
	
	net_->CopyTrainedLayersFrom(trained_file);
	net_->lowrank->CopyTrainedLayersFrom(lowrank_trained_file);

	
	/* For DVFS */
	if(config_mode)
		net_->setup_DVFS(period,config_file,Uncertainty_file);
	else
		net_->shutdown_config();

	CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
	CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

	Blob<float>* input_layer = net_->input_blobs()[0];
	num_channels_ = input_layer->channels();
	CHECK(num_channels_ == 3 || num_channels_ == 1)
		<< "Input layer should have 1 or 3 channels.";
	input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

	/* Load the binaryproto mean file. */
	SetMean(mean_file);

	/* Load labels. */
	std::ifstream labels(label_file.c_str());
	CHECK(labels) << "Unable to open labels file " << label_file;
	string line;
	while (std::getline(labels, line))
		labels_.push_back(string(line));

	Blob<float>* output_layer = net_->output_blobs()[0];
	CHECK_EQ(labels_.size(), output_layer->channels())
		<< "Number of labels is different from the output layer dimension.";
}

static bool PairCompare(const std::pair<float, int>& lhs,
		const std::pair<float, int>& rhs) {
	return lhs.first > rhs.first;
}

/* Return the indices of the top N values of vector v. */
static std::vector<int> Argmax(const std::vector<float>& v, int N) {
	std::vector<std::pair<float, int> > pairs;
	for (size_t i = 0; i < v.size(); ++i)
		pairs.push_back(std::make_pair(v[i], i));
	std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

	std::vector<int> result;
	for (int i = 0; i < N; ++i)
		result.push_back(pairs[i].second);
	return result;
}

/* Return the top N predictions. */
std::vector<Prediction> Classifier::Classify(const cv::Mat& img, int N) {
	std::vector<float> output = Predict(img);
	N = std::min<int>(labels_.size(), N);
	std::vector<int> maxN = Argmax(output, N);
	std::vector<Prediction> predictions;
	for (int i = 0; i < N; ++i) {
		int idx = maxN[i];
		predictions.push_back(std::make_pair(labels_[idx], output[idx]));
	}

	return predictions;
}


/* Load the mean file in binaryproto format. */
void Classifier::SetMean(const string& mean_file) {
	BlobProto blob_proto;
	ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);

	/* Convert from BlobProto to Blob<float> */
	Blob<float> mean_blob;
	mean_blob.FromProto(blob_proto);
	CHECK_EQ(mean_blob.channels(), num_channels_)
		<< "Number of channels of mean file doesn't match input layer.";

	/* The format of the mean file is planar 32-bit float BGR or grayscale. */
	std::vector<cv::Mat> channels;
	float* data = mean_blob.mutable_cpu_data();
	for (int i = 0; i < num_channels_; ++i) {
		/* Extract an individual channel. */
		cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
		channels.push_back(channel);
		data += mean_blob.height() * mean_blob.width();
	}

	/* Merge the separate channels into a single image. */
	cv::Mat mean;
	cv::merge(channels, mean);

	/* Compute the global mean pixel value and create a mean image
	 * filled with this value. */
	cv::Scalar channel_mean = cv::mean(mean);
	mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
}

std::vector<float> Classifier::Predict(const cv::Mat& img) {
	Blob<float>* input_layer = net_->input_blobs()[0];
	input_layer->Reshape(1, num_channels_,
			input_geometry_.height, input_geometry_.width);
	/* Forward dimension change to all layers. */
	net_->Reshape();
	net_->lowrank->Reshape();

	std::vector<cv::Mat> input_channels;
	WrapInputLayer(&input_channels);

	Preprocess(img, &input_channels);

	net_->Forward();

	/* Copy the output layer to a std::vector */
	Blob<float>* output_layer = net_->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels();
	return std::vector<float>(begin, end);
}


/* Wrap the input layer of the network in separate cv::Mat objects
 * (one per channel). This way we save one memcpy operation and we
 * don't need to rely on cudaMemcpy2D. The last preprocessing
 * operation will write the separate channels directly to the input
 * layer. */
void Classifier::WrapInputLayer(std::vector<cv::Mat>* input_channels) {
	Blob<float>* input_layer = net_->input_blobs()[0];

	int width = input_layer->width();
	int height = input_layer->height();
	float* input_data = input_layer->mutable_cpu_data();
	for (int i = 0; i < input_layer->channels(); ++i) {
		cv::Mat channel(height, width, CV_32FC1, input_data);
		input_channels->push_back(channel);
		input_data += width * height;
	}
}


void Classifier::Preprocess(const cv::Mat& img,
		std::vector<cv::Mat>* input_channels) {
	/* Convert the input image to the input image format of the network. */
	cv::Mat sample;
	if (img.channels() == 3 && num_channels_ == 1)
		cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
	else if (img.channels() == 4 && num_channels_ == 1)
		cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
	else if (img.channels() == 4 && num_channels_ == 3)
		cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
	else if (img.channels() == 1 && num_channels_ == 3)
		cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
	else
		sample = img;

	//cv::imshow("orig", sample);
	//cv::waitKey(0);

	cv::Mat sample_resized;
	if (sample.size() != input_geometry_)
		cv::resize(sample, sample_resized, input_geometry_);
	else
		sample_resized = sample;

	//cv::imshow("resized", sample_resized);
	//cv::waitKey(0);

	cv::Mat sample_float;
	if (num_channels_ == 3)
		sample_resized.convertTo(sample_float, CV_32FC3);
	else
		sample_resized.convertTo(sample_float, CV_32FC1);
	//cv::imshow("float", sample_float);
	//cv::waitKey(0);

	cv::Mat sample_normalized;
	cv::subtract(sample_float, mean_, sample_normalized);
	//cv::imshow("normalized", sample_normalized);
	//cv::waitKey(0);

	/* This operation will write the separate BGR planes directly to the
	 * input layer of the network because it is wrapped by the cv::Mat
	 * objects in input_channels. */
	cv::split(sample_normalized, *input_channels);

	//cv::imshow("channel(0)", input_channels->at(0));
	//cv::waitKey(0);
	//cv::imshow("channel(1)", input_channels->at(1));
	//cv::waitKey(0);
	//cv::imshow("channel(2)", input_channels->at(2));
	//cv::waitKey(0);

	CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
			== net_->input_blobs()[0]->cpu_data())
		<< "Input channels are not wrapping the input layer of the network.";
}

int main(int argc, char** argv) {
	//if (argc != 8) {
	//	std::cerr << "Usage: " << argv[0]
	//		<< " deploy.prototxt network.caffemodel"
	//		<< " mean.binaryproto labels.txt img.jpg" << std::endl;
	//	return 1;
	//}

	//::google::InitGoogleLogging(argv[0]);

	string model_file   = argv[1];
	string trained_file = argv[2];
	string lowrank_model_file = argv[11];
	string lowrank_trained_file = argv[12];
	string mean_file    = argv[3];
	string label_file   = argv[4];
	string file = argv[5];
	int iteration = atoi(argv[6]); // num of iterations
	uint64_t period = atoi(argv[7]); // period in ms
	float slack = atof(argv[8]); // useless
	string config_file = argv[9];
	string Uncertainty_file = argv[10];
	int config_mode = atoi(argv[11]); // 0:config off, 1:config on

	std::cout << "---------- Prediction for "<< file << " ----------" << std::endl;

	cv::Mat img = cv::imread(file, -1);
	CHECK(!img.empty()) << "Unable to decode image " << file;

	//Classifier classifier(model_file, trained_file, mean_file, label_file,config_file,Uncertainty_file,slack, period);
	//Classifier classifier(model_file, trained_file, mean_file, label_file,config_file,Uncertainty_file,slack, period,lowrank_model_file,lowrank_trained_file);
	Classifier classifier(model_file, trained_file, mean_file, label_file,config_file,Uncertainty_file,slack, period,lowrank_model_file,lowrank_trained_file,config_mode);

	int N=5; // Show top 5 results

	// All kinds of preprocess
	Blob<float>* input_layer = classifier.net_->input_blobs()[0];
	input_layer->Reshape(1, classifier.num_channels_,
			classifier.input_geometry_.height, classifier.input_geometry_.width);
	classifier.net_->Reshape();
	classifier.net_->lowrank->Reshape();
	std::vector<cv::Mat> input_channels;
	classifier.WrapInputLayer(&input_channels);
	classifier.Preprocess(img, &input_channels);

	// Init the energy stuff
	//uint64_t start_nj[4], end_nj[4];
	//struct timespec ts;
	//uint64_t race_us, idle_us;
	//double race_mj, idle_mj;
	////dvfs dv(2);
	//energymon em;
	//energymon_get_odroid(&em);
	//em.finit(&em);
	//em.fread(&em, start_nj);

	int repeat = 1; // averaged for accurate purpose

	for(int iter =0; iter < iteration; iter++){
		//dv.set_maxFreq();	
		//em.fread(&em, start_nj);
		//energymon_clock_gettime(&ts);
		for(int r = 0; r < repeat; r++){
			classifier.net_->Forward();
		}
		//usleep(10000);
		//race_us = energymon_gettime_us(&ts);
		//race_us = race_us/repeat;
		//em.fread(&em, end_nj);
		//race_mj = (end_nj[0]-start_nj[0])/1000000.0/repeat;
		//cout << "exec_us: " << race_us << endl;
		//cout << "exec_energy: " << race_mj << endl;
#if 0
		if(race_us < period*1000){
			uint64_t idle = period*1000-race_us;
			//dv.set_idle();
			//em.fread(&em, start_nj);
			//energymon_clock_gettime(&ts);
			for(int r = 0; r < repeat; r++){
				usleep(idle);
			}
			//idle_us = energymon_gettime_us(&ts);
			//idle_us = idle_us/repeat;
			//em.fread(&em, end_nj);
			//idle_mj = (end_nj[0]-start_nj[0])/1000000.0/repeat;
			//cout << "idle_us: " << idle_us << endl;
			//cout << "idle_energy: " << idle_mj << endl;

			std::cout << "++++ " ;
		}else{
			//idle_us = 0;
			//idle_mj = 0;
			//std::cout << "---- " ;
		}
#endif
		//std::cout << "Total_Latency(us)/Energy(mJ): " << race_us + idle_us << " " << race_mj + idle_mj << std::endl;
	}

	//em.ffinish(&em);

	return 0;
	// Resume fast for postprocess	
	//dv.set_maxFreq();	
	
	// All kinds of postprocess
	Blob<float>* output_layer = classifier.net_->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels();
	std::vector<float> output(begin, end);
	N = std::min<int>(classifier.labels_.size(), N);
	std::vector<int> maxN = Argmax(output, N);
	std::vector<Prediction> predictions;
	for (int i = 0; i < N; ++i) {
		int idx = maxN[i];
		predictions.push_back(std::make_pair(classifier.labels_[idx], output[idx]));
	}
	for (size_t i = 0; i < predictions.size(); ++i) {
		Prediction p = predictions[i];
		std::cout << std::fixed << std::setprecision(4) << p.second << " - \""
			<< p.first << "\"" << std::endl;
	}
	return 0;
}
