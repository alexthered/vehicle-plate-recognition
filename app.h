#ifndef APP_H
#define APP_H

#include "platedetector.h"
#include "platerecognizer.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>


using namespace std;
using namespace cv;

class App
{
public:
    App();

    int Run();
};

#endif // APP_H
