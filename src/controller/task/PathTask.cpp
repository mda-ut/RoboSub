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
 *
 */
int maxSide(std::vector<cv::Point> approx){
    int curMax = 0;
    int i;
    for (i = 0; i < 4; i++){
        if (lineLength(approx[i], approx[(i+1) % 4]) > lineLength(approx[curMax], approx[(curMax+1) % 4])){
            curMax = i;
        }
    }
    return curMax;

}




/**
 * This defines the task we draw upon
 *
 */
PathTask::PathTask(Model* cameraModel, TurnTask *turnTask, SpeedTask *speedTask) {
    this->cameraModel = dynamic_cast<CameraModel*>(cameraModel);
    this->turnTask = turnTask;
    this->speedTask = speedTask;
    moving = false;
    alignThreshold = 75;
    forwardSpeed = 20;

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
    turnTask->setYawDelta(angle);
    turnTask->execute();
    usleep(500000);
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
    } else {
        printf("Move to: %f %f\n", pos.y, pos.x);
        float ang = atan2(-1*(pos.x - imgWidthF / 2), (pos.y - imgHeightF/ 2)) * 180 / M_PI;

//        float ang = 5;
        if ((pos.y - imgHeightF/ 2)>0){
            //if y displacement is positive, the angle you need to move is greater than 90
            ang=ang-(ang/std::abs(ang)*M_PI);
        }
        logger->info("moveto Rotating " + std::to_string(ang) + " degrees");
        rotate(ang);
        sleep(1);
        //setSpeed(forwardSpeed);
    }
}

/**
 *The actual pathtask
 *
 */
void PathTask::execute() {

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

     cv::namedWindow("imgThresholded",CV_WINDOW_AUTOSIZE);
     cv::moveWindow("imgThresholded", 400, 500);
     cv::namedWindow("HSV",CV_WINDOW_AUTOSIZE);
     cv::moveWindow("HSV", 100, 200);

     bool angleThresholdMet = false;
     while (!angleThresholdMet){
         delete data;
         data = dynamic_cast<ImgData*> (dynamic_cast<CameraState*>(cameraModel->getState())->getDeepState("raw"));

         Mat imgOriginal;
         logger->trace("Got image from camera");
         imgOriginal = data->getImg();
         cv::Size s = imgOriginal.size();
         imgWidth = s.width;
         imgHeight = s.height;

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


         for (int i = 0; i < contours.size(); i++){

             //printf("Contours size: %d\n", contours.size());
             // Approximate contour with accuracy proportional
             // to the contour perimeter
             cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

             // Skip small or non-convex objects CS is this actually working???
             if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
                continue;

             // draw approx on the img threshold screen
             for (int i = 0; i < approx.size(); i++){
                 cv::circle(bw, approx.at(i), 5, Scalar(255,0,0));
             }

//             imshow("imgThresholded", bw);


             /**
              *orient relative to the sub IF a rectangle shape has been found
             *
             */
             if (approx.size() == 4 || approx.size() == 5 || approx.size() == 6){
                /**
                 *orient into the center of the screen
                 * if the path is NOT within middle 20%
                *
                */
                //find distance between the contour and the centre
//                slide(deltaX)
                /**
                 *Do math to find the angle relative to vertical of the max side
                *
                */

                int maxS = maxSide(approx);

                logger->debug("MAX SIDE point 1 "+ std::to_string(maxS)+ "MAX SIDE point 2 "+std::to_string((maxS +1)%4));
                float dx=approx[maxS].x - approx[(maxS+1)%4].x;
                logger->debug("dx "+ std::to_string(dx));
                float hyp=lineLength(approx[maxS], approx[(maxS+1)%4]);
                logger->debug("hypoteneuse "+ std::to_string(hyp));
                float ajusterAngle=(asin(dx/hyp)*180/(M_PI));

                //then if you know that the difference in
                //y is negative, then reverse the angle.
                if ((approx[maxS].y - approx[(maxS+1)%4].y)<0){
                    ajusterAngle=-1*ajusterAngle;
                }

                logger->info("the path is "+ std::to_string(ajusterAngle)+"degrees from the reference");
                //if(abs(ajusterAngle) < 2.5) angleThresholdMet = true; // threshold to get out of loop
                rotate(-1*ajusterAngle);
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
        cv::Point origin( imgWidth/2, imgHeight/2);
        if (lineLength(mc[0], origin)<50) {
            closeEnough=true;
        }

         /**
          *Move to the centre of the contour detected
          * IF we are not close enough
         *
         */

         if (!closeEnough){
             //if we aren't close enough
             //go back to the beginning of loop to try again
             moveTo(mc[0]);
             continue;
         }
     }

     logger->info("EXITING");
    return;
}
