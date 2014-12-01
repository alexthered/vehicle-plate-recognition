#include "platedetector.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <QApplication>
#include <QPair>
#include <QString>

using namespace std;
using namespace cv;

//ctor
PlateDetector::PlateDetector()
{
    enlarge_factor = 1.05;
    n_plot = 2;
    cur_plot = 0;
    w = new PlotWindow;
    threshold = 0.5;
}

PlateDetector::~PlateDetector()
{
    //delete all plotting windows
    delete w;
}

void PlateDetector::DetectPlate(const cv::Mat &_in_img, std::vector<cv::Mat>& plate_img)
{    
    in_img = _in_img.clone(); //clone the input image
    img_size =  Size(in_img.cols, in_img.rows); //get input image'size

    PreprocessImg(in_img);
    DetectRegion(gray_img);

    //copy plate image to output file
    plate_img = cand_plates;

#if VERBOSE_MODE //show all intermediate results
    //cv::imshow("grayscale image after Gaussian blur", gray_img);
    cv::imshow("horizontal gradient image", x_sobel_img);
    //cv::imshow("vertical gradient image", y_sobel_img);
#endif    

}

//Pre-process the image: grayscale conversion + Gaussian smoothing + histogram equalization
void PlateDetector::PreprocessImg(const cv::Mat& in_img)
{
    cvtColor(in_img, gray_img, CV_RGB2GRAY);

    //gaussian smoothing
    GaussianBlur(gray_img, gray_img, cv::Size(7,7), 1.5, 1.5);;
}

//detect regions which are possible to contain plate
void PlateDetector::DetectRegion(const cv::Mat& gray_img)
{
    //apply sobel filter to reveal horizontal and vertical gradient image
    Sobel(gray_img, x_sobel_img, CV_8U, 1, 0, 3, 1, 0);

    /**
     *Now project the edge image into x and y axis
     */
    CalDimSum(x_sobel_img, row_sum, 0);

    //normalize the result
    NormalizeVectorAndFindSegment(row_sum, row_segment);


    //get all possible strip regions which can contain plates
    for (int i=0; i<row_segment.size(); ++i){
        QPair<int, int> row_range = row_segment[i];
        //QPair<int, int> col_range = col_segment[j];

        cv::Mat cur_plate = gray_img(Rect(0, row_range.first,
                                        in_img.cols - 1,
                                        row_range.second - row_range.first));
        cand_plates.push_back(cur_plate);
    }

    /**
     * for each possible strip gray image, find the region which can
     * be a plate image
     */
    for(int i=0; i<cand_plates.size(); ++i){
        cv::Mat cur_strip = cand_plates[i];
        //apply sobel operator
        Sobel(cur_strip, sobel_strip, CV_8U, 1, 0, 3, 1, 0);
        //Otsu thresholding
        cv::threshold(sobel_strip, threshold_strip, 60, 255, CV_THRESH_BINARY);
        cv::imshow("strip region", threshold_strip);
        vector< vector<Point> > contours;
        findContours(threshold_strip .clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        //iterate over the list of contour and find the bounding rectangle
        vector< vector<Point> >::iterator iter = contours.begin();
        while(iter!=contours.end()){
            //get the minimum area bounding rectangle
            RotatedRect rect = minAreaRect(Mat(*iter));
            Rect cur_rect = rect.boundingRect();
            //crop the image region containing the plate image
            Mat cur_plate = cur_strip (ClipRect(cur_rect, cv::Size(cur_strip.cols,cur_strip.rows)));
            if (!VerifyRegion(cur_rect)){
                iter = contours.erase(iter); //remove this region
            } else {
                plates.push_back(cur_plate);
                iter++;
            }
        }
    }

#if VERBOSE_MODE  //display all candidate region
    for (int i=0; i<plates.size(); ++i){
           QString idx = QString::number(i);
           QString win_name = "Candidate plate region" + idx;
           cv::imshow(win_name.toStdString(), plates[i]);
    }
#endif

    Visualize();
}
/**
 * @brief PlateDetector::NormalizeVectorAndFindSegment
 * Normalize the projection vector and at the same time,
 * find the segment where the values are greater than threshold
 * @param in_vec: vector to normalize
 */
void PlateDetector::NormalizeVectorAndFindSegment(QVector<double>& in_vec, QVector<QPair<int, int> >& segment)
{
    double maxVal = 0, minVal = 1000000;
    //Apply median filter
    //MedianFilter(in_vec, 5, minVal, maxVal);
    AvgFilter(in_vec, 9, minVal, maxVal);
    //normalize the vector: map the range of value to [0,1]
    bool in_segment = false;
    QPair<int, int> cur_segment;
    for(int i=0; i<in_vec.size();  ++i){
        in_vec[i] = (in_vec[i] - minVal)/(maxVal-minVal);
        if (in_vec[i] > threshold && !in_segment){  //start the segment
            cur_segment.first = i;
            in_segment = true;
        }
        if (in_vec[i] < threshold && in_segment) {  //end the segment
            cur_segment.second = i;
            in_segment = false;
            if (VerifySegment(cur_segment))
                segment.push_back(cur_segment);  //store the found segment
        }
    }

}

// calculate the sum of each column or row in a Mat
void PlateDetector::CalDimSum(const cv::Mat gra_img, QVector<double>& dim_sum, int dim)
{
    Mat img;
    if (dim == 0) //calculate sum of each row
        img = gra_img.clone();
    else if (dim == 1) //calculate sum of each column -> transpose input
        cv::transpose(gra_img, img);
    else {
        std::cout << "Unrecognized dimension flag!" << std::endl;
        return;
    }

    for(int i = 0; i < img.rows; i++)
    {
        const uchar* Mi = img.ptr<uchar>(i);
        double sum=0;
        for(int j = 0; j < img.cols; j++)
            sum += double(Mi[j]);
        dim_sum.push_back(sum);
    }

}

//visualize the projection and found segment
void PlateDetector::Visualize()
{
    /* Visualize the projection */
    QVector<double> row_idx(img_size.height), col_idx(img_size.width);
    for (int i=0; i<img_size.height; i++){
        row_idx[i] = i;
    }
    for (int i=0; i<img_size.width; i++){
        col_idx[i] = i;
    }

    w->plot(row_idx, row_sum, QString("Horizontal projection"));

    w->plotSegment(row_segment);

    w->display();
}

//verify if a found segment is valid or not
int PlateDetector::VerifySegment(const QPair<int, int> in_pair){
    if ((in_pair.second - in_pair.first) < 25)
        return 0;
    return 1;
}

void PlateDetector::AvgFilter(QVector<double>& in_vec, int filter_size,
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
void PlateDetector::MedianFilter(QVector<double>& in_vec, int filter_size,
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

//enlarge a rectangle to contain all the letters and numbers
//TODO: clip the enlarged region if it goes beyond the image border
//TODO: better way to enlarge: floodfil algorithm to include all pixels with white background
void PlateDetector::EnlargeRect(cv::Rect& rect)
{
    float center_x = float(rect.x) + float(rect.width)/2.0;
    float center_y = float(rect.y) + float(rect.height)/2.0;

    //get new size
    rect.width = int(enlarge_factor*float(rect.width)+0.5);
    rect.height= int(enlarge_factor*float(rect.height)+0.5);

    //get new top-left corner's coordinates
    rect.x = int(center_x - float(rect.width)/2.0 + 0.5);
    rect.y = int(center_y - float(rect.height)/2.0 + 0.5);
}

