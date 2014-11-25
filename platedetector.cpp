#include "platedetector.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>


using namespace std;
using namespace cv;
//ctor
PlateDetector::PlateDetector()
{
}

PlateDetector::~PlateDetector()
{

}

void PlateDetector::DetectPlate(const cv::Mat &_in_img)
{
    in_img = _in_img.clone(); //clone the input image
    PreprocessImg(in_img);
    DetectRegion(gray_img);

#if VERBOSE_MODE //show all intermediate results
    cv::imshow("grayscale image after smoothing", gray_img);
    cv::imshow("grayscale image after histogram equalization", gray_img);
    cv::imshow("grayscale image after sobel", sobel_img);
    cv::imshow("grayscale image after Otsu thresholding and closing operator", threshold_img);
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

    //apply closing operator
    Mat ele = getStructuringElement(MORPH_RECT, Size(10,3)); //horizontal closing
    morphologyEx(threshold_img, threshold_img, CV_MOP_CLOSE, ele);

    //get list of connected component
    vector< vector<Point> > contours;
    vector<RotatedRect> boundingRect;
    findContours(threshold_img.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    //iterate over the list of contour and find the bounding rectangle
    vector< vector<Point> >::iterator iter = contours.begin();
    while(iter!=contours.end()){
        //get the minimum area bounding rectangle
        RotatedRect rect = minAreaRect(Mat(*iter));
        Rect cur_rect = rect.boundingRect();
        if (!VerifyRegion(cur_rect)){
            iter = contours.erase(iter); //remove this region
        } else {
            boundingRect.push_back(rect);
            iter++;
        }
    }

#if VERBOSE_MODE
    //superposition the detected bounding rect into the input image
    for (int i=0; i<boundingRect.size(); i++){
        Rect cur_rect= boundingRect[i].boundingRect();
        cv::rectangle(in_img, cur_rect, Scalar(0,255,0),1,8,0);
    }

    imshow("Input image with detected plate", in_img);
#endif
}

//verify if a region is possible to contain a plate
int PlateDetector::VerifyRegion(const cv::Rect rect)
{
    //get the width-height ratio
    float ratio = float(rect.width)/float(rect.height);
    if (ratio < 1)
        ratio = 1.0/ratio;

    if (ratio < 5 || ratio > 7)
        return 0;


    //minimum area condition (highly tuned for specific case)
    int area = rect.width * rect.height;
    std::cout << area << std::endl;
    if ( area < 3500 || area > 10000)
        return 0;

    return 1;
}
