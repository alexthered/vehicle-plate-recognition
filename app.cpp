#include "app.h"

App::App()
{
}

int App::Run()
{
    /* read input image */
    Mat in_img = imread("../../data/P6070015.jpg");

    /* Detect plate region within an image (if any) */
    PlateDetector p_detector;
    vector<Mat> plates;
    p_detector.DetectPlate(in_img, plates);


    /* Recognize plate image */
    for (int i=0; i<plates.size(); i++){
        cv::imshow("Detected plate image", plates[i]);
        std::cout << "Processing plate " << (i+1) << " out of " << plates.size() << std::endl;
        //p_recognizer.RecognizePlate(plates[i], content_plate);
    }

    cv::imshow("Input image", in_img);
    waitKey(0);

    return 1;
}
