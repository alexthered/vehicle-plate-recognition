#include "platedetector.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <QApplication>

using namespace std;
using namespace cv;
//ctor
PlateDetector::PlateDetector()
{
    enlarge_factor = 1.05;
    n_plot = 2;
    cur_plot = 0;
    w = new PlotWindow[n_plot];
}

PlateDetector::~PlateDetector()
{
    //delete all plotting windows
    delete[] w;
}

void PlateDetector::DetectPlate(const cv::Mat &_in_img, std::vector<cv::Mat>& plate_img)
{    
    in_img = _in_img.clone(); //clone the input image
    img_size =  Size(in_img.cols, in_img.rows); //get input image'size

    PreprocessImg(in_img);
    DetectRegion(gray_img);

    //copy plate image to output file
    plate_img = plates;

#if VERBOSE_MODE //show all intermediate results
    //cv::imshow("grayscale image after Gaussian blur", gray_img);
    //cv::imshow("horizontal gradient image", x_sobel_img);
    //cv::imshow("vertical gradient image", y_sobel_img);
#endif    

}

//Pre-process the image: grayscale conversion + Gaussian smoothing + histogram equalization
void PlateDetector::PreprocessImg(const cv::Mat& in_img)
{
    cvtColor(in_img, gray_img, CV_RGB2GRAY);

    //gaussian smoothing
    GaussianBlur(gray_img, gray_img, cv::Size(7,7), 1.0, 1.0);;
}

//detect regions which are possible to contain plate
void PlateDetector::DetectRegion(const cv::Mat& gray_img)
{
    //apply sobel filter to reveal horizontal and vertical gradient image
    Sobel(gray_img, x_sobel_img, CV_8U, 1, 0, 3, 1, 0);
    Sobel(gray_img, y_sobel_img, CV_8U, 0, 1, 3, 1, 0);

    /**
     *Now project the edge image into x and y axis
     */
    //QVector to store the sum of each column and row
    QVector<double> col_sum(img_size.width), row_sum(img_size.height);
    CalDimSum(x_sobel_img, row_sum, 0);
    CalDimSum(y_sobel_img, col_sum, 1);

    QVector<double> row_idx(img_size.height), col_idx(img_size.width);
    for (int i=0; i<img_size.height; i++){
        row_idx[i] = i;
    }
    for (int i=0; i<img_size.width; i++){
        col_idx[i] = i;
    }

    w[cur_plot++].plot(row_idx, row_sum, QString("Horizontal projection"));
    w[cur_plot++].plot(col_idx, col_sum, QString("Vertical projection"));
    //threshold the image using Otsu's algorithm

    //threshold(x_sobel_img, threshold_img, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
    /*
    //apply closing operator
    Mat ele = getStructuringElement(MORPH_RECT, Size(10,3)); //horizontal closing
    morphologyEx(threshold_img, threshold_img, CV_MOP_CLOSE, ele);

    //get list of connected component
    vector< vector<Point> > contours;
    vector<Rect> boundingRect;
    findContours(threshold_img.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    //iterate over the list of contour and find the bounding rectangle
    vector< vector<Point> >::iterator iter = contours.begin();
    while(iter!=contours.end()){
        //get the minimum area bounding rectangle
        RotatedRect rect = minAreaRect(Mat(*iter));
        Rect cur_rect = rect.boundingRect();
        if (!VerifyRegion(cur_rect)){
            iter = contours.erase(iter); //remove this region
        } else {
            //enlarge a rectangle a bit
            EnlargeRect(cur_rect);
            boundingRect.push_back(cur_rect);

            //crop the image region containing the plate image
            Mat plate_img = in_img(cur_rect);
            plates.push_back(plate_img);
            iter++;
        }
    }
    */

#if VERBOSE_MODE
    //superposition the detected bounding rect into the input image
    //for (int i=0; i<boundingRect.size(); i++){
        //cv::rectangle(in_img, boundingRect[i], Scalar(0,255,0),1,8,0);
        //imshow("Detected plate image", plates[i]);
    //}
    //imshow("Input image with detected plate", in_img);
#endif
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
        dim_sum[i] = sum;
    }
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
