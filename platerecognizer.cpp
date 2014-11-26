#include "platerecognizer.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <stdio.h>

using namespace cv;
using namespace std;

//ctor
PlateRecognizer::PlateRecognizer()
{

}


void PlateRecognizer::RecognizePlate(const cv::Mat& in_plate, std::string& content_plate)
{
    cvtColor(in_plate, gray_plate, CV_RGB2GRAY);
    //extract character's images
    ExtractCharacterImages(gray_plate);

#if VERBOSE_MODE
    cv::imshow("Gray image of plate", gray_plate);
    cv::imshow("Gray image of plate after thresholding", threshold_gray_plate);
#endif
}

//separate each character's image from the whole plate image
void PlateRecognizer::ExtractCharacterImages(const cv::Mat& in_plate_gray)
{
    //a simple thresholding
    cv::threshold(in_plate_gray, threshold_gray_plate, 60, 255, CV_THRESH_BINARY_INV);

    //find connected components
    vector< vector<Point> > contours;
    findContours(threshold_gray_plate.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    //iterate over the list of contour and find the bounding rectangle
    vector< vector<Point> >::iterator iter = contours.begin();
    while(iter!=contours.end()){
        //get the minimum area bounding rectangle
        RotatedRect rect = minAreaRect(Mat(*iter));
        Rect cur_rect = rect.boundingRect();
        //crop the image region containing the plate image
        Mat character= threshold_gray_plate(cur_rect);
        if (!VerifyCharacterRegion(character)){
            iter = contours.erase(iter); //remove this region
        } else {
            character_imgs.push_back(character);
            iter++;
        }
    }

#if VERBOSE_MODE
    for (int i=0; i<character_imgs.size(); i++){
        char window_name[100];
        sprintf(window_name, "extracted character %d from plate image", i+1);
        cv::imshow(window_name, character_imgs[i]);
    }
#endif
}

// verify if a region can contain a character image or not
int PlateRecognizer::VerifyCharacterRegion(const cv::Mat character_img)
{
    float ratio = float(character_img.cols) / float(character_img.rows);
    if (ratio < 1)
        ratio = 1.0/ratio;

    //ratio test
    if (ratio < 0.5 || ratio > 3)
        return 0;

    float area = float(character_img.rows)*float(character_img.cols);
    //area test
    if (area < 100)
        return 0;

    //white pixel proportion test: the number of white pixels must be greater than a fixed number
    Scalar sum = cv::sum(character_img);
    if ((float(sum[0])/area) < 0.4)
        return 0;

    return 1;
}
