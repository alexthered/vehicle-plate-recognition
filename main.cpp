#include "platedetector.h"
#include <iostream>

using namespace std;
using namespace cv;


int main()
{
    /* read input image */
    Mat in_img = imread("../../data/P6070015.jpg");

    namedWindow("Input image", CV_WINDOW_NORMAL || CV_WINDOW_KEEPRATIO || CV_GUI_EXPANDED);
    cv::imshow("Input image", in_img);
    waitKey(0);
}
