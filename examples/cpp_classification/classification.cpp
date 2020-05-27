#include <caffe/caffe.hpp>
#ifdef USE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif  // USE_OPENCV

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

#ifdef USE_OPENCV
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
				const string& config_file,
				const string& Uncertainty_file,
				float slack);
		
		Classifier(const string& model_file,
				const string& trained_file,
				const string& mean_file,
				const string& label_file,
				const int batch_size);


		std::vector<Prediction> Classify(const cv::Mat& img, int N = 5);
		std::vector< vector<Prediction> > ClassifyBatch(const vector< cv::Mat > imgs, int num_classes = 5);

	private:
		void SetMean(const string& mean_file);

		std::vector<float> Predict(const cv::Mat& img);
		std::vector<float> PredictBatch(const vector< cv::Mat > imgs);

		void WrapInputLayer(std::vector<cv::Mat>* input_channels);
		void WrapBatchInputLayer(std::vector<std::vector<cv::Mat> > *input_batch);

		void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);
		void PreprocessBatch(const vector<cv::Mat> imgs, std::vector< std::vector<cv::Mat> >* input_batch);

	private:
		shared_ptr<Net<float> > net_;
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
		const string& label_file) {
	#ifdef CPU_ONLY
	Caffe::set_mode(Caffe::CPU);
	#else
	Caffe::set_mode(Caffe::GPU);
	#endif

	_slack = INT_MAX;
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
		float slack) {
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
	net_->setup_DVFS(_slack,config_file,Uncertainty_file);


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

	_slack = INT_MAX;
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

std::vector< vector<Prediction> > Classifier::ClassifyBatch(const vector< cv::Mat > imgs, int num_classes){
	std::vector<float> output_batch = PredictBatch(imgs);
	std::vector< std::vector<Prediction> > predictions;
	for(int j = 0; j < imgs.size(); j++){
		std::vector<float> output(output_batch.begin() + j*num_classes, output_batch.begin() + (j+1)*num_classes);
		std::vector<int> maxN = Argmax(output, num_classes);
		std::vector<Prediction> prediction_single;
		for (int i = 0; i < num_classes; ++i) {
			int idx = maxN[i];
			prediction_single.push_back(std::make_pair(labels_[idx], output[idx]));
		}
		predictions.push_back(std::vector<Prediction>(prediction_single));
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

std::vector< float >  Classifier::PredictBatch(const vector< cv::Mat > imgs) {
	Blob<float>* input_layer = net_->input_blobs()[0];

	input_layer->Reshape(batch_size_, num_channels_,
			input_geometry_.height,
			input_geometry_.width);

	/* Forward dimension change to all layers. */
	net_->Reshape();

	std::vector< std::vector<cv::Mat> > input_batch;
	WrapBatchInputLayer(&input_batch);

	PreprocessBatch(imgs, &input_batch);

	net_->ForwardPrefilled();
	//net_->Forward();

	/* Copy the output layer to a std::vector */
	Blob<float>* output_layer = net_->output_blobs()[0];
	const float* begin = output_layer->cpu_data();
	const float* end = begin + output_layer->channels()*imgs.size();
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

void Classifier::WrapBatchInputLayer(std::vector<std::vector<cv::Mat> > *input_batch){
	Blob<float>* input_layer = net_->input_blobs()[0];

	int width = input_layer->width();
	int height = input_layer->height();
	int num = input_layer->num();
	std::cout << "++++ width: " << width  << std::endl;
	std::cout << "++++ height: " << height  << std::endl;
	std::cout << "++++ nums: " << num  << std::endl;
	float* input_data = input_layer->mutable_cpu_data();
	for ( int j = 0; j < num; j++){
		vector<cv::Mat> input_channels;
		for (int i = 0; i < input_layer->channels(); ++i){
			cv::Mat channel(height, width, CV_32FC1, input_data);
			input_channels.push_back(channel);
			input_data += width * height;
		}
		input_batch -> push_back(vector<cv::Mat>(input_channels));
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

void Classifier::PreprocessBatch(const vector<cv::Mat> imgs,
		std::vector< std::vector<cv::Mat> >* input_batch){
	for (int i = 0 ; i < imgs.size(); i++){
		cv::Mat img = imgs[i];
		std::vector<cv::Mat> *input_channels = &(input_batch->at(i));

		/* Convert the input image to the input image format of the network. */
		cv::Mat sample;
		if (img.channels() == 3 && num_channels_ == 1)
			cv::cvtColor(img, sample, CV_BGR2GRAY);
		else if (img.channels() == 4 && num_channels_ == 1)
			cv::cvtColor(img, sample, CV_BGRA2GRAY);
		else if (img.channels() == 4 && num_channels_ == 3)
			cv::cvtColor(img, sample, CV_BGRA2BGR);
		else if (img.channels() == 1 && num_channels_ == 3)
			cv::cvtColor(img, sample, CV_GRAY2BGR);
		else
			sample = img;

		cv::Mat sample_resized;
		if (sample.size() != input_geometry_)
			cv::resize(sample, sample_resized, input_geometry_);
		else
			sample_resized = sample;

		cv::Mat sample_float;
		if (num_channels_ == 3)
			sample_resized.convertTo(sample_float, CV_32FC3);
		else
			sample_resized.convertTo(sample_float, CV_32FC1);

		cv::Mat sample_normalized;
		cv::subtract(sample_float, mean_, sample_normalized);

		/* This operation will write the separate BGR planes directly to the
		 * input layer of the network because it is wrapped by the cv::Mat
		 * objects in input_channels. */
		cv::split(sample_normalized, *input_channels);
		//        CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
		//              == net_->input_blobs()[0]->cpu_data())
		//          << "Input channels are not wrapping the input layer of the network.";
	}
	//cv::imshow("channel(0)", input_batch->at(0).at(0));
	//cv::waitKey(0);
	//cv::imshow("channel(1)", input_batch->at(0).at(1));
	//cv::waitKey(0);
	//cv::imshow("channel(2)", input_batch->at(0).at(2));
	//cv::waitKey(0);
}

int main(int argc, char** argv) {
	if (argc != 6 && argc !=7 && argc != 9) {
		std::cerr << "Usage: " << argv[0]
			<< " deploy.prototxt network.caffemodel"
			<< " mean.binaryproto labels.txt img.jpg slack freq.config Uncertainty" << std::endl;
		return 1;
	}

	::google::InitGoogleLogging(argv[0]);

	if(argc ==9)
	{
		string model_file   = argv[1];
		string trained_file = argv[2];
		string mean_file    = argv[3];
		string label_file   = argv[4];
		float slack = atof(argv[6]);
		string config_file = argv[7];
		string Uncertainty_file = argv[8];
		Classifier classifier(model_file, trained_file, mean_file, label_file,config_file,Uncertainty_file,slack);

		string file = argv[5];

		std::cout << "---------- Prediction for "
			<< file << " ----------" << std::endl;

		cv::Mat img = cv::imread(file, -1);
		CHECK(!img.empty()) << "Unable to decode image " << file;
		std::vector<Prediction> predictions;
		int iteration = 10;

		//Hush
		energymon em;
		uint64_t start_nj[4], end_nj[4];
		struct timespec ts;
		uint64_t exec_us;
		energymon_get_odroid(&em);
		em.finit(&em);

		
		for (int i=0;i<iteration; i++){
			energymon_clock_gettime(&ts);
			em.fread(&em, start_nj);

			predictions = classifier.Classify(img);

			em.fread(&em, end_nj);
			exec_us = energymon_gettime_us(&ts);
			std::cout << "====> Exec_us: " << exec_us << std::endl;
			std::cout << "====> Energy (mJ): "
				<< (end_nj[0]-start_nj[0])/1000000.0 <<  std::endl;

		}

		//em.fread(&em, end_nj);
		//exec_us = energymon_gettime_us(&ts);

		//std::cout << "====> Exec_us: " << exec_us << std::endl;

		//std::cout << "====> Classify one image (mJ): "
		//	<< (end_nj[0]-start_nj[0])/1000000.0 << "\t"
		//	<< (end_nj[1]-start_nj[1])/1000000.0 << "\t"
		//	<< (end_nj[2]-start_nj[2])/1000000.0 << "\t"
		//	<< (end_nj[3]-start_nj[3])/1000000.0 << std::endl;

		//std::cout << "====> Classify one image (J/s): "
		//	<< (end_nj[0]-start_nj[0])/1000.0/exec_us << "\t"
		//	<< (end_nj[1]-start_nj[1])/1000.0/exec_us << "\t"
		//	<< (end_nj[2]-start_nj[2])/1000.0/exec_us << "\t"
		//	<< (end_nj[3]-start_nj[3])/1000.0/exec_us << std::endl;


		em.ffinish(&em);

		/* Print the top N predictions. */
		for (size_t i = 0; i < predictions.size(); ++i) {
			Prediction p = predictions[i];
			std::cout << std::fixed << std::setprecision(4) << p.second << " - \""
				<< p.first << "\"" << std::endl;
		}
	}
	else if(argc ==6){
		string model_file   = argv[1];
		string trained_file = argv[2];
		string mean_file    = argv[3];
		string label_file   = argv[4];
		Classifier classifier(model_file, trained_file, mean_file, label_file);

		string file = argv[5];

		std::cout << "---------- Prediction for "
			<< file << " ----------" << std::endl;

		cv::Mat img = cv::imread(file, -1);
		CHECK(!img.empty()) << "Unable to decode image " << file;
		std::vector<Prediction> predictions;
		int iteration = 10;

		//Hush
		//energymon em;
		//uint64_t start_nj[4], end_nj[4];
		//struct timespec ts;
		//uint64_t exec_us;

		//energymon_get_odroid(&em);
		//em.finit(&em);
		//energymon_clock_gettime(&ts);
		//em.fread(&em, start_nj);

		for (int i=0;i<iteration; i++){
			predictions = classifier.Classify(img);
		}

		//em.fread(&em, end_nj);
		//exec_us = energymon_gettime_us(&ts);

		//std::cout << "====> Exec_us: " << exec_us << std::endl;

		//std::cout << "====> Classify one image (mJ): "
		//	<< (end_nj[0]-start_nj[0])/1000000.0 << "\t"
		//	<< (end_nj[1]-start_nj[1])/1000000.0 << "\t"
		//	<< (end_nj[2]-start_nj[2])/1000000.0 << "\t"
		//	<< (end_nj[3]-start_nj[3])/1000000.0 << std::endl;

		//std::cout << "====> Classify one image (J/s): "
		//	<< (end_nj[0]-start_nj[0])/1000.0/exec_us << "\t"
		//	<< (end_nj[1]-start_nj[1])/1000.0/exec_us << "\t"
		//	<< (end_nj[2]-start_nj[2])/1000.0/exec_us << "\t"
		//	<< (end_nj[3]-start_nj[3])/1000.0/exec_us << std::endl;


		//em.ffinish(&em);

		/* Print the top N predictions. */
		for (size_t i = 0; i < predictions.size(); ++i) {
			Prediction p = predictions[i];
			std::cout << std::fixed << std::setprecision(4) << p.second << " - \""
				<< p.first << "\"" << std::endl;
		}
	}

	if(argc ==7){
		string model_file   = argv[1];
		string trained_file = argv[2];
		string mean_file    = argv[3];
		string label_file   = argv[4];
		int batch_size = atoi(argv[5]);
		Classifier classifier(model_file, trained_file, mean_file, label_file, batch_size);
		string fileDir = argv[6];
		std::cout << "---------- Prediction for DIRECTORY"
			<< fileDir << " ----------" << std::endl;

		vector<vector<cv::Mat> > batched_images;
		vector<cv::Mat> images;

		DIR *dp;
		string imgPath;
		struct dirent *dirp;
		dp = opendir(fileDir.c_str());
		vector<string> imgNames;
		if(dp == NULL){
			fprintf(stderr, "Invalid DIR!\n");
			return -1;
		}
		int count = 0;
		while((dirp = readdir(dp)) != NULL){
			string str(dirp->d_name);
			if(str.find("JPEG") != std::string::npos){
				imgPath = fileDir + "/" + str;
				imgNames.push_back(imgPath);
				cv::Mat img = cv::imread(imgPath, -1);
				CHECK(!img.empty()) << "----Unable to decode image " << imgPath;
				if(count > 0 && count % batch_size == 0){
					batched_images.push_back(images);
					images.clear();
					count = 0;
				}
				images.push_back(img);
				count++;
			}
		}	
		
		batched_images.push_back(images);
		for (int k =0; k< batched_images.size(); k++){
			std::cout << batched_images[k].size() << std:: endl;
		}

		for (int k =0; k< batched_images.size(); k++){
			std::vector< vector<Prediction> > predictions = classifier.ClassifyBatch(batched_images[k]);
			/* Print the top N predictions. */
			for(int i =0;i< predictions.size();i++){
				vector<Prediction> preds = predictions[i];
				for(int j =0; j< preds.size(); j++){
					Prediction p = preds[j];
					std::cout << std::fixed << std::setprecision(4) << p.second << " - \""
						<< p.first << "\"" << std::endl;

				}
			}
			std::cout << "=======================================" << std::endl;
		}	
	}

	return 0;
}
#else
int main(int argc, char** argv) {
	LOG(FATAL) << "This example requires OpenCV; compile with USE_OPENCV.";
}
#endif  // USE_OPENCV
