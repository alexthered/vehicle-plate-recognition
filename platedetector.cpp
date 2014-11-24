#include "platedetector.h"
#include "opencv2/imgproc/imgproc.hpp"


using namespace std;
using namespace cv;
//ctor
PlateDetector::PlateDetector()
{
}

PlateDetector::~PlateDetector()
{

}

void PlateDetector::DetectPlate(const cv::Mat &in_img)
{
    PreprocessImg(in_img);
    DetectRegion(gray_img);

#if VERBOSE_MODE //show all intermediate results
    cv::imshow("grayscale image after smoothing", gray_img);
    cv::imshow("grayscale image after histogram equalization", gray_img);
    cv::imshow("grayscale image after sobel", sobel_img);
    cv::imshow("grayscale image after Otsu thresholding", sobel_img);
#endif
}

//Pre-process the image: grayscale conversion + Gaussian smoothing + histogram equalization
void PlateDetector::PreprocessImg(const cv::Mat& in_img)
{
    cvtColor(in_img, gray_img, CV_RGB2GRAY);

    //gaussian smoothing
    GaussianBlur(gray_img, gray_img, cv::Size(5,5), 0.5, 0.5);

    //histogram equalization (not sure it's needed)
    equalizeHist(gray_img, gray_img);
}

//detect regions which are possible to contain plate
void PlateDetector::DetectRegion(const cv::Mat& gray_img)
{
    //apply sobel filter to reveal region with big number of vertical edges
    Sobel(gray_img, sobel_img, CV_8U, 1, 0, 3, 1, 0);

    //threshold the image using Otsu's algorithm
    threshold(sobel_img, threshold_img, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
}
