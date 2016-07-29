#ifndef PATHTASK_H
#define PATHTASK_H

#include "Task.h"
#include "TurnTask.h"
#include "SpeedTask.h"
#include "CameraModel.h"
#include "LineFilter.h"
#include "ShapeFilter.h"
#include "Logger.h"
#include <unistd.h>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

class PathTask : public Task {
public:
    virtual ~PathTask();
    PathTask(Model* cameraModel, TurnTask* turnTask, SpeedTask* speedTask);

    void execute();

    bool filterRect(cv::Mat img, cv::Point2f &center, std::vector<cv::Point> &poly, std::vector<int> hsv);
private:
    Logger* logger = new Logger("PathTask");

    CameraModel* cameraModel;
    TurnTask* turnTask;
    SpeedTask* speedTask;

    bool moving;
    bool done;
    bool invertThresholded;

    float forwardSpeed;

    // distance in pixels considered to be in line with the sub
    float alignThreshold;
    float rect_angle_threshold;
    int imgWidth, imgHeight;

    // Helper functions
    void setSpeed(float amount);
    void stop();
    void rotate(float amount);
    void moveTo(cv::Point2f pos);
    double rectAngle(std::vector<cv::Point> contour);
};

#endif // PATHTASK_H
