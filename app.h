#ifndef APP_H
#define APP_H

#include "platedetector.h"
#include "platerecognizer.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
#include <QMainWindow>

using namespace std;
using namespace cv;

enum WorkingMode {
    UndefinedMode = -1,
    CollectMode = 0,   //collect character's image to train model
    TrainMode = 1,     //train model from collected images
    RunMode = 2        //perform plate detection & recognition on given image
};

class App : public QMainWindow
{
public:
    App();
    ~App();

    //load a single image
    int LoadImage(const std::string img_name);
    //load all images in a directory
    int LoadDirectory(const std::string path_to_dir);
    int Run();

private:
    PlateDetector* p_detector;
    PlateRecognizer* p_recognizer;

    //input data
    cv::Mat in_img;
    std::vector<cv::Mat> in_img_vec;


    //working mode
    WorkingMode work_mode;
};

#endif // APP_H
