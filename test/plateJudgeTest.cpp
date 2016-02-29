#include "easypr/core/plate_recognize.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace cv;
using namespace std;
using namespace easypr;

int main1(int argc, char**argv)
{
	cout << "hello world!" << endl;
	ifstream listFile("/home/xiao/Project/PlateReco/EasyPR/resources/image/list.txt");
	int svm_counter = 0;
	while(1)
	{
		string line;
		getline(listFile, line);
		cout << line << endl;
		line = "/home/xiao/Project/PlateReco/EasyPR/resources/image/plates0218/" + line;
		//line = "./resources/image/general_test/" + line;
		Mat img = imread(line);
		namedWindow("test", 0);
		imshow("test", img);
		int result = -1;
        PlateJudge::instance()->plateJudge(img, result);
        if(result == 1)
        {
            svm_counter++;
        }
        cout << svm_counter << endl;
		//waitKey();
	}
	listFile.close();
	return 0;
}


