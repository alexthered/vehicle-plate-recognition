#include "platerecognizer.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

//ctor
PlateRecognizer::PlateRecognizer()
{

}


void PlateRecognizer::RecognizePlate(const cv::Mat& in_plate, std::string& content_plate)
{
    cvtColor(in_plate, gray_plate, CV_RGB2GRAY);
    //extract character's images
    ExtractCharacterImages(gray_plate);

#if VERBOSE_MODE
    cv::imshow("Gray image of plate", gray_plate);
    cv::imshow("Gray image of plate after thresholding", threshold_gray_plate);
#endif
}

//separate each character's image from the whole plate image
void PlateRecognizer::ExtractCharacterImages(const cv::Mat& in_plate_gray)
{
    //a simple thresholding
    cv::threshold(in_plate_gray, threshold_gray_plate, 60, 255, CV_THRESH_BINARY_INV);

}
