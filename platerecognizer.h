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
    void SaveCharacterImg(const cv::Mat& _in_plate, const std::string& output_dir);

private:
    void ExtractCharacterImages();
    int VerifyCharacterRegion(const cv::Mat character_img);
    void Preprocess();


    cv::Mat gray_plate, threshold_gray_plate, in_plate;
    std::vector<cv::Mat> character_imgs;

    int counter;
};

#endif // PLATERECOGNIZER_H
