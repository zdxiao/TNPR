#ifndef EASYPR_CORE_CHARSIDENTIFY_H_
#define EASYPR_CORE_CHARSIDENTIFY_H_

#include "easypr/util/kv.h"
#include "mxnet/c_predict_api.h"

#include <memory>
#include <opencv2/opencv.hpp>

namespace easypr {

class CharsIdentify {
 public:
  static CharsIdentify* instance();

  std::pair<std::string, std::string> identify(cv::Mat input);
  std::pair<std::string, std::string> identify2(cv::Mat input);
  std::pair<std::string, std::string> identify3(cv::Mat input);
  std::pair<std::string, std::string> identify(cv::Mat input,double& prob);
  std::pair<std::string, std::string> identify2(cv::Mat input,double& prob);
  std::pair<std::string, std::string> identify3(cv::Mat input,double& prob);

 private:
  CharsIdentify();
  ~CharsIdentify();
  static CharsIdentify* instance_;
  cv::Ptr<cv::ml::ANN_MLP> ann_;
  std::shared_ptr<Kv> kv_;
  //mxnet variable
  PredictorHandle out = 0;  // alias for void *
  PredictorHandle out2 = 0;  // alias for void *

  // Parameters
  int dev_type = 1;  // 1: cpu, 2: gpu
  int dev_id = 0;  // arbitrary.
  // Image size and channels
  int width = 20;
  int height = 20;
  int channels = 1;
  // Synset path for your model, you have to modify it
  std::vector<std::string> synset;
  std::vector<std::string> synset2;


};
}

#endif  //  EASYPR_CORE_CHARSIDENTIFY_H_
