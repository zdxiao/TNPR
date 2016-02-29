#include "easypr/core/plate_recognize.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace cv;
using namespace std;
using namespace easypr;

int main(int argc, char**argv)
{
	cout << "hello world!" << endl;
	ifstream listFile("notRecoLists.txt");
    string line;
	while(getline(listFile, line))
	{
		cout << line << endl;
		//line = "../2015_12_09/" + line;
		line = "./resources/image/native_test/" + line;
		Mat img = imread(line);
		namedWindow("test", 0);
		imshow("test", img);
//		CPlateDetect cpd;
        CPlateRecognize cpr;
//		cpd.setPDDebug(true);
        cpr.setLifemode(false);
        cpr.setDebug(true);
		vector<CPlate> resultVec;
		vector<string> licenseVec;
//		cpd.plateDetect(img, resultVec);
        cpr.plateRecognize(img, licenseVec);
		//waitKey();
	}
	listFile.close();
	return 0;
}

