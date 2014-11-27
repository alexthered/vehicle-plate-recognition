#ifndef PLATEDETECTOR_H
#define PLATEDETECTOR_H

#pragma once
#include "opencv2/core/core.hpp"
#include <vector>
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
    void DetectPlate(const cv::Mat& in_img, std::vector<cv::Mat>& plate_img);

private:

    //pre-process the input image
    void PreprocessImg(const cv::Mat& in_img);
    void DetectRegion(const cv::Mat& gray_img);
    int VerifyRegion(const cv::Rect rect);
    void EnlargeRect(cv::Rect& rect);

    //buffer to store intermediate results
    cv::Mat in_img;
    cv::Mat gray_img;
    cv::Mat preprocessed_img;
    cv::Mat x_sobel_img, y_sobel_img; //horizontal and vertical gradient image
    cv::Mat threshold_img;
    std::vector<cv::Mat> plates;

    //other constants
    float enlarge_factor;
};

#endif // IMGPREPROCESSOR_H
