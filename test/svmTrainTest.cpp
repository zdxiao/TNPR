#include "easypr/train/svm_train.h"
#include <iostream>

using namespace std;
using namespace easypr;

int main3(int argc, char **argv)
{
    SvmTrain trainModel("/home/xiao/Documents/svm", "/home/xiao/Documents/svm.xml");
    trainModel.train();
    trainModel.test();
    return 0;
}
