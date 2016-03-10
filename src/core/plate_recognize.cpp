#include "easypr/core/plate_recognize.h"
#include "easypr/config.h"

int realPlateCounter = 0;
//#define DEBUG_SAVE_PLATES


namespace easypr {

CPlateRecognize::CPlateRecognize() { }

// !车牌识别模块

int CPlateRecognize::plateRecognize(Mat src,
                                    std::vector<std::string> &licenseVec) {

  // 车牌方块集合

  std::vector<CPlate> plateVec;

  // 进行深度定位，使用颜色信息与二次Sobel

  int resultPD = plateDetect(src, plateVec, kDebug, 0);

  if (resultPD == 0) {
    size_t num = plateVec.size();
    int index = 0;

    //依次识别每个车牌内的符号
    double prob=0.0;
    std::string license;
    for (size_t j = 0; j < num; j++) {
      CPlate item = plateVec[j];
      Mat plate = item.getPlateMat();

      //获取车牌颜色

      std::string plateType = getPlateColor(plate);

      //获取车牌号

      std::string plateIdentify = "";
      double tmp=0.0;

    #ifndef DEBUG_SAVE_PLATES
      if(getPDDebug())
      {
        imshow("plate", plate);
        std::cout << license << std::endl;
        waitKey();
      }
    #endif

      int resultCR = charsRecognise(plate, plateIdentify,tmp);
      if (resultCR == 0) {

        std::string licensetmp = plateType + ":" + plateIdentify;
        if(tmp>prob) {prob=tmp;license=licensetmp;}
      }
    }
   if(prob>0.01) licenseVec.push_back(license);
    //完整识别过程到此结束

    //如果是Debug模式，则还需要将定位的图片显示在原图左上角

    if (getPDDebug()) {
      Mat result;
      src.copyTo(result);

      for (size_t j = 0; j < num; j++) {
        CPlate item = plateVec[j];
        Mat plate = item.getPlateMat();

        int height = 36;
        int width = 136;
        if (height * index + height < result.rows) {
          Mat imageRoi = result(Rect(0, 0 + height * index, width, height));
          addWeighted(imageRoi, 0, plate, 1, 0, imageRoi);
        }
        index++;

        RotatedRect minRect = item.getPlatePos();
        Point2f rect_points[4];
        minRect.points(rect_points);

        Scalar lineColor = Scalar(255, 255, 255);

        if (item.getPlateLocateType() == SOBEL) lineColor = Scalar(255, 0, 0);

        if (item.getPlateLocateType() == COLOR) lineColor = Scalar(0, 255, 0);

        for (int j = 0; j < 4; j++)
          line(result, rect_points[j], rect_points[(j + 1) % 4], lineColor, 2,
               8);
      }

      //显示定位框的图片

      showResult(result);
    }
  }

  return resultPD;
}

/*************************************/



// !车牌识别模块

int CPlateRecognize::plateRecognize(Mat src,
                                    std::vector<std::string> &licenseVec, CPlate& plateout) {

  // 车牌方块集合

  std::vector<CPlate> plateVec;

  // 进行深度定位，使用颜色信息与二次Sobel

  int resultPD = plateDetect(src, plateVec, kDebug, 0);

  if (resultPD == 0) {
    size_t num = plateVec.size();
    int index = 0;

    //依次识别每个车牌内的符号
    double prob=0.0;
    std::string license;
    for (size_t j = 0; j < num; j++) {
      CPlate item = plateVec[j];
      Mat plate = item.getPlateMat();

      //获取车牌颜色

      std::string plateType = getPlateColor(plate);

      //获取车牌号

      std::string plateIdentify = "";
      double tmp=0.0;
      int resultCR = charsRecognise(plate, plateIdentify,tmp);
      if (resultCR == 0) {
        std::string licensetmp = plateType + ":" + plateIdentify;
        if(tmp>prob&&plateIdentify.size()==9) {prob=tmp;license=licensetmp;plateout=item;}
      }
    }
   if(prob>0.01) licenseVec.push_back(license);
    //完整识别过程到此结束

    //如果是Debug模式，则还需要将定位的图片显示在原图左上角

    if (getPDDebug()) {
      Mat result;
      src.copyTo(result);

      for (size_t j = 0; j < num; j++) {
        CPlate item = plateVec[j];
        Mat plate = item.getPlateMat();

        int height = 36;
        int width = 136;
        if (height * index + height < result.rows) {
          Mat imageRoi = result(Rect(0, 0 + height * index, width, height));
          addWeighted(imageRoi, 0, plate, 1, 0, imageRoi);
        }
        index++;

        RotatedRect minRect = item.getPlatePos();
        Point2f rect_points[4];
        minRect.points(rect_points);

        Scalar lineColor = Scalar(255, 255, 255);

        if (item.getPlateLocateType() == SOBEL) lineColor = Scalar(255, 0, 0);

        if (item.getPlateLocateType() == COLOR) lineColor = Scalar(0, 255, 0);

        for (int j = 0; j < 4; j++)
          line(result, rect_points[j], rect_points[(j + 1) % 4], lineColor, 2,
               8);
      }

      //显示定位框的图片

      showResult(result);
    }
  }

  return resultPD;
}





int CPlateRecognize::plateRecognize(Mat src,
                                    std::vector<std::string> &licenseVec, CPlate& plateout, std::vector<Mat> &matChar) {

  // 车牌方块集合

  std::vector<CPlate> plateVec;

  // 进行深度定位，使用颜色信息与二次Sobel

  int resultPD = plateDetect(src, plateVec, kDebug, 0);
  std::cout<<plateVec.size()<<std::endl;

  if (resultPD == 0) {
    size_t num = plateVec.size();
    int index = 0;

    //依次识别每个车牌内的符号
    double prob=0.0;
    std::string license;
    for (size_t j = 0; j < num; j++) {
      CPlate item = plateVec[j];
      Mat plate = item.getPlateMat();

      //获取车牌颜色

      std::string plateType = getPlateColor(plate);

      //获取车牌号

      std::string plateIdentify = "";
      double tmp=0.0;
      std::vector<Mat> matChars;
      int resultCR = charsRecognise(plate, plateIdentify,tmp,matChars);
      // int resultCR = charsRecognise(plate, plateIdentify,tmp);
      if (resultCR == 0) {
        std::string licensetmp = plateType + ":" + plateIdentify;
        if(tmp>=prob&&plateIdentify.size()==9) {prob=tmp;license=licensetmp;plateout=item;matChar=matChars;}
      }
    }
   if(prob>0.01){
    licenseVec.push_back(license);
    //完整识别过程到此结束

    //如果是Debug模式，则还需要将定位的图片显示在原图左上角

    if (getPDDebug()) {
      std::cout<<license<<std::endl;
      Mat result;
      src.copyTo(result);

      // for (size_t j = 0; j < num; j++) {
        CPlate item = plateout;
        Mat plate = item.getPlateMat();

        int height = 36;
        int width = 136;
        if (height * index + height < result.rows) {
          Mat imageRoi = result(Rect(0, 0 + height * index, width, height));
          addWeighted(imageRoi, 0, plate, 1, 0, imageRoi);
        }
        index++;

        RotatedRect minRect = item.getPlatePos();
        Point2f rect_points[4];
        minRect.points(rect_points);

        Scalar lineColor = Scalar(255, 255, 255);

        if (item.getPlateLocateType() == SOBEL) lineColor = Scalar(255, 0, 0);

        if (item.getPlateLocateType() == COLOR) lineColor = Scalar(0, 255, 0);

        for (int j = 0; j < 4; j++)
          line(result, rect_points[j], rect_points[(j + 1) % 4], lineColor, 2,
               8);
      // }

      //显示定位框的图片

        showResult(result);
      }
    }else{
       ///////////////////////////////////////////////
 /////////////maogeng///////////////////////////
      std::vector<CPlate> all_Plates;
      if (plateVec.size())
      {
        Mat src_clone = src.clone();
        src_clone.setTo(Scalar(0,0,0));
        for(size_t i = 0; i < plateVec.size(); i++){
          Rect rect = plateVec[i].getPlatePos().boundingRect();
          Rect_<float> safeBoundRect;
          bool isFormRect = calcSafeRect(plateVec[i].getPlatePos(), src, safeBoundRect);
          src_clone(safeBoundRect)=Scalar(255,255,255);
          // imshow("src_clone",src_clone);
          // waitKey();
        }
        Mat mat_gray;
        cvtColor(src_clone, mat_gray, CV_BGR2GRAY);
        threshold(mat_gray, mat_gray, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);

        std::vector<std::vector<Point> > contours;
        findContours(mat_gray,contours,               // a vector of contours
                     CV_RETR_EXTERNAL,
                     CV_CHAIN_APPROX_NONE);
        std::cout<<contours.size()<<std::endl;
        for(std::vector<std::vector<Point> >::iterator itc = contours.begin();itc!=contours.end();itc++){
          CPlate plate;
          Mat plate_mat;
          plate_mat.create(36, 136, CV_8UC3);
          RotatedRect mr = minAreaRect(Mat(*itc));

          Rect_<float> safeBoundRect;
          bool isFormRect = calcSafeRect(mr, src, safeBoundRect);

          resize(src(safeBoundRect), plate_mat, plate_mat.size(), 0, 0, INTER_AREA);
          plate.setPlateMat(plate_mat);
          plate.setPlatePos(mr);
          all_Plates.push_back(plate);
        }
        double prob=0.0;
        std::string license;
        for (size_t j = 0; j < all_Plates.size(); j++) {
          CPlate item = all_Plates[j];
          Mat plate = item.getPlateMat();

          //获取车牌颜色

          std::string plateType = getPlateColor(plate);

          //获取车牌号

          std::string plateIdentify = "";
          double tmp=0.0;
          std::vector<Mat> matChars;
          int resultCR = charsRecognise(plate, plateIdentify,tmp,matChars);
          // int resultCR = charsRecognise(plate, plateIdentify,tmp);
          if (resultCR == 0) {
            std::string licensetmp = plateType + ":" + plateIdentify;
            if(tmp>=prob&&plateIdentify.size()==9) {prob=tmp;license=licensetmp;plateout=item;matChar=matChars;}
          }
        }

       if(prob>0.01){
        licenseVec.push_back(license);
        //完整识别过程到此结束

        //如果是Debug模式，则还需要将定位的图片显示在原图左上角

        if (getPDDebug()) {
          std::cout<<license<<std::endl;
          Mat result;
          src.copyTo(result);

          // for (size_t j = 0; j < num; j++) {
            CPlate item = plateout;
            Mat plate = item.getPlateMat();

            int height = 36;
            int width = 136;
            if (height * index + height < result.rows) {
              Mat imageRoi = result(Rect(0, 0 + height * index, width, height));
              addWeighted(imageRoi, 0, plate, 1, 0, imageRoi);
            }
            index++;

            RotatedRect minRect = item.getPlatePos();
            Point2f rect_points[4];
            minRect.points(rect_points);

            Scalar lineColor = Scalar(255, 255, 255);

            if (item.getPlateLocateType() == SOBEL) lineColor = Scalar(255, 0, 0);

            if (item.getPlateLocateType() == COLOR) lineColor = Scalar(0, 255, 0);

            for (int j = 0; j < 4; j++)
              line(result, rect_points[j], rect_points[(j + 1) % 4], lineColor, 2,
                   8);
          // }

          //显示定位框的图片

            showResult(result);
          }
        } 
      }
 /////////////maogeng///////////////////////////
 ///////////////////////////////////////////////

    }
  }

  return resultPD;
}





}
