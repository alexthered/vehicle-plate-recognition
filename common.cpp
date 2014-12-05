#include "common.h"

cv::Rect ClipRect(cv::Rect rect, cv::Size img_size)
{
    cv::Rect clipped_rect;
    clipped_rect.x = (rect.x < 0) ? 0 : rect.x;
    clipped_rect.y = (rect.y < 0) ? 0 : rect.y;

    clipped_rect.width = ((clipped_rect.x + rect.width) <= img_size.width) ?
                          rect.width : (img_size.width - clipped_rect.x);
    clipped_rect.height = ((clipped_rect.y + rect.height) <= img_size.height) ?
                          rect.height : (img_size.height - clipped_rect.y);

    return clipped_rect;
}

//check if two rectangles are similar or not
bool IsSimilarRect(const cv::Rect rect1, const cv::Rect rect2)
{
    cv::Rect intersect_rect = rect1 & rect2;

    double union_area = double(rect1.area() + rect2.area() - intersect_rect.area());

    if ((double(intersect_rect.area())/union_area) > 0.75)
        return true;
    else
        return false;
}


//convert the rectangle's coordinate from strip to origin image
cv::Rect StripRectToImageRect(const cv::Rect in_rect, const cv::Rect strip_rect)
{
    cv::Rect out_rect;
    out_rect.x = in_rect.x + strip_rect.x;
    out_rect.y = in_rect.x + strip_rect.y;
    out_rect.width = in_rect.width;
    out_rect.height = in_rect.height;

    return out_rect;
}

//convert the rectangle's coordinate from strip to origin image
cv::RotatedRect StripRotatedRectToImageRotatedRect(const cv::RotatedRect in_rect, const cv::Rect strip_rect)
{
    cv::RotatedRect out_rect;
    out_rect.center.x = in_rect.center.x + strip_rect.x;
    out_rect.center.y = in_rect.center.y + strip_rect.y;
    out_rect.size.width = in_rect.size.width;
    out_rect.size.height = in_rect.size.height;

    return out_rect;
}


void AvgFilter(QVector<double>& in_vec, int filter_size,
                               double& minVal, double& maxVal)
{
    int half_filter_size = int(filter_size/2);
    for(int i=half_filter_size; i<(in_vec.size()-half_filter_size); ++i){
        //get the sum
        double sum = 0;
        for(int j=0; j<filter_size; j++)
           sum += in_vec[i-half_filter_size+j];

        //get the middle element
        in_vec[i] = sum/double(filter_size);
        //update min and max value
        if (minVal > in_vec[i])
            minVal = in_vec[i];
        if (maxVal < in_vec[i])
            maxVal = in_vec[i];
    }
}

//perform 1D median vector
void MedianFilter(QVector<double>& in_vec, int filter_size,
                                 double& minVal, double& maxVal)
{
    double* window = new double[filter_size];

    int half_filter_size = int(filter_size/2);
    for(int i=half_filter_size; i<(in_vec.size()-half_filter_size); ++i){
        //copy related item from
        for(int j=0; j<filter_size; j++)
            window[j] = in_vec[i-half_filter_size+j];
        //order half of the elements
        for(int j=0; j<3; j++){
            int min = j;
            //get the position of minimum element
            for (int k=j+1; k<filter_size; k++){
                if (window[k] < window[min])
                    min = k;
                //put the minimum element to the left most one
                const double tmp = window[j];
                window[j] = window[min];
                window[min] = tmp;
            }
        }
        //get the middle element
        in_vec[i] = window[half_filter_size];
        //update min and max value
        if (minVal > in_vec[i])
            minVal = in_vec[i];
        if (maxVal < in_vec[i])
            maxVal = in_vec[i];
    }
    delete window;
}

