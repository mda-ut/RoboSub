#ifndef SIMCAM_H
#define SIMCAM_H

#include <opencv2/opencv.hpp>

class SimCam
{
public:
    SimCam(cv::Mat *frame, int pos);
    cv::Mat* getFrame();
    int getPos();
private:
    cv::Mat *frame;
    int pos;
};

#endif // SIMCAM_H
