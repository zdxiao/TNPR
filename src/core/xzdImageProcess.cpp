/*******************************************************************/
/*
			Author:		    Zedong Xiao
			Creat Date:	    2015/11/15
			Email:		    zd.xiao@qq.com
			Last Modify:		2016/1/10
*/
/*********************************************************************/

#include "easypr/core/xzdImageProcess.h"

#define  MAX_HUE	180

void hueImgBinarySegment(Mat src, Mat &dst, const int centerLevel, const int rangeSize)
{
	dst.create(src.rows, src.cols, src.type());

	for(int row = 0; row != src.rows; ++row)
	{
		uchar *p_src = src.ptr<uchar>(row);
		uchar *p_dst = dst.ptr<uchar>(row);
		for(int col = 0; col != src.cols; ++col)
		{
			if(p_src[col] > MAX_HUE)
			{
				p_dst[col] = 0;
				continue;
			}
			if((p_src[col] >= centerLevel - rangeSize && p_src[col] <= centerLevel + rangeSize)
				|| (p_src[col] > MAX_HUE + centerLevel - rangeSize) // case "centerLevel < rangeSize"
				|| ((p_src[col] < -MAX_HUE + centerLevel + rangeSize))) // case "centerLevel > MAX_HUE - rangeSize"
			{
				p_dst[col] = 255;
			}
			else
			{
				p_dst[col] = 0;
			}
		}
	}
	Mat SE = getStructuringElement(MORPH_RECT, Size(3, 3));
	erode(dst, dst, SE);
	dilate(dst, dst, SE);
}

void bgr2hue(Mat gbrImg, Mat &hue, double whiteThres)
{
	Mat hsv;
	cvtColor(gbrImg, hsv, CV_BGR2HSV);
	hue.create(hsv.rows, hsv.cols, CV_8UC1);
    for(int row = 0; row != hsv.rows; ++row)
    {
        uchar *p_hsv = hsv.ptr<uchar>(row);
        uchar *p_H   = hue.ptr<uchar>(row);

        for(int col = 0; col != hsv.cols; ++col)
        {
            if(p_hsv[3 * col + 1] > whiteThres * 255 && p_hsv[3 * col + 2] > 30)
            {
                p_H[col] = p_hsv[3 * col];
            }
            else
            {
                p_H[col] = 255;
            }
        }
    }
}


void lineDetect(Mat binaryImg)
{
	Mat color_dst;
    cvtColor( binaryImg, color_dst, CV_GRAY2BGR );
	std::vector<Vec4i> lines;
	HoughLinesP(binaryImg, lines, 20, CV_PI/10, 30, 10, 20);
	std::cout << lines.size() << std:: endl;
	for( size_t i = 0; i < lines.size(); i++ )
	{
		line(color_dst, Point(lines[i][0], lines[i][1]),
			Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8);
		namedWindow( "Detected Lines", 1);
		imshow( "Detected Lines", color_dst);
		std::cout << "line " << i << std::endl;
		waitKey();
	}
	return;
}

/** @function cornerHarris_demo */
void cornerHarris_demo(Mat src_gray, int thresh)
{
  char corners_window[20] = "corners";
  Mat dst, dst_norm, dst_norm_scaled;
  dst = Mat::zeros( src_gray.size(), CV_32FC1);

  /// Detector parameters
  int blockSize = 2;
  int apertureSize = 3;
  double k = 0.04;

  /// Detecting corners
  cornerHarris( src_gray, dst, blockSize, apertureSize, k, BORDER_DEFAULT );

  /// Normalizing
  normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
  convertScaleAbs( dst_norm, dst_norm_scaled );

  /// Drawing a circle around corners
  for( int j = 0; j < dst_norm.rows ; j++ )
     { for( int i = 0; i < dst_norm.cols; i++ )
          {
            if( (int) dst_norm.at<float>(j,i) > thresh )
              {
               circle( dst_norm_scaled, Point( i, j ), 5,  Scalar(0), 2, 8, 0 );
              }
          }
     }
  /// Showing the result
  namedWindow( corners_window, 1);
  imshow( corners_window, dst_norm_scaled );
}

int randTwoPoints(std::vector<Point2i> points, int &index_point1, int &index_point2, double minPointsDistance = 10)
{
	int pointsNum = points.size();
	index_point1 = rand() % pointsNum;
	index_point2 = rand() % pointsNum;
	double dx = points[index_point1].x - points[index_point2].x;
	double dy = points[index_point1].y - points[index_point2].y;
	double distance = sqrt(dx * dx + dy * dy);
	int iter = 0, maxIter = 100;
	while(distance < minPointsDistance && iter < maxIter)
	{
		index_point2 = rand() % pointsNum;
		dx = points[index_point1].x - points[index_point2].x;
		dy = points[index_point1].y - points[index_point2].y;
		distance = sqrt(dx * dx + dy * dy);
		++iter;
	}
	if(distance < minPointsDistance)
	{
		return false;
	}
	return true;
}
template <typename T>
void getLineNormFunc(T &point1, T &point2, Vec3d &_line)
{
	// line ax + by + c = 0 && a*a + b*b = 1;
	_line[0] = point2.y - point1.y; // a = y2 - y1
	_line[1] = point1.x - point2.x; // b = x1 - x2
	_line[2] = -_line[1] * point1.y - _line[0] * point1.x; // c = (x2 - x1)y1 - (y2 - y1)x1
	double lineM = sqrt(_line[0] * _line[0] + _line[1] * _line[1]);
	_line /= lineM;
}

void lineInlier(std::vector<Point2i> &points, Vec3d &_line, std::vector<Point2i> &inliers,
				std::vector<Point2i> &outliers, double maxInlinerDis = 5)
{
	if(!inliers.empty())
	{
		inliers.clear();
	}
	if(!outliers.empty())
	{
		outliers.clear();
	}
	if(points.empty() || (_line[0] == 0 && _line[1] == 0))
	{
		return;
	}
	double norm_ab = sqrt(_line[0] * _line[0] + _line[1] * _line[1]);
	// find inliers and calculate inlier number
	for(int index = 0; index != points.size(); ++index)
	{
		Point2i point = points[index];
		double distance = fabs(_line[0] * point.x + _line[1] * point.y + _line[2]) / norm_ab;
		if(distance < maxInlinerDis)
		{
			inliers.push_back(point);
		}
		else
		{
			outliers.push_back(point);
		}
	}
}

void lineDetectByRANSAC(std::vector<Point2i> &edgePoints, std::vector<Vec3d> &lines, double minLineLength,
						double maxInlinerDis, Mat &garyImage, std::vector<Point2d> &linesMidPoint)
{
	if(edgePoints.empty())
	{
		return;
	}
	int max_iteration = 100; // after calculation, 15 times iteration is enough for keep lost line probability less than 0.01%
	int lineNum = 5;
	double minRandomPointsDis = 2 * minLineLength / 3.0; // choose as a quarter of the shortest line
	int iter = 0, detectedLinesNum = 0;

	srand(time(NULL));
	std::vector<Point2i> points(edgePoints);

	Mat color_dst;
	cvtColor(garyImage, color_dst, CV_GRAY2BGR );

	while(iter < max_iteration && detectedLinesNum < lineNum && points.size() > minLineLength)
	{
		++iter;
#ifdef DEBUG_COUT
		std::cout << "iter = " << iter << std::endl;
#endif
		int index_point1 = 0, index_point2 = 0;
		if(!randTwoPoints(points, index_point1, index_point2, minRandomPointsDis))
		{
			return;
		}
		Vec3d _line;
		// line ax + by + c = 0
		getLineNormFunc(points[index_point1], points[index_point2], _line);
		Point2d mid_point = (points[index_point1] + points[index_point2]) * 0.5;
		std::vector<Point2i> outliers, inliers;
		lineInlier(points, _line, inliers, outliers, maxInlinerDis / 2);
		// if find a line
		if(inliers.size() > minLineLength)
		{
			Vec4f tmpLine;
			fitLine(inliers, tmpLine, CV_DIST_L2, 0, 0.01, 0.01);
			Vec3d newLine;
			Point2d tmp1 = Point2d(tmpLine[2], tmpLine[3]);
			Point2d tmp2 = Point2d(tmpLine[2] + tmpLine[0], tmpLine[3] + tmpLine[1]);
			getLineNormFunc(tmp1, tmp2, _line);
			lineInlier(points, _line, inliers, outliers, maxInlinerDis);
			// fit a new line
			if(inliers.size() > minLineLength)
			{
				++detectedLinesNum;
#ifdef DEBUG_COUT
				std::cout << "line " << detectedLinesNum << std::endl;
#endif
				lines.push_back(_line);
				Point2i pointsSum = Point2i(0, 0);
				for(int midId = 0; midId != inliers.size(); ++midId)
				{
					pointsSum += inliers[midId];
				}
				mid_point.x = double(pointsSum.x) / inliers.size();
				mid_point.y = double(pointsSum.y) / inliers.size();
				linesMidPoint.push_back(mid_point);
#ifdef DEBUG_LINE
				line(color_dst, Point(points[index_point1].x, points[index_point1].y), Point(points[index_point2].x, points[index_point2].y), Scalar(0,0,255), 3, 8);
				namedWindow( "Detected Lines", 1);
				imshow( "Detected Lines", color_dst);
				waitKey();
#endif
				points = outliers;
			}
		}
	}
}

void mergeParallelLines(std::vector<Vec3d> &lines, double angleThres, std::vector<Point2d> linesPoint)
{
	if(lines.size() < 2)
	{
		return;
	}
	int* fullSet = new int[lines.size()];
	for(int i = 0; i != lines.size(); ++i)
	{
		fullSet[i] = i;
	}
	std::vector<std::vector<int> > paralleLines;
	int *tmp = new int[lines.size() + 1];
	while(fullSet[0] != -1)
	{
		for(int j = 0; j != lines.size() + 1; ++j) tmp[j] = -1;
		tmp[0] = fullSet[0];
		for(int j = 0; j != lines.size() - 1; ++j) fullSet[j] = fullSet[j + 1];
		fullSet[lines.size() - 1] = -1;
		int tmp_k = 1;
		for(int index = 0; tmp[index] != -1; ++index)
		{
			for(int i = 0; fullSet[i] != -1; ++i)
			{
				if(isParallelClose(lines[tmp[index]], lines[fullSet[i]], angleThres,
					linesPoint[tmp[index]], linesPoint[fullSet[i]]))
				{
					// if the parallel lines direction are not in the same, add and get mean of "a b c " is wrong
					double direct_sign = lines[tmp[index]][0] * lines[fullSet[i]][0] +
						lines[tmp[index]][1] * lines[fullSet[i]][1];
					if(direct_sign < 0) // change the line direction when it is direct the oppsite direction
					{
						lines[fullSet[i]] *= -1;
					}
					tmp[tmp_k] = fullSet[i];
					++tmp_k;
					for(int j = i; j != lines.size() - 1; ++j) fullSet[j] = fullSet[j + 1];
					fullSet[lines.size() - 1] = -1;
					--i;
				}
			}
		}
		std::vector<int> sameParalle;
		for(int index = 0; tmp[index] != -1; ++index)
		{
			sameParalle.push_back(tmp[index]);
		}
		paralleLines.push_back(sameParalle);
	}
	delete [] tmp;
	delete [] fullSet;
	std::vector<Vec3d> newLines;
	for(std::vector<std::vector<int> >::iterator iter = paralleLines.begin(); iter != paralleLines.end(); ++iter)
	{
		Vec3d tmpLine(0, 0, 0);
		for(int index = 0; index != (*iter).size(); ++index)
		{
			tmpLine += lines[(*iter)[index]];
		}
		tmpLine = tmpLine / int((*iter).size());
		newLines.push_back(tmpLine);
	}
	lines = newLines;
}

bool isParallelClose(Vec3d line1, Vec3d line2, double angleThres, Point2d point1, Point2d point2, double max_distance)
{
	double lineM = (sqrt(line1[0] * line1[0] + line1[1] * line1[1]) *
		sqrt(line2[0] * line2[0] + line2[1] * line2[1]));
	if(lineM < 1e-6)
	{
		return false;
	}
	double dis = fabs(line1[0] * point2.x + line1[1] * point2.y + line1[2]) / sqrt(line1[0] * line1[0] + line1[1] * line1[1]);
	double angleCos = (line1[0] * line2[0] + line1[1] * line2[1]) / lineM;
	if(fabs(angleCos) > cos(angleThres / 180 * CV_PI) && dis < max_distance)
	{
		return true;
	}
	return false;
}

bool isParallel(Vec3d line1, Vec3d line2, double angleThres)
{
	double lineM = (sqrt(line1[0] * line1[0] + line1[1] * line1[1]) *
		sqrt(line2[0] * line2[0] + line2[1] * line2[1]));
	if(lineM < 1e-6)
	{
		return false;
	}

	double angleCos = (line1[0] * line2[0] + line1[1] * line2[1]) / lineM;
	if(fabs(angleCos) > cos(angleThres / 180 * CV_PI))
	{
		return true;
	}
	return false;
}

bool calCrossPoint(Vec3d line1, Vec3d line2, Point2d &crossPoint)
{
	double tmp1 = line1[0] * line2[1] - line2[0] * line1[1]; // a1b2 - a2b1
	if(fabs(tmp1) < 1e-6)
	{
		return false;
	}
	double tmp2 = line1[1] * line2[2] - line2[1] * line1[2]; // b1c2 - b2c1
	double tmp3 = line2[0] * line1[2] - line1[0] * line2[2]; // a2c1 - a1c2

	crossPoint.x = tmp2 / tmp1;
 	crossPoint.y = tmp3 / tmp1;
	return true;
}

void showPoints(std::vector<Point2d> &points, Mat grayImage)
{
	Mat color_dst;
	cvtColor(grayImage, color_dst, CV_GRAY2BGR );
	for(int index = 0; index != points.size(); ++index)
	{
		circle(color_dst, points[index], 5,  Scalar(0, 0, 255), 2, 8, 0);
	}
	namedWindow("corners", 1);
	imshow("corners", color_dst);
	waitKey();
	return;
}

bool calAngle(Vec2d vec1, Vec2d vec2, double &angle)
{
	if(norm(vec1) < 1e-6 || norm(vec2) < 1e-6)
	{
		return false;
	}
	double tmp = (vec1[0] * vec2[0] + vec1[1] * vec2[1]) / (norm(vec1) * norm(vec2));
	angle = acos(tmp) * 180.0 / CV_PI;
	return true;
}

void warpHomo(Mat src, Mat &dst, Mat& H)
{
	//double Homog[3][3] =
	//{	2.43274390470416,			0.708142994169358	,	-322.615388744916,
	//	0.00743826832301289,		2.68142959180608,		153.648366066942,
	//	-1.04786119481409e-05,	0.00150446099136316,	1.08839620099834};
	//double HomogNew[3][3] =
	//{	0.733481496700970	,		0.200257483504231,	-6.26468199560132,
	//	0.000729338983379437,		-0.564385709578822,	361.854325316173,
	//	5.26858500440132e-06,		0.000554960144832507,	1};

	warpPerspective(src, dst, H, Size(1100, 800));
#ifdef DEBUG_HOMO
	imshow("wraped", dst);
#endif
}

void whiteBlackSegment(Mat src, Mat &dst, int flag, double thres)
{
	Mat hsv, gray, histEq_gray;
	int whiteValueThres = 100, blackValueThres = 50;
	cvtColor(src, hsv, CV_BGR2HSV);
	cvtColor(src, gray, CV_BGR2GRAY);
	equalizeHist(gray, histEq_gray);
//#define DEBUG_WHITE_SEGMENT
#ifdef DEBUG_WHITE_SEGMENT
    imshow("gray", gray);
    imshow("histEq_gray", histEq_gray);
#endif
	dst.create(src.rows, src.cols, CV_8UC1);
	for(int row = 0; row != hsv.rows; ++row)
	{
        uchar* p = hsv.ptr<uchar>(row);
        uchar* p_dst = dst.ptr<uchar>(row);
        uchar* p_gray = histEq_gray.ptr<uchar>(row);
        for(int col = 0; col != hsv.cols; ++col)
        {
            if(p[3 * col + 1] < thres * 255)
            {
                if(flag && p_gray[col] > whiteValueThres)
                {
                    p_dst[col] = 255;
                }
                else if(!flag && p_gray[col] < blackValueThres)
                {
                    p_dst[col] = 255;
                }
                else
                {
                    p_dst[col] = 0;
                }
            }
            else
            {
                p_dst[col] = 0;
            }
        }
	}
}


