#include "app.h"

App::App():
    QMainWindow()
{
    p_detector = new PlateDetector();
    p_recognizer = new PlateRecognizer();
}

App::~App()
{
    delete p_detector;
    delete p_recognizer;
}

int App::Run()
{
    /* read input image */
    Mat in_img = imread("../../data/P6070016.jpg");

    /* Detect plate region within an image (if any) */
    vector<Mat> plates;
    p_detector->DetectPlate(in_img, plates);

    std::string content_plate;
    /* Recognize plate image */
    for (int i=0; i<plates.size(); i++){
        std::cout << "Processing plate " << (i+1) << " out of " << plates.size() << std::endl;
        p_recognizer->RecognizePlate(plates[i], content_plate);
    }

    cv::imshow("Input image", in_img);
    cv::waitKey(0);

    return 1;
}
