#ifndef PLATERECOGNIZER_H
#define PLATERECOGNIZER_H
#pragma once
#include <opencv2/core/core.hpp>
#include <iostream>
#include <vector>
#include <QString>

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
    void ExtractCharacterImages();
    int VerifyCharacterRegion(const cv::Mat character_img);

    cv::Mat gray_plate, threshold_gray_plate, in_plate;
    std::vector<cv::Mat> character_imgs;
};

#endif // PLATERECOGNIZER_H
