#ifndef PLATEDETECTOR_H
#define PLATEDETECTOR_H

#pragma once
#include "opencv2/core/core.hpp"
#include <vector>
#include <iostream>
#include <QVector>
#include <QPair>
#include "PlotDrawing/plotwindow.h"
#include "common.h"

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
    int VerifySegment(const QPair<int, int> in_pair);
    void EnlargeRect(cv::Rect& rect);
    //calculate the sum of each row or column
    void CalDimSum(const cv::Mat gra_img, QVector<double>& dim_sum, int dim);
    //normalize the projection of gradient image
    void NormalizeVectorAndFindSegment(QVector<double>& in_vec, QVector<QPair<int, int> >& segment);
    //visualize the projection and found segment
    void Visualize();

    //buffer to store intermediate results
    cv::Mat in_img;
    cv::Mat gray_img;
    cv::Mat preprocessed_img;
    cv::Mat x_sobel_img, y_sobel_img; //horizontal and vertical gradient image
    cv::Mat threshold_strip, sobel_strip;

    std::vector<cv::Mat> cand_plates; //candidate plates (after first step)
    std::vector<cv::Mat> plates;      //detected plates (after second step)

    //other constants
    float enlarge_factor;
    //input image'size
    cv::Size img_size;

    PlotWindow *w;
    int n_plot, cur_plot;

    double threshold;

    QVector<double> col_sum, row_sum;
    QVector<QPair<int, int> > col_segment, row_segment;
};

#endif // IMGPREPROCESSOR_H
