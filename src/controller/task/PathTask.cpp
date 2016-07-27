#include "PathTask.h"
#include "Timer.h"
#include <math.h>
#include <iostream>
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include "chrono"
#include "thread"
#include <cmath>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;
using namespace std;



//The destructor erases logs from memory
//new task should be paired with delete all the time.
PathTask::~PathTask() {
  delete logger;
}


/**
 * This function calculates the length of the line between 2 points
 * using pythagorean theorem
 */
static double lineLength (cv::Point p1, cv::Point p2) {
  float side= sqrt(pow(p1.x-p2.x,2.0)+(pow(p1.y-p2.y,2.0)));
  return side;
}


/**
 * Helper function to display text in the center of a contour
 */
void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour){
  int fontface = cv::FONT_HERSHEY_SIMPLEX;
  double scale = 0.4;
  int thickness = 1;
  int baseline = 0;

  cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
  cv::Rect r = cv::boundingRect(contour);

  cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
  cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255,255,255), CV_FILLED);
  cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}

/**
 * Helper function to find maximum side out of sides
 * parameter returned is index into point vector of first
 *
 */
int maxSide(std::vector<cv::Point> approx){
  int curMax = 0;

  int numVertices=approx.size();
  int i;
  for (i = 0; i < numVertices; i++){
    if (lineLength(approx[i], approx[(i+1) % numVertices]) > lineLength(approx[curMax], approx[(curMax+1) % numVertices])){
      curMax = i;
    }
  }
  return curMax;

}


/**
 * This defines the task we draw upon
 * 'Constructor'
 *
 */
PathTask::PathTask(Model* cameraModel, TurnTask *turnTask, SpeedTask *speedTask) {
  this->cameraModel = dynamic_cast<CameraModel*>(cameraModel);
  this->turnTask = turnTask;
  this->speedTask = speedTask;
  moving = false;
}


/**
 * Do we use this?
 *
 */
void PathTask::setSpeed(float amount) {
  speedTask->setTargetSpeed(amount);
  speedTask->execute();
  moving = true;
  if (amount != 0) {
  } else {
    moving = false;
  }
  logger->info("Speed set to " + std::to_string(amount));

}

/**
 * Do we use this?
 *
 */
void PathTask::stop() {
  // Stop
  setSpeed(0);
}

/**
 * James' function for rotating using Turntask
 * takes input in ******degrees********
 *
 */
void PathTask::rotate(float angle) {
  logger->debug("Rotating sub by " + std::to_string(angle) + " degrees");
  turnTask->setYawCurrentDelta(angle);
  turnTask->execute();
  usleep(1000000*abs(angle)/180*10);
}

/**
 *
 * James' function to move to a select place
 *
 */
void PathTask::moveTo(cv::Point2f pos) {
  //This assumes that (0,0) is in top left
  float imgHeightF = static_cast<float>(imgHeight);
  float imgWidthF = static_cast<float>(imgWidth);
  int backwards=1;
  int sign;
  printf("POSITION: %f %f\n", pos.y, pos.x);
  printf("START: %f %f\n", pos.y-imgHeightF/2, pos.x-imgWidthF/2);
  if (std::abs(pos.x - imgWidthF / 2) < alignThreshold) {
    //float distance = std::sqrt(pos.x * pos.x + pos.y * pos.y);
    if (pos.y - imgHeightF / 2 <0) {
      logger->info("Moveto moving forwards");
      setSpeed(forwardSpeed);
    } else {
      logger->info("Moveto moving backwards");
      setSpeed(-forwardSpeed);
    }
  }
  else {
    setSpeed(0);
    //in the case that x is not aligned,
    //strategy is to rotate in right direction and move either forwards or backwards
    //to center the path in the camera
    //but never move more than 90 degrees so we don't lose orientation.
    printf("Move to: %f %f\n", pos.y, pos.x);
    //find the angle we need to move.
    float ang = -1*atan2(-1*(pos.x - imgWidthF / 2), (imgHeightF/ 2-pos.y)) * 180 / M_PI;
    if (std::abs(ang)>=90){
      //if more than 90,must rotate and move backwards.
      backwards=-1; //this is a multiplier giving which direction to move
      sign=ang/std::abs(ang);
      ang=-1*(sign)*(180-std::abs(ang));
      //setSpeed(-forwardSpeed);
    }
    logger->info("moveto Rotating " + std::to_string(ang) + " degrees");
    rotate(ang);
    //timing?
    sleep(0.5);
  }
}

/**
 *The actual pathtask
 *
 */
void PathTask::execute() {
  float ajusterAngle=0;
  int accumulator = 0;
  float accMassX = 0;
  float accMassY = 0;
  float accAngle = 0;
  // Load properties
  PropertyReader* propReader;
  Properties* settings;
  propReader = new PropertyReader("settings/path_task_settings.txt");
  settings = propReader->load();
  ImgData* data = dynamic_cast<ImgData*> (dynamic_cast<CameraState*>(cameraModel->getState())->getDeepState("raw"));
  namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control", basically an ajustable HSV filter

  //parameters to distinguish orange from other colors: hardcoded.
  int iLowH = std::stoi(settings->getProperty("LOWH"));
  int iHighH = std::stoi(settings->getProperty("HIGHH"));

  int iLowS =std::stoi(settings->getProperty("LOWS"));
  int iHighS = std::stoi(settings->getProperty("HIGHS"));

  int iLowV = std::stoi(settings->getProperty("LOWV"));
  int iHighV = std::stoi(settings->getProperty("HIGHV"));

  alignThreshold = std::stoi(settings->getProperty("ALIGN_THRESHOLD"));
  forwardSpeed = std::stoi(settings->getProperty("FORWARD_SPEED"));
  rect_angle_threshold = std::stoi(settings->getProperty("RECTANGLE_ANGLE_THRESHOLD"));


  //Create trackbars in "Control" window
  createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
  createTrackbar("HighH", "Control", &iHighH,179);
  createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
  createTrackbar("HighS", "Control", &iHighS, 255);
  createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
  createTrackbar("HighV", "Control", &iHighV, 255);

  //Capture a temporary image from the camera
  Mat imgTmp;
  //     cap.read(imgTmp);
  cout << "about to threshold image\n";
  cv::namedWindow("imgThresholded",CV_WINDOW_AUTOSIZE);
  cv::moveWindow("imgThresholded", 400, 500);
  cv::namedWindow("HSV",CV_WINDOW_AUTOSIZE);
  cv::moveWindow("HSV", 100, 200);

  bool angleThresholdMet = false;
  cout << "entering the while loop\n";
  bool centeredOnce = false;
  while (!angleThresholdMet){

    delete data;
    data = dynamic_cast<ImgData*> (dynamic_cast<CameraState*>(cameraModel->getState())->getDeepState("raw"));


    Mat imgOriginal;
    logger->trace("Got image from camera");
    imgOriginal = data->getImg();
    cv::Size s = imgOriginal.size();
    imgWidth = s.width;
    imgHeight = s.height;
    float imgHeightF = static_cast<float>(imgHeight);
    float imgWidthF = static_cast<float>(imgWidth);

    Mat imgHSV;
    Mat imgThresholded;
    cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image


    //morphological opening (removes small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    //morphological closing (removes small holes from the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );


    logger->trace("Thresholded input");


    // Use Canny instead of threshold to catch squares with gradient shading
    //canny algorithm is an ubedge detector
    cv::Mat bw;
    cv::Canny(imgThresholded, bw, 10, 50, 5);
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    cout << " found contours\n";
    //if there's no contour found, re-iterate the while loop
    if (contours.size()<1){
      setSpeed(forwardSpeed);
      continue;
    }
    accumulator++;
    std::vector<cv::Point> approx;


    // Get the moments
    vector<Moments> mu(contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
      { mu[i] = moments( contours[i], false ); }

    //  Get the mass centers:
    vector<Point2f> mc( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
      { mc[i] = Point2f( static_cast<float>(mu[i].m10/mu[i].m00) , static_cast<float>(mu[i].m01/mu[i].m00) ); }


    cv::circle(imgHSV, mc[0], 5, Scalar(180,105,255));

    imshow("HSV", imgHSV);
    imshow("bw", bw);

    /**
     *Do you see a shape
     *
     */

    printf("contours.size %d\n", contours.size());
    // printf("contours at 0, %d\n",contours[0]);
    for (int i = 0; i < contours.size(); i++){

      //printf("Contours size: %d\n", contours.size());
      // Approximate contour with accuracy proportional
      // to the contour perimeter
      cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

      // Skip small or non-convex objects CS is this actually working???
      if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
        continue;

      // draw approx on the img threshold screen
      //             for (int i = 0; i < approx.size(); i++){
      //                 cv::circle(bw, approx.at(i), 5, Scalar(255,0,0));
      //             }

      //             imshow("imgThresholded", bw);


      /**
       *orient relative to the sub IF a rectangle shape has been found
       *
       */
      if (approx.size() == 4 || approx.size() == 5 || approx.size() == 6){

        //find distance between the contour and the centre
        //                slide(deltaX)
        /*
         *Find the angle relative to vertical of the max side
         *
         */
        int maxS = maxSide(approx);
        int numVertices=approx.size();
        logger->debug("MAX SIDE point 1 "+ std::to_string(maxS)+ "MAX SIDE point 2 "+std::to_string((maxS +1)%numVertices));
        float dx=approx[maxS].x - approx[(maxS+1)%numVertices].x;
        logger->debug("dx "+ std::to_string(dx));
        float hyp=lineLength(approx[maxS], approx[(maxS+1)%numVertices]);
        logger->debug("hypoteneuse "+ std::to_string(hyp));
        ajusterAngle=(asin(dx/hyp)*180/(M_PI));

        //then if you know that the difference in
        //y is negative, then reverse the angle.
        if ((approx[maxS].y - approx[(maxS+1)%numVertices].y)<0){
          ajusterAngle=-1*ajusterAngle;
        }

        logger->info("the path is "+ std::to_string(ajusterAngle)+"degrees from the reference");
        //if(abs(ajusterAngle) < 2.5) angleThresholdMet = true; // threshold to get out of loop

        cv::Mat imgLines = imgOriginal.clone();
        line(imgLines, approx[0], approx[1], Scalar(0,0,255), 2);
        line(imgLines, approx[1], approx[2], Scalar(0,0,255), 2);
        line(imgLines, approx[2], approx[3], Scalar(0,0,255), 2);
        line(imgLines, approx[3], approx[0], Scalar(0,0,255), 2);
        imshow("contours", imgLines);

      }//if loop end

    }//for loop
    //imshow("imgThresholded", bw);

    /**
     * Are we close enough?
     *
     */
    bool closeEnough=false;
    cv::Point origin( imgWidthF/2, imgHeightF/2);
    if (lineLength(mc[0], origin)<30) {
      closeEnough=true;
      setSpeed(0);
    }

    /**
     *Move to the centre of the contour detected
     * IF we are not close enough
     *
     */

    if (!closeEnough && !centeredOnce){
      //if we aren't close enough
      //go back to the beginning of loop to try again
      //printf("contours.size %d\n", contours.size());
      accMassX += mc[0].x;
      accMassY += mc[0].y;
      if (accumulator == 15){
        cv::Point avgMass( accMassX/15, accMassY/15);
        moveTo(avgMass);
        accMassX = 0;
        accMassY = 0;
        accumulator = 0;
        accAngle = 0;

      }

    }
    //rotating relative to the rectangle we have found.
    else {
      centeredOnce = true;
      accAngle += ajusterAngle;
      if (accumulator == 15){
        accumulator = 0;
        if(abs(accAngle)/15 < rect_angle_threshold ){
          angleThresholdMet = true; // threshold to get out of loop
        }
        else    {
          rotate(-1*accAngle/15);
        }
        accAngle = 0;
        accMassX = 0;
        accMassY = 0;

      }
    }
  }

  //for competition, start moving along path
  setSpeed(20);
  logger->info("EXITING");
  return;
}
