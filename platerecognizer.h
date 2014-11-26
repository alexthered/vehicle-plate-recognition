#ifndef PLATERECOGNIZER_H
#define PLATERECOGNIZER_H
#pragma once
#include <opencv2/core/core.hpp>
#include <iostream>
#include <vector>

/**
 * @brief Recognize characters from a plate image (letters and numbers)
 *
 */

class PlateRecognizer
{
public:
    PlateRecognizer();

    void RecognizePlate(const cv::Mat& in_plate, std::string& content_plate);

private:
    void ExtractCharacterImages(const cv::Mat& in_plate_gray);
    int VerifyCharacterRegion(const cv::Mat character_img);

    cv::Mat gray_plate, threshold_gray_plate;
    std::vector<cv::Mat> character_imgs;
};

#endif // PLATERECOGNIZER_H
