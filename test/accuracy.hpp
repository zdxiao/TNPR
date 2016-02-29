#ifndef EASYPR_ACCURACY_HPP
#define EASYPR_ACCURACY_HPP

#include <easypr.h>
#include <ctime>
#include <fstream>
#include <list>
#include <memory>

using namespace std;

namespace easypr {

namespace demo {

int accuracyTest(const char* test_path) {
  std::shared_ptr<easypr::Kv> kv(new easypr::Kv);
  kv->load("etc/chinese_mapping");

  auto files = Utils::getFiles(test_path);

  CPlateRecognize pr;

  // 设置Debug模式

  pr.setDebug(false);

  pr.setLifemode(false);

  // 设置要处理的一张图片中最多有多少车牌

  pr.setMaxPlates(4);

  int size = files.size();

  if (0 == size) {
    cout << "No File Found in general_test/native_test!" << endl;
    return 0;
  }

  cout << "Begin to test the easypr accuracy!" << endl;

  // 总的测试图片数量

  int count_all = 0;

  // 错误的图片数量

  int count_err = 0;

  // 未识别的图片数量

  int count_norecogin = 0;

  std::list<std::string> not_recognized_files;

  std::list<std::string> reco_error_files;

  // 总的字符差距

  float diff_all = 0;

  // 平均字符差距

  float diff_avg = 0;

  // 完全匹配的识别次数

  float match_count = 0;

  // 完全匹配的识别次数所占识别图片中的比例

  float match_rate = 0;

  // 开始和结束时间

  time_t begin, end;
  time(&begin);

  for (int i = 0; i < size; i++) {
    string filepath = files[i].c_str();

    // EasyPR开始判断车牌

    Mat src = imread(filepath);

    // 如果是非图像文件，直接过去

    if (!src.data) continue;

    cout << "------------------" << endl;

    // 获取真实的车牌

    string plateLicense = Utils::getFileName(filepath);
    cout << kv->get("original_plate") << ":" << plateLicense << endl;

    vector<string> plateVec;
    /**
    int result = pr.plateRecognize(src, plateVec);
*/
    CPlate plateout;
    std::vector<Mat> matChar;
    int result = pr.plateRecognize(src, plateVec, plateout, matChar);

  //  int result = pr.plateRecognize(src, plateVec);


    if (result == 0) {
      int num = plateVec.size();

      if (num == 0) {
        char buffer[50];
        string filename = filepath.substr(filepath.find_last_of("/")+1,filepath.find_last_of("."));
        cout << kv->get("empty_plate") << endl;
        // cout << plateLicense << endl;
        sprintf(buffer, "resources/image/result/none/%s", filename.c_str());
        // imshow("test",src);
        // waitKey();
        if (plateLicense != kv->get("empty_plate")) {
          not_recognized_files.push_back(filepath);
          count_norecogin++;
        }
        utils::imwrite(buffer, src);
      } else if (num > 1) {

        // 多车牌使用diff最小的那个记录

        int mindiff = 10000;
        for (int j = 0; j < num; j++) {
          cout << plateVec[j] << " (" << j + 1 << ")" << endl;
          string colorplate = plateVec[j];

          // 计算"蓝牌:苏E7KU22"中冒号后面的车牌大小"

          vector<string> spilt_plate = Utils::splitString(colorplate, ':');

          int size = spilt_plate.size();
          if (size == 2 && spilt_plate[1] != "") {
            int diff = utils::levenshtein_distance(plateLicense,
                                                   spilt_plate[size - 1]);
            if (diff < mindiff) mindiff = diff;
          }
        }

        cout << kv->get("diff") << ":" << mindiff << kv->get("char") << endl;
        if (mindiff == 0) {

          // 完全匹配

          match_count++;
        }
        else
        {
            reco_error_files.push_back(filepath);
        }
        diff_all = diff_all + mindiff;
      } else {

        // 单车牌只计算一次diff

        for (int j = 0; j < num; j++) {
          cout << plateVec[j] << endl;
          string colorplate = plateVec[j];

          // 计算"蓝牌:苏E7KU22"中冒号后面的车牌大小"

          vector<string> spilt_plate = Utils::splitString(colorplate, ':');

          int size = spilt_plate.size();
          if (size == 2 && spilt_plate[1] != "") {

           std::string str1=plateLicense.substr(3);
           std::string str2=spilt_plate[size - 1].substr(3);
          // strstr1.substr(3);
           int diff = utils::levenshtein_distance(str1,str2);
            cout << kv->get("diff") << ":" << diff << kv->get("char") << endl;

            if (diff == 0) {

              // 完全匹配

              match_count++;
            }
            else
            {
                reco_error_files.push_back(filepath);
            }

/////////////////////maogeng////////////////////////////////////////////////////
          char buffer[50];

          string filename = filepath.substr(filepath.find_last_of("/")+1,filepath.find_last_of("."));
          if(diff == 0)
          {
            sprintf(buffer, "resources/image/result/correct/%s_%s.jpg", filename.c_str(), colorplate.c_str());
          }
          else
          {
            sprintf(buffer, "resources/image/result/wrong/%s_%s.jpg", filename.c_str(), colorplate.c_str());
          }

          Mat src_clone = src.clone();
          Mat plate_resize = plateout.getPlateMat();
          resize(plate_resize, plate_resize, Size(), 2, 2, CV_INTER_AREA);
          src_clone(Rect(src.cols-400,10, 300,120)) = Scalar(255,255,255);
          Mat imageOut = src_clone(Rect(src.cols-400+10,10+2,plate_resize.cols,plate_resize.rows));
          addWeighted(imageOut, 0, plate_resize, 1, 0, imageOut);
          src_clone(Rect(src.cols-400,60+plate_resize.rows, 300,70)) = Scalar(255,0,0);

          for(size_t char_i = 0;char_i<matChar.size();char_i++){
            resize(matChar[char_i], matChar[char_i], Size(), 2, 2, CV_INTER_AREA);
            Mat imageChar = src_clone(Rect(src.cols-400+(matChar[char_i].cols+2)*char_i,90,matChar[char_i].cols,matChar[char_i].rows));
            cvtColor(matChar[char_i],matChar[char_i],CV_GRAY2BGR);
            matChar[char_i].copyTo(imageChar);
            // imshow("matChar",src_clone);
            // waitKey();
          }
          // cout<<matChar[0].rows<<" "<<matChar[0].cols<<endl;
          putText( src_clone, "["+spilt_plate[1].substr(3,spilt_plate[1].size()-3)+"]", Point( src.cols-400,40+plate_resize.rows*2),CV_FONT_HERSHEY_COMPLEX, 2, Scalar(255, 255, 255), 4 );
          // imshow("test",src_clone);
          // waitKey();
          utils::imwrite(buffer, src_clone);
/////////////////////maogeng////////////////////////////////////////////////////

            diff_all = diff_all + diff;
          }
        }
      }
    } else {
      cout << kv->get("error_code") << ":" << result << endl;
      count_err++;
    }
    count_all++;
  }
  time(&end);

  cout << "------------------" << endl;
  cout << "Easypr accuracy test end!" << endl;
  cout << "------------------" << endl;
  cout << endl;
  cout << kv->get("summaries") << ":" << endl;
  cout << kv->get("sum_pictures") << ":" << count_all << ",  ";
  cout << kv->get("unrecognized") << ":" << count_norecogin << ",  ";

  float count_recogin = float(count_all - (count_err + count_norecogin));
  float count_rate = count_recogin / count_all;
  cout << kv->get("locate_rate") << ":" << count_rate * 100 << "%  " << endl;

  if (count_recogin > 0) {
    diff_avg = diff_all / count_recogin;
  }

  if (count_recogin > 0) {
    match_rate = match_count / count_recogin * 100;
  }

  cout << kv->get("diff_average") << ":" << diff_avg << ",  ";
  cout << kv->get("full_match") << ":" << match_count << ",  ";
  cout << kv->get("full_rate") << ":" << match_rate << "%  " << endl;

  double seconds = difftime(end, begin);
  double avgsec = seconds / double(count_all);

  cout << kv->get("seconds") << ":" << seconds << kv->get("sec") << ",  ";
  cout << kv->get("seconds_average") << ":" << avgsec << kv->get("sec") << endl;

  cout << kv->get("unrecognized") << ":" << endl;

  for (auto it = not_recognized_files.begin(); it != not_recognized_files.end();
       ++it) {
    cout << *it << endl;
  }

  cout << endl;

  cout << "------------------" << endl;

  ofstream myfile("accuracy.txt", ios::app);
  if (myfile.is_open()) {
    time_t t = time(0);  // get time now
    struct tm* now = localtime(&t);
    char buf[80];

    strftime(buf, sizeof(buf), "%Y-%m-%d %X", now);
    myfile << string(buf) << endl;

    myfile << kv->get("sum_pictures") << ":" << count_all << ",  ";
    myfile << kv->get("unrecognized") << ":" << count_norecogin << ",  ";
    myfile << kv->get("locate_rate") << ":" << count_rate * 100 << "%  "
        << endl;
    myfile << kv->get("diff_average") << ":" << diff_avg << ",  ";
    myfile << kv->get("full_match") << ":" << match_count << ",  ";
    myfile << kv->get("full_rate") << ":" << match_rate << "%  " << endl;
    myfile << kv->get("seconds") << ":" << seconds << kv->get("sec") << ",  ";
    myfile << kv->get("seconds_average") << ":" << avgsec << kv->get("sec")
        << endl;

    for (auto it = not_recognized_files.begin(); it != not_recognized_files.end();
       ++it) {
        myfile << *it << endl;
    }
    myfile << "-------------------------------------------" << endl;
    for (auto it = reco_error_files.begin(); it != reco_error_files.end();
       ++it) {
        myfile << *it << endl;
    }
    myfile.close();
  } else {
    cout << "Unable to open file";
  }
  return 0;
}

}
}

#endif  // EASYPR_ACCURACY_HPP
