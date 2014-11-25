#include "platedetector.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;


int main()
{
    /* read input image */
    Mat in_img = imread("../../data/P6070015.jpg");

    PlateDetector p_detector;
    vector<Mat> plates;
    p_detector.DetectPlate(in_img, plates);

    cv::imshow("Input image", in_img);
    waitKey(0);
}
