/*******************************************************************/
/*
			Author:		    Zedong Xiao
			Creat Date:	    2015/11/15
			Email:		    zd.xiao@qq.com
*/
/*******************************************************************/

#ifndef	XZD_IMAGE_PROCESS_H
#define	XZD_IMAGE_PROCESS_H

#include <vector>
#include <iostream>
#include <ctime>
#include <cmath>
#include <opencv2/opencv.hpp>


//#define	DEBUG_LINE
//#define	DEBUG_ANGLE
//#define	DEBUG_COUT
//#define	DEBUG_POSITION
//#define	DEBUG_CORNER
//#define		DEBUG_SEGMENT
//#define		DEBUG_HOMO
#define		AUTO_PARSE_TANGRAM

using namespace cv;

// segement one hue from Binary hue image
void hueImgBinarySegment(Mat src, Mat &dst, const int centerLevel, const int rangeSize = 20);
// convert gbr to hue
void bgr2hue(Mat gbrImg, Mat &hue, double whiteThres = 0.25);
// detect line using hogh transform
void lineDetect(Mat binaryImg);
// detect corner by harris method
void cornerHarris_demo(Mat src_gray, int thresh);
// detect line using RANSAC
void lineDetectByRANSAC(std::vector<Point2i> &edgePoints, std::vector<Vec3d> &lines, double minLineLength,
						double maxInlinerDis, Mat &garyImage, std::vector<Point2d> &linesMidPoint);
// merge parallel lines
void mergeParallelLines(std::vector<Vec3d> &lines, double angleThres, std::vector<Point2d> linesPoint);
// judge if the lines are paralle lines
bool isParallelClose(Vec3d line1, Vec3d line2, double angleThres, Point2d point1, Point2d point2, double max_distance = 20);
// judge if the lines are paralle lines
bool isParallel(Vec3d line1, Vec3d line2, double angleThres = 15);
// calculate cross point of two lines
bool calCrossPoint(Vec3d line1, Vec3d line2, Point2d &crossPoint);
// show given points in a gray image
void showPoints(std::vector<Point2d> &Points, Mat grayImage);
// calculate angle of two std::vectors
bool calAngle(Vec2d vec1, Vec2d vec2, double &angle);
// act homography
void warpHomo(Mat src, Mat &dst, Mat& H);

/**
 *@brief white black segment
 *@param src -- input gbr image
 *@param dst -- output image of a binary image with 0 and 255,
    when segement black, 255 represent black pixel, 0 others;
    when segement white, 255 represent white pixel, 0 others;
 *@param flag -- 0 segment black, 1 segment white
 *@param thres -- saturtion threshold, (255 * thres)
 */
void whiteBlackSegment(Mat src, Mat &dst, int flag = 1, double thres= 0.3);

#endif
