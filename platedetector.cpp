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
#if PLOT_DISPLAYING
    n_plot = 2;
    cur_plot = 0;
    w = new PlotWindow;
#endif
    threshold = 0.5;
}

PlateDetector::~PlateDetector()
{
 #if PLOT_DISPLAYING
    //delete all plotting windows
    delete w;
 #endif
}

void PlateDetector::DetectPlate(const cv::Mat &_in_img, std::vector<cv::Mat>& plate_img)
{    
    in_img = _in_img.clone(); //clone the input image
    img_size =  Size(in_img.cols, in_img.rows); //get input image'size

    PreprocessImg(in_img);
    DetectRegion();

    //copy plate image to output file
    plate_img = plates;

#if VERBOSE_MODE //show all intermediate results
    //cv::imshow("grayscale image after Gaussian blur", gray_img);
    cv::imshow("horizontal gradient image", x_sobel_img);
    //cv::imshow("vertical gradient image", y_sobel_img);
#endif    

}

//Pre-process the image: grayscale conversion + Gaussian smoothing + histogram equalization
void PlateDetector::PreprocessImg(const cv::Mat& _in_img)
{
    //copy input image
    in_img = _in_img.clone();

    cvtColor(in_img, gray_img, CV_RGB2GRAY);

    //gaussian smoothing
    GaussianBlur(gray_img, gray_img, cv::Size(7,7), 1.5, 1.5);;
}

//detect regions which are possible to contain plate
void PlateDetector::DetectRegion()
{
    //apply sobel filter to reveal horizontal and vertical gradient image
    Sobel(gray_img, x_sobel_img, CV_8U, 1, 0, 3, 1, 0);

    /**
     *Now project the edge image into x and y axis
     */
    CalDimSum(x_sobel_img, row_sum, 0);

    //normalize the result
    NormalizeVectorAndFindSegment(row_sum, row_segment);

    /**
     * First step: Detect image strips which can contain the plate
     *
     * */
    //get all possible strip regions which can contain plates
    for (int i=0; i<row_segment.size(); ++i){
        QPair<int, int> row_range = row_segment[i];
        //rectangle containing strip region
        cv::Rect cur_rect = Rect(0, row_range.first,
                                  in_img.cols - 1,
                                  row_range.second - row_range.first);
        cand_plates.push_back(cur_rect);
    }

    /**
     * Second step:
     * for each possible strip image, find the region which can
     * be a plate image
     */
    for(int i=0; i<cand_plates.size(); ++i){
        vector<Rect> strip_plate;
        //get the strip image from input gray image
        cv::Mat cur_strip = gray_img(cand_plates[i]);
        //apply sobel operator
        Sobel(cur_strip, sobel_strip, CV_8U, 1, 0, 3, 1, 0);
        //Otsu thresholding
        cv::threshold(sobel_strip, threshold_strip, 60, 255, CV_THRESH_BINARY);
        //apply closing operator
        cv::Mat ele = cv::getStructuringElement(MORPH_RECT, Size(15,3));
        morphologyEx(threshold_strip, threshold_strip, CV_MOP_CLOSE, ele);
        cv::imshow("strip region", threshold_strip);

        //collect connected components
        vector< vector<Point> > contours;
        findContours(threshold_strip.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        //iterate over the list of contour and find the bounding rectangle
        vector< vector<Point> >::iterator iter = contours.begin();
        while(iter!=contours.end()){
            //get the minimum area bounding rectangle and convert to original image's coordinate
            RotatedRect rect = StripRotatedRectToImageRotatedRect(minAreaRect(Mat(*iter)), cand_plates[i]);
            //crop the image region containing the plate image
            if (rect.size.width * rect.size.height >= 500){
                //perform flood fill
                FloodFill(in_img, rect);
                //get the bounding rectangle and convert it to original image's coordinate (with clipping)
                Rect cur_rect = ClipRect(rect.boundingRect(),
                                         cv::Size(gray_img.cols,gray_img.rows));
                if (VerifySize(cur_rect)){
                    strip_plate.push_back(cur_rect);
                }
            }
            iter = contours.erase(iter); //erase this region
        }

        //remove overlapping regions in this strip
        while(strip_plate.size()!=0){
            vector<Rect>::iterator iter_i = strip_plate.begin();
            vector<Rect>::iterator iter_j = strip_plate.begin();
            iter_j++;
            while(iter_j!= strip_plate.end()){
                //compare if two region is similar
                if (IsSimilarRect((*iter_i),(*iter_j))){
                    (*iter_i) |= (*iter_j); //grouping
                    strip_plate.erase(iter_j); //delete it
                } else
                    iter_j++;
            }
            //get the plate image
            std::cout << ((*iter_i).x) << " " << ((*iter_i).y) << " " << ((*iter_i).width) << " " <<((*iter_i).height) << std::endl;
            Mat cur_plate = in_img((*iter_i));
            plates.push_back(cur_plate);
            strip_plate.erase(iter_i); //delete iter_i
        }
    }

#if VERBOSE_MODE  //display all candidate region
    for (int i=0; i<plates.size(); ++i){
           QString idx = QString::number(i);
           QString win_name = "Candidate plate region" + idx;
           cv::imshow(win_name.toStdString(), plates[i]);
    }
#endif


#if PLOT_DISPLAYING
    Visualize();
#endif
}

/**
 * Perform flood fill algorithm since the background
 * of the plate is always homogeneous (white or yellow)
 * */

void PlateDetector::FloodFill(const cv::Mat input_img, cv::RotatedRect& min_rect)
{
    //get the minimum size
    int min_size = (min_rect.size.width < min_rect.size.height)?(min_rect.size.width):(min_rect.size.height);
    //initialize rand
    srand(time(NULL));

    //initialize floodfill parameters and variables
    cv::Mat mask = Mat(input_img.rows+2, input_img.cols+2, CV_8UC1);
    mask = Scalar::all(0); //set mask to all 0

    int loDiff = 30;
    int upDiff = 30;
    int connectivity = 4;
    int newMaskVal = 255;
    int NumSeeds = 10;
    Rect ccomp;
    int flags = connectivity + (newMaskVal << 8 ) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;
    for(int j=0; j<NumSeeds; j++){
        Point seed;
        seed.x=min_rect.center.x+rand()%(int)min_size-(min_size/2);
        seed.y=min_rect.center.y+rand()%(int)min_size-(min_size/2);
        int area = floodFill(input_img, mask, seed, Scalar(255,0,0), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff), flags);
    }

#if VERBOSE_MODE
    //cv::imshow("Mask from flood fill", mask);
#endif

    //Check new floodfill mask match for a correct patch.
    //Get all points detected for get Minimal rotated Rect
    vector<Point> pointsInterest;
    Mat_<uchar>::iterator itMask= mask.begin<uchar>();
    Mat_<uchar>::iterator end= mask.end<uchar>();
    for( ; itMask!=end; ++itMask)
        if(*itMask==255)
            pointsInterest.push_back(itMask.pos());
    //adjust the new rotated rect
    min_rect = minAreaRect(pointsInterest);
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
#if PLOT_DISPLAYING
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

#endif

//verify if a found segment is valid or not
int PlateDetector::VerifySegment(const QPair<int, int> in_pair){
    if ((in_pair.second - in_pair.first) < 25)
        return 0;
    return 1;
}

//verify if a region is possible to contain a plate
int PlateDetector::VerifySize(const cv::Rect rect)
{
    //max width and height size requirement
    int min_size = (rect.width < rect.height) ? rect.width : rect.height;
    if (min_size > 100)
        return 0;
    //get the width-height ratio
    float ratio = float(rect.width)/float(rect.height);
    if (ratio < 1)
        ratio = 1.0/ratio;

    if (ratio < 2 || ratio >15)
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

