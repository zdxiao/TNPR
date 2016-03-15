#include "easypr/core/plate_detect.h"
#include "easypr/util/util.h"
#include "easypr/core/xzdImageProcess.h"

// #define DEBUG
int locatedPlatesCounter = 0;

namespace easypr {

CPlateDetect::CPlateDetect() {
  m_plateLocate = new CPlateLocate();
  // 默认EasyPR在一幅图中定位最多3个车
  m_maxPlates = 3;
}

CPlateDetect::~CPlateDetect() { SAFE_RELEASE(m_plateLocate); }

int CPlateDetect::plateDetect(Mat src, std::vector<CPlate> &resultVec,
                              bool showDetectArea, int index) {
  std::vector<CPlate> color_Plates;
  std::vector<CPlate> sobel_Plates;
  std::vector<CPlate> color_result_Plates;
  std::vector<CPlate> sobel_result_Plates;

  std::vector<CPlate> all_result_Plates;

  //如果颜色查找找到n个以上（包含n个）的车牌，就不再进行Sobel查找了。

  const int color_find_max = m_maxPlates;

  m_plateLocate->plateColorLocate(src, color_Plates, index);
  expandPlate(src,color_Plates);
  color_result_Plates = color_Plates;


  PlateJudge::instance()->plateJudge(color_Plates, color_result_Plates);


  #ifdef DEBUG_LOCATE_PLATE
  for(auto iter = color_result_Plates.begin(); iter != color_result_Plates.end(); ++iter)
  {
    #ifndef DEBUG_SAVE_LOCATE_PLATES
    imshow("ColorLocatedPlate", (*iter).getPlateMat());
    waitKey();
    #else
    char buffer[50];
    sprintf(buffer, "resources/image/locatePlates0224/%04d.jpg", locatedPlatesCounter);
    utils::imwrite(buffer, (*iter).getPlateMat());
    locatedPlatesCounter++;
    #endif
  }
  #endif


  for (size_t i = 0; i < color_result_Plates.size(); i++) {
    CPlate plate = color_result_Plates[i];

    // #ifdef DEBUG
    // imshow("ColorLocatedPlate", plate.getPlateMat());
    // waitKey();
    // #endif

    plate.setPlateLocateType(COLOR);
    all_result_Plates.push_back(plate);
  }

  // Mat src_process = src;

  //颜色和边界闭操作同时采用

  {
    m_plateLocate->plateSobelLocate(src, sobel_Plates, index);
    expandPlate(src,sobel_Plates);
    sobel_result_Plates = sobel_Plates;

    PlateJudge::instance()->plateJudge(sobel_Plates, sobel_result_Plates);

    for (size_t i = 0; i < sobel_result_Plates.size(); i++) {
      CPlate plate = sobel_result_Plates[i];

      // #ifdef DEBUG
      // imshow("SobelLocatedPlate", plate.getPlateMat());
      // waitKey();
      // #endif

      plate.bColored = false;
      plate.setPlateLocateType(SOBEL);
      all_result_Plates.push_back(plate);
    }
  }
 ///////////////////////////////////////////////
 /////////////maogeng///////////////////////////
  // std::vector<CPlate> all_Plates;
  // std::vector<int> flag(all_result_Plates.size(),1);

  // if (all_result_Plates.size()>1)
  // {
  //   for (std::vector<CPlate>::iterator iter = all_result_Plates.begin(); iter != all_result_Plates.end(); ++iter)
  //   {
  //     for (std::vector<CPlate>::iterator iter_inner = iter+1; iter_inner != all_result_Plates.end(); ++iter_inner)
  //     {
  //       CPlate plate;
  //       Rect rect1 = iter->getPlatePos().boundingRect();
  //       Rect rect2 = iter_inner->getPlatePos().boundingRect();
  //       float ratio = 0.95;
  //       int result = plateUnion(rect1, rect2, ratio);
  //       if (result==1)
  //       {
  //         flag[iter_inner - all_result_Plates.begin()] = 0;
  //       }
  //       else if(result == 2){
  //         flag[iter - all_result_Plates.begin()] = 0;
  //       }
  //     }
  //   }

  //   for (int i = 0; i < flag.size(); ++i)
  //   {
  //     if(flag[i]==1){
  //       all_Plates.push_back(all_result_Plates[i]);
  //     }
  //   }

  // }else{
  //   all_Plates = all_result_Plates;
  // }

 /////////////maogeng///////////////////////////
 ///////////////////////////////////////////////

  for (size_t i = 0; i < all_result_Plates.size(); i++) {

    // 把截取的车牌图像依次放到左上角
    CPlate plate = all_result_Plates[i];
    // Mat src_clone = src.clone();
    // // Rect rect = plate.getPlatePos().boundingRect();
    // Rect_<float> safeBoundRect;
    // bool isFormRect = calcSafeRect(plate.getPlatePos(), src, safeBoundRect);
    // // std::cout<<"safeBoundRect:"<<safeBoundRect.x<<":"<<safeBoundRect.y<<std::endl;
    // Rect rect = safeBoundRect;
    // // src_clone(safeBound.size()<<std::endl;
    // int width = rect.width;
    // int height = rect.height;

    // int ratio = 10;
    // if(rect.x - width/ratio>0){ rect.x = rect.x - width/ratio; rect.width = rect.width + width/ratio;} else rect.x = 0;
    // if(rect.y - height/ratio>0){ rect.y = rect.y - height/ratio; rect.height = rect.height + height/ratio;} else rect.y = 0;
    // if(rect.x + rect.width + width/ratio < src.cols) rect.width = rect.width + width/ratio;  else rect.width = src.cols-rect.x;
    // if(rect.y + rect.height + height/ratio < src.rows) rect.height = rect.height + height/ratio; else rect.height = src.rows-rect.y;
    // // std::cout<<"rect.x:"<<rect.x<<",rect.y:"<<rect.y<<",rect.width:"<<rect.width<<",rect.height:"<<rect.height<<std::endl;

    // Mat mat = src(rect);

    // plate.setPlateMat(mat);

    #ifdef DEBUG
    // std::cout<<plate_mat.rows<<std::endl;
    imshow("Plate", plate.getPlateMat());
    waitKey();
    #endif

    resultVec.push_back(plate);
    // std::cout<<resultVec.size()<<std::endl;
  }
  return 0;
}

void CPlateDetect::expandPlate(Mat src, std::vector<CPlate> &all_result_Plates){
  std::vector<CPlate> resultVec;
  for (size_t i = 0; i < all_result_Plates.size(); i++) {

    // 把截取的车牌图像依次放到左上角
    CPlate plate = all_result_Plates[i];
    Mat src_clone = src.clone();
    // Rect rect = plate.getPlatePos().boundingRect();
    Rect_<float> safeBoundRect;
    bool isFormRect = calcSafeRect(plate.getPlatePos(), src, safeBoundRect);
    // std::cout<<"safeBoundRect:"<<safeBoundRect.x<<":"<<safeBoundRect.y<<std::endl;
    Rect rect = safeBoundRect;
    // src_clone(safeBound.size()<<std::endl;
    int width = rect.width;
    int height = rect.height;

    int ratio = 10;
    if(rect.x - width/ratio>0){ rect.x = rect.x - width/ratio; rect.width = rect.width + width/ratio;} else rect.x = 0;
    if(rect.y - height/ratio>0){ rect.y = rect.y - height/ratio; rect.height = rect.height + height/ratio;} else rect.y = 0;
    if(rect.x + rect.width + width/ratio < src.cols) rect.width = rect.width + width/ratio;  else rect.width = src.cols-rect.x;
    if(rect.y + rect.height + height/ratio < src.rows) rect.height = rect.height + height/ratio; else rect.height = src.rows-rect.y;
    // std::cout<<"rect.x:"<<rect.x<<",rect.y:"<<rect.y<<",rect.width:"<<rect.width<<",rect.height:"<<rect.height<<std::endl;

    Mat mat = src(rect);

    plate.setPlateMat(mat);
    resultVec.push_back(plate);
  }
  all_result_Plates = resultVec;
}

int CPlateDetect::showResult(const Mat &result) {
  namedWindow("EasyPR", CV_WINDOW_AUTOSIZE);

  const int RESULTWIDTH = 640;   // 640 930
  const int RESULTHEIGHT = 540;  // 540 710

  Mat img_window;
  img_window.create(RESULTHEIGHT, RESULTWIDTH, CV_8UC3);

  int nRows = result.rows;
  int nCols = result.cols;

  Mat result_resize;
  if (nCols <= img_window.cols && nRows <= img_window.rows) {
    result_resize = result;

  } else if (nCols > img_window.cols && nRows <= img_window.rows) {
    float scale = float(img_window.cols) / float(nCols);
    resize(result, result_resize, Size(), scale, scale, CV_INTER_AREA);

  } else if (nCols <= img_window.cols && nRows > img_window.rows) {
    float scale = float(img_window.rows) / float(nRows);
    resize(result, result_resize, Size(), scale, scale, CV_INTER_AREA);

  } else if (nCols > img_window.cols && nRows > img_window.rows) {
    Mat result_middle;
    float scale = float(img_window.cols) / float(nCols);
    resize(result, result_middle, Size(), scale, scale, CV_INTER_AREA);

    if (result_middle.rows > img_window.rows) {
      float scale = float(img_window.rows) / float(result_middle.rows);
      resize(result_middle, result_resize, Size(), scale, scale, CV_INTER_AREA);

    } else {
      result_resize = result_middle;
    }
  } else {
    result_resize = result;
  }

  Mat imageRoi = img_window(Rect((RESULTWIDTH - result_resize.cols) / 2,
                                 (RESULTHEIGHT - result_resize.rows) / 2,
                                 result_resize.cols, result_resize.rows));
  addWeighted(imageRoi, 0, result_resize, 1, 0, imageRoi);

  imshow("EasyPR", img_window);
  waitKey();

  destroyWindow("EasyPR");

  return 0;
}

bool CPlateDetect::calcSafeRect(const RotatedRect &roi_rect, const Mat &src,
                                Rect_<float> &safeBoundRect) {
  Rect_<float> boudRect = roi_rect.boundingRect();

  // boudRect的左上的x和y有可能小于0

  float tl_x = boudRect.x > 0 ? boudRect.x : 0;
  float tl_y = boudRect.y > 0 ? boudRect.y : 0;

  // boudRect的右下的x和y有可能大于src的范围

  float br_x = boudRect.x + boudRect.width < src.cols
               ? boudRect.x + boudRect.width - 1
               : src.cols - 1;
  float br_y = boudRect.y + boudRect.height < src.rows
               ? boudRect.y + boudRect.height - 1
               : src.rows - 1;

  float roi_width = br_x - tl_x;
  float roi_height = br_y - tl_y;

  if (roi_width <= 0 || roi_height <= 0) return false;

  // 新建一个mat，确保地址不越界，以防mat定位roi时抛异常

  safeBoundRect = Rect_<float>(tl_x, tl_y, roi_width, roi_height);

  return true;
}

int CPlateDetect::plateUnion(Rect rect1, Rect rect2, float ratio){
  float intersection = rrOverlap(rect1,rect2);
  if(intersection > ratio){
    if (rect1.width*rect1.height>rect1.width*rect2.height)
    {
      return 1;
    }else{
      return 2;
    }
  }else{
    return 0;
  }
}

float CPlateDetect::rrOverlap(const Rect& box1,const Rect& box2)
{
  if (box1.x > box2.x+box2.width) { return 0.0; }
  if (box1.y > box2.y+box2.height) { return 0.0; }
  if (box1.x+box1.width < box2.x) { return 0.0; }
  if (box1.y+box1.height < box2.y) { return 0.0; }
  float colInt =  min(box1.x+box1.width,box2.x+box2.width) - max(box1.x, box2.x);
  float rowInt =  min(box1.y+box1.height,box2.y+box2.height) - max(box1.y,box2.y);
  float intersection = colInt * rowInt;
  float area1 = box1.width*box1.height;
  float area2 = box2.width*box2.height;
  return intersection / (area1 + area2 - intersection);
}

}
