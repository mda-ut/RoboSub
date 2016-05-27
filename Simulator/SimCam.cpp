#include "SimCam.h"

SimCam::SimCam(cv::Mat *frame, int pos)
{
    this->frame = frame;
    this->pos = pos;
}

cv::Mat* SimCam::getFrame(){
    return frame;
}

int SimCam::getPos(){
    return pos;
}
