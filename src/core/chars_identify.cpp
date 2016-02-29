
/*!
*  Copyright (c) 2016 by Heiyu-tech
* \file demo.cpp
* \brief C++ predict example of chars recognition.
*/

//
//  File: deno.cpp
//  It uses opencv for image reading
//  Created by Jingping Xu on 2016/2/17
//  WeChat: sea942127
//  E-mail: vincent.0812@qq.com
//

#include <stdio.h>

// Path for c_predict_api
#include "mxnet/c_predict_api.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include "easypr/core/chars_identify.h"
#include "easypr/config.h"
#include "easypr/core/core_func.h"



// Read file to buffer
class BufferFile {
public:
	std::string file_path_;
	int length_;
	char* buffer_;

	explicit BufferFile(std::string file_path)
		:file_path_(file_path) {

		std::ifstream ifs(file_path.c_str(), std::ios::in | std::ios::binary);
		if (!ifs) {
			std::cerr << "Can't open the file. Please check " << file_path << ". \n";
			assert(false);
		}

		ifs.seekg(0, std::ios::end);
		length_ = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		std::cout << file_path.c_str() << " ... " << length_ << " bytes\n";

		buffer_ = new char[sizeof(char)* length_];
		ifs.read(buffer_, length_);
		ifs.close();
	}

	int GetLength() {
		return length_;
	}
	char* GetBuffer() {
		return buffer_;
	}

	~BufferFile() {
		delete[] buffer_;
		buffer_ = NULL;
	}
};




void GetMeanFile(cv::Mat im_ori, mx_float* image_data,
	const int channels, const cv::Size resize_size) {
	//

	if (im_ori.empty()) {
		std::cerr << "the input data error. Please check . \n";
		assert(false);
	}

	cv::Mat im;

	resize(im_ori, im, resize_size);

	// Better to be read from a mean.nb file
	float mean = 0;

	//int size = im.rows * im.cols * 3;

	mx_float* ptr_image_r = image_data;

	for (int i = 0; i < im.rows; i++) {
		uchar* data = im.ptr<uchar>(i);

		for (int j = 0; j < im.cols; j++) {

			mx_float r = static_cast<mx_float>(*data++) - mean;

			*ptr_image_r++ = r;
			//       std::cout<<r<<"  ";

		}
		// std::cout<<std::endl;
	}
}




// LoadSynsets
// Code from : https://github.com/pertusa/mxnet_predict_cc/blob/master/mxnet_predict.cc
std::vector<std::string> LoadSynset(const char *filename) {
	std::ifstream fi(filename);

	if (!fi.is_open()) {
		std::cerr << "Error opening file " << filename << std::endl;
		assert(false);
	}

	std::vector<std::string> output;

	std::string synset, lemma;
	while (fi >> synset) {
		getline(fi, lemma);
		output.push_back(lemma);
	}

	fi.close();

	return output;
}



void PrintOutputResult(const std::vector<float>& data, const std::vector<std::string>& synset) {
	if (data.size() != synset.size()) {
		std::cerr << "Result data and synset size does not match!" << std::endl;
	}

	float best_accuracy = 0.0;
	int best_idx = 0;

	for (int i = 0; i < static_cast<int>(data.size()); i++) {
		//    printf("Accuracy[%d] = %.8f\n", i, data[i]);

		if (data[i] > best_accuracy) {
			best_accuracy = data[i];
			best_idx = i;
		}
	}

	printf("Best Result: [%s] id = %d, accuracy = %.8f\n",
		synset[best_idx].c_str(), best_idx, best_accuracy);
}
/************************************************/

namespace easypr {



	CharsIdentify* CharsIdentify::instance_ = nullptr;

	CharsIdentify* CharsIdentify::instance() {
		if (!instance_) {
			instance_ = new CharsIdentify;
		}
		return instance_;
	}

	CharsIdentify::CharsIdentify() {
		//init
		// Models path for your model, you have to modify it
		BufferFile json_data("./models/chars2_net.json");
		BufferFile param_data("./models/chars2_net.params");
		synset = LoadSynset("./models/synset.txt");
		// Parameters
		dev_type = 1;  // 1: cpu, 2: gpu
		dev_id = 0;  // arbitrary.
		// Image size and channels
		width = 20;
		height = 20;
		channels = 1;


		mx_uint num_input_nodes = 1;  // 1 for feedforward
		const char* input_key[1] = { "data" };
		const char** input_keys = input_key;


		const mx_uint input_shape_indptr[2] = { 0, 4 };
		// ( trained_width, trained_height, channel, num)
		const mx_uint input_shape_data[4] = { 1,
			static_cast<mx_uint>(channels),
			static_cast<mx_uint>(width),
			static_cast<mx_uint>(height) };
		out = 0;  // alias for void *

		//-- Create Predictor
		MXPredCreate((const char*)json_data.GetBuffer(),
			(const char*)param_data.GetBuffer(),
			static_cast<size_t>(param_data.GetLength()),
			dev_type,
			dev_id,
			num_input_nodes,
			input_keys,
			input_shape_indptr,
			input_shape_data,
			&out);

		/* Chinese recog*/
		// Models path for your model, you have to modify it
		BufferFile json_data2("./models/charsZH_net.json");
		BufferFile param_data2("./models/charsZH_net.params");
		synset2 = LoadSynset("./models/synset_zh.txt");
		// Parameters
		dev_type = 1;  // 1: cpu, 2: gpu
		dev_id = 0;  // arbitrary.
		// Image size and channels
		width = 20;
		height = 20;
		channels = 1;
		//-- Create Predictor
		MXPredCreate((const char*)json_data2.GetBuffer(),
			(const char*)param_data2.GetBuffer(),
			static_cast<size_t>(param_data.GetLength()),
			dev_type,
			dev_id,
			num_input_nodes,
			input_keys,
			input_shape_indptr,
			input_shape_data,
			&out2);

		//ann_ = ml::ANN_MLP::load<ml::ANN_MLP>(kDefaultAnnPath);
		kv_ = std::shared_ptr<Kv>(new Kv);
		kv_->load("etc/province_mapping");

	}

	CharsIdentify::~CharsIdentify() {
		// Release Predictor
		MXPredFree(out);
		MXPredFree(out2);

	}

	std::pair<std::string, std::string> CharsIdentify::identify(cv::Mat input) {
		cv::Mat feature = features(input, kPredictSize);

		// Just a big enough memory 1000x1000x3
		int image_size = width * height * channels;
		std::vector<mx_float> image_data = std::vector<mx_float>(image_size);
		GetMeanFile(input, image_data.data(), channels, cv::Size(width, height));
		 //-- Set Input Image
       MXPredSetInput(out, "data", image_data.data(), image_size);
	   //-- Do Predict Forward
	   MXPredForward(out);

	   mx_uint output_index = 0;

	   mx_uint *shape = 0;
	   mx_uint shape_len;

	   //-- Get Output Result
	   MXPredGetOutputShape(out, output_index, &shape, &shape_len);

	   size_t size = 1;
	   for (mx_uint i = 0; i < shape_len; ++i) size *= shape[i];

	   std::vector<float> data(size);

	   MXPredGetOutput(out, output_index, &(data[0]), size);

	   float best_accuracy = 0.0;
	   int best_idx = 0;

	   for (int i = 0; i < static_cast<int>(data.size()); i++) {
		   //    printf("Accuracy[%d] = %.8f\n", i, data[i]);

		   if (data[i] > best_accuracy) {
			   best_accuracy = data[i];
			   best_idx = i;
		   }
	   }

           auto index=static_cast<int>(best_idx);
           return std::make_pair(kChars[index], kChars[index]);
	}
/************************************************************************/



std::pair<std::string, std::string> CharsIdentify::identify(cv::Mat input, double& prob) {
		cv::Mat feature = features(input, kPredictSize);

		// Just a big enough memory 1000x1000x3
		int image_size = width * height * channels;
		std::vector<mx_float> image_data = std::vector<mx_float>(image_size);
		GetMeanFile(input, image_data.data(), channels, cv::Size(width, height));
		 //-- Set Input Image
       MXPredSetInput(out, "data", image_data.data(), image_size);
	   //-- Do Predict Forward
	   MXPredForward(out);

	   mx_uint output_index = 0;

	   mx_uint *shape = 0;
	   mx_uint shape_len;

	   //-- Get Output Result
	   MXPredGetOutputShape(out, output_index, &shape, &shape_len);

	   size_t size = 1;
	   for (mx_uint i = 0; i < shape_len; ++i) size *= shape[i];

	   std::vector<float> data(size);

	   MXPredGetOutput(out, output_index, &(data[0]), size);

	   float best_accuracy = 0.0;
	   int best_idx = 0;

	   for (int i = 0; i < static_cast<int>(data.size()); i++) {
		   //    printf("Accuracy[%d] = %.8f\n", i, data[i]);

		   if (data[i] > best_accuracy) {
			   best_accuracy = data[i];
			   best_idx = i;
		   }
	   }

           prob=best_accuracy;
           auto index=static_cast<int>(best_idx);
           return std::make_pair(kChars[index], kChars[index]);
	}



/*************************************************************************/
	std::pair<std::string, std::string> CharsIdentify::identify2(cv::Mat input) {
		cv::Mat feature = features(input, kPredictSize);

		// Just a big enough memory 1000x1000x3
		int image_size = width * height * channels;
		std::vector<mx_float> image_data = std::vector<mx_float>(image_size);
		GetMeanFile(input, image_data.data(), channels, cv::Size(width, height));
		//-- Set Input Image
		MXPredSetInput(out2, "data", image_data.data(), image_size);
		//-- Do Predict Forward
		MXPredForward(out2);

		mx_uint output_index = 0;

		mx_uint *shape = 0;
		mx_uint shape_len;

		//-- Get Output Result
		MXPredGetOutputShape(out2, output_index, &shape, &shape_len);

		size_t size = 1;
		for (mx_uint i = 0; i < shape_len; ++i) size *= shape[i];

		std::vector<float> data(size);

		MXPredGetOutput(out2, output_index, &(data[0]), size);

		float best_accuracy = 0.0;
		int best_idx = 0;

		for (int i = 0; i < static_cast<int>(data.size()); i++) {
			//    printf("Accuracy[%d] = %.8f\n", i, data[i]);
			if (data[i] > best_accuracy) {
				best_accuracy = data[i];
				best_idx = i;
			}
		}
			       // std::cout<<synset[best_idx].c_str();
                   // best_idx=data.size();
                    best_idx=best_idx>31?31:best_idx;
               //     int index=static_cast<int>(27+34);//best_idx+34;
                    int index=static_cast<int>(best_idx+34);//best_idx+34;
		    const char* key = kChars[index];
		    std::string s = key;
		    std::string province = kv_->get(s);
		    return std::make_pair(s, province);
		return std::make_pair(province,province);
	}

/***********************************************************************************/


std::pair<std::string, std::string> CharsIdentify::identify2(cv::Mat input,double& prob) {
		cv::Mat feature = features(input, kPredictSize);

		// Just a big enough memory 1000x1000x3
		int image_size = width * height * channels;
		std::vector<mx_float> image_data = std::vector<mx_float>(image_size);
		GetMeanFile(input, image_data.data(), channels, cv::Size(width, height));
		//-- Set Input Image
		MXPredSetInput(out2, "data", image_data.data(), image_size);
		//-- Do Predict Forward
		MXPredForward(out2);

		mx_uint output_index = 0;

		mx_uint *shape = 0;
		mx_uint shape_len;

		//-- Get Output Result
		MXPredGetOutputShape(out2, output_index, &shape, &shape_len);

		size_t size = 1;
		for (mx_uint i = 0; i < shape_len; ++i) size *= shape[i];

		std::vector<float> data(size);

		MXPredGetOutput(out2, output_index, &(data[0]), size);

		float best_accuracy = 0.0;
		int best_idx = 0;

		for (int i = 0; i < static_cast<int>(data.size()); i++) {
			//    printf("Accuracy[%d] = %.8f\n", i, data[i]);
			if (data[i] > best_accuracy) {
				best_accuracy = data[i];
				best_idx = i;
			}
		}
                    prob=best_accuracy;
			       // std::cout<<synset[best_idx].c_str();
                   // best_idx=data.size();
                    best_idx=best_idx>31?31:best_idx;
               //     int index=static_cast<int>(27+34);//best_idx+34;
                    int index=static_cast<int>(best_idx+34);//best_idx+34;
		    const char* key = kChars[index];
		    std::string s = key;
		    std::string province = kv_->get(s);
		    //return std::make_pair(s, province);

		return std::make_pair(province,province);
	}
/**************************************/

 std::pair<std::string, std::string> CharsIdentify::identify3(cv::Mat input) {
		cv::Mat feature = features(input, kPredictSize);

		// Just a big enough memory 1000x1000x3
		int image_size = width * height * channels;
		std::vector<mx_float> image_data = std::vector<mx_float>(image_size);
		GetMeanFile(input, image_data.data(), channels, cv::Size(width, height));
		 //-- Set Input Image
       MXPredSetInput(out, "data", image_data.data(), image_size);
	   //-- Do Predict Forward
	   MXPredForward(out);

	   mx_uint output_index = 0;

	   mx_uint *shape = 0;
	   mx_uint shape_len;

	   //-- Get Output Result
	   MXPredGetOutputShape(out, output_index, &shape, &shape_len);

	   size_t size = 1;
	   for (mx_uint i = 0; i < shape_len; ++i) size *= shape[i];

	   std::vector<float> data(size);

	   MXPredGetOutput(out, output_index, &(data[0]), size);

	   float best_accuracy = 0.0;
	   int best_idx = 0;

	   for (int i = 0; i < static_cast<int>(data.size()); i++) {
		   //    printf("Accuracy[%d] = %.8f\n", i, data[i]);
                if(i==0||i>9){
		   if (data[i] > best_accuracy) {
			   best_accuracy = data[i];
			   best_idx = i;
		   }
                }
	   }
           auto index=static_cast<int>(best_idx);
           index=index==0?65:index;
           return std::make_pair(kChars[index], kChars[index]);
	}

/***************************************/
 std::pair<std::string, std::string> CharsIdentify::identify3(cv::Mat input,double &prob) {
		cv::Mat feature = features(input, kPredictSize);

		// Just a big enough memory 1000x1000x3
		int image_size = width * height * channels;
		std::vector<mx_float> image_data = std::vector<mx_float>(image_size);
		GetMeanFile(input, image_data.data(), channels, cv::Size(width, height));
		 //-- Set Input Image
       MXPredSetInput(out, "data", image_data.data(), image_size);
	   //-- Do Predict Forward
	   MXPredForward(out);

	   mx_uint output_index = 0;

	   mx_uint *shape = 0;
	   mx_uint shape_len;

	   //-- Get Output Result
	   MXPredGetOutputShape(out, output_index, &shape, &shape_len);

	   size_t size = 1;
	   for (mx_uint i = 0; i < shape_len; ++i) size *= shape[i];

	   std::vector<float> data(size);

	   MXPredGetOutput(out, output_index, &(data[0]), size);

	   float best_accuracy = 0.0;
	   int best_idx = 0;

	   for (int i = 0; i < static_cast<int>(data.size()); i++) {
		   //    printf("Accuracy[%d] = %.8f\n", i, data[i]);
                if(i==0||i>9){
		   if (data[i] > best_accuracy) {
			   best_accuracy = data[i];
			   best_idx = i;
		   }
                }
	   }
         prob=best_accuracy;

           auto index=static_cast<int>(best_idx);
           index=index==0?65:index;
           return std::make_pair(kChars[index], kChars[index]);
	}
}
