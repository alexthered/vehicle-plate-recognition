#ifndef PLATEDETECTOR_H
#define PLATEDETECTOR_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

/**
 * Extract region containg plate from an input image
 *
 **/

class PlateDetector
{
public:
    PlateDetector();
    ~PlateDetector();

    //Detect the region containing plate from input image
    void DetectPlate(const cv::Mat& in_img);

private:

    //pre-process the input image
    void PreprocessImg(const cv::Mat& in_img);
    void DetectRegion(const cv::Mat& gray_img);
    int VerifyRegion(const cv::Rect rect);

    //buffer to store intermediate results
    cv::Mat in_img;
    cv::Mat gray_img;
    cv::Mat preprocessed_img;
    cv::Mat sobel_img;
    cv::Mat threshold_img;
};

#endif // IMGPREPROCESSOR_H
