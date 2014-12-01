#ifndef COMMON_H
#define COMMON_H

/**
 * Implement some common functions to use across all classes
 *
 */

#pragma once

#include "opencv2/core/core.hpp"
#include <QVector>


/**
 * clip a rectangle so that it can fit into an image
 *
 */

cv::Rect ClipRect(cv::Rect rect, cv::Size img_size);

void AvgFilter(QVector<double>& in_vec, int filter_size,
               double& minVal, double& maxVal);

void MedianFilter(QVector<double>& in_vec, int filter_size,
                                 double& minVal, double& maxVal);

#endif // COMMON_H
