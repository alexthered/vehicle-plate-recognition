#include "platerecognizer.h"
#include "common.h"
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


void PlateRecognizer::RecognizePlate(const cv::Mat& _in_plate, std::string& content_plate)
{
    in_plate = _in_plate.clone();

    cvtColor(in_plate, gray_plate, CV_RGB2GRAY);

    //histogram equalization to boost the constrast
    cv::equalizeHist(gray_plate, gray_plate);

    //a simple thresholding
    cv::threshold(gray_plate, threshold_gray_plate, 65, 255, CV_THRESH_BINARY_INV);

    //extract character's images
    ExtractCharacterImages();

#if VERBOSE_MODE
    cv::imshow("Gray image of plate", gray_plate);
    cv::imshow("Gray image of plate after thresholding", threshold_gray_plate);
#endif
}

//separate each character's image from the whole plate image
void PlateRecognizer::ExtractCharacterImages()
{
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
        Mat character= in_plate(ClipRect(cur_rect, Size(gray_plate.cols, gray_plate.rows)));
        if (!VerifyCharacterRegion(character)){
            iter = contours.erase(iter); //remove this region
        } else {
            character_imgs.push_back(character);
            iter++;
        }
    }

#if VERBOSE_MODE
    for (int i=0; i<character_imgs.size(); i++){
        QString idx = QString::number(i);
        QString win_name = "Extracted character " + idx + " from plate image";
        cv::imshow(win_name.toStdString(), character_imgs[i]);
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
    if (ratio < 0.2 || ratio > 8)
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
