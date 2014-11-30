#include "platedetector.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <QApplication>
#include <QPair>

using namespace std;
using namespace cv;
//ctor
PlateDetector::PlateDetector()
{
    enlarge_factor = 1.05;
    n_plot = 2;
    cur_plot = 0;
    w = new PlotWindow[n_plot];
    threshold = 0.66666;

    //allocate memory for the projection
    //col_sum = QVector<double>(img_size.width);
    //row_sum = QVector<double>(img_size.height);
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
    cv::imshow("horizontal gradient image", x_sobel_img);
    cv::imshow("vertical gradient image", y_sobel_img);
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
    CalDimSum(x_sobel_img, row_sum, 0);
    CalDimSum(y_sobel_img, col_sum, 1);

    //normalize the result
    NormalizeVectorAndFindSegment(row_sum, row_segment);
    NormalizeVectorAndFindSegment(col_sum, col_segment);

    Visualize();


#if VERBOSE_MODE
    //superposition the detected bounding rect into the input image
    //for (int i=0; i<boundingRect.size(); i++){
        //cv::rectangle(in_img, boundingRect[i], Scalar(0,255,0),1,8,0);
        //imshow("Detected plate image", plates[i]);
    //}
    //imshow("Input image with detected plate", in_img);
#endif
}
/**
 * @brief PlateDetector::NormalizeVectorAndFindSegment
 * Normalize the projection vector and at the same time,
 * find the segment where the values are greater than threshold
 * @param in_vec: vector to normalize
 */
void PlateDetector::NormalizeVectorAndFindSegment(QVector<double>& in_vec, QVector<QPair<int, int> >& segment)
{
    QVector<double>::iterator iter = in_vec.begin();
    double maxVal = 0, minVal = 1000000;
    while(iter!= in_vec.end()){
        if ((*iter) > maxVal)
            maxVal = (*iter);
        if ((*iter) < minVal)
            minVal = (*iter);
        ++iter;
    }

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

    w[0].plot(row_idx, row_sum, QString("Horizontal projection"));
    w[1].plot(col_idx, col_sum, QString("Vertical projection"));

    w[0].plotSegment(row_segment);
    w[1].plotSegment(col_segment);

    w[0].display();
    w[1].display();

}

//verify if a found segment is valid or not
int PlateDetector::VerifySegment(const QPair<int, int> in_pair){
    if ((in_pair.second - in_pair.first) < 10)
        return 0;
    return 1;
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
