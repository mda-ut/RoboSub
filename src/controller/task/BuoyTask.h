#ifndef BUOYTASK_H
#define BUOYTASK_H

///For the submarine movement
#include "Task.h"
#include "TurnTask.h"
#include "SpeedTask.h"
#include "DepthTask.h"
#include "CameraModel.h"
#include "ShapeFilter.h"
#include <unistd.h>

///For filter detection
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

using namespace cv;
using namespace std;

class BuoyTask: public Task
{
public:
    BuoyTask(Model* camModel, TurnTask* tk, SpeedTask* st, DepthTask* dt);
    virtual ~BuoyTask();

    void execute();

private:
    Logger* logger = new Logger("BuoyTask");
    Properties* settings;
    CameraModel* camModel;
    int imgWidth = -1;
    int imgHeight = -1;
    TurnTask* tk;
    SpeedTask* st;
    DepthTask* dt;
    void thresh_callback(int, Mat, float*, float*, float*);
    void detectBuoy(ImgData*, std::string, float*, float*, float*);

    // Load properties file
    PropertyReader* propReader;
    int erosion_size;
    int dilation_size;
    int thresh;
    int max_thresh;
    int alignRadius;
    int lowRedH;
    int lowRedS;
    int lowRedV;
    int highRedH;
    int highRedS;
    int highRedV;
    int lowRedH2;
    int lowRedS2;
    int lowRedV2;
    int highRedH2;
    int highRedS2;
    int highRedV2;
    int lowGreenH;
    int lowGreenS;
    int lowGreenV;
    int highGreenH;
    int highGreenS;
    int highGreenV;
    int lowGreenH2;
    int lowGreenS2;
    int lowGreenV2;
    int highGreenH2;
    int highGreenS2;
    int highGreenV2;

};

#endif // BUOYTASK_H
