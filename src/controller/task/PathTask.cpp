#include "PathTask.h"
#include "Timer.h"
#include <math.h>
#include <iostream>
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include "chrono"
#include "thread"
#include <cmath>

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

// comparison function object
bool compareContourAreas ( std::vector<cv::Point> contour1, std::vector<cv::Point> contour2 ) {
    double i = fabs( contourArea(cv::Mat(contour1)) );
    double j = fabs( contourArea(cv::Mat(contour2)) );
    return ( i > j );
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

    char buffer[80];
    strftime(buffer, 80, "PATH%I:%M:%S", timer.getTimeStamp());
    foldername = std::string(buffer);
    system( ("mkdir "+foldername).c_str() );

}

bool PathTask::filterRect(Mat img, Point2f &center, std::vector<Point> &poly, vector<int> hsv) {
    Mat imgOriginal = cv::Mat(img.clone());
    imgOriginal = img;
    imshow("image", imgOriginal);
    Mat imgHSV;
    Mat imgThresholded;
    cvtColor(imgOriginal.clone(), imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
    inRange(imgHSV, Scalar(hsv[0], hsv[2], hsv[4]), Scalar(hsv[1], hsv[3], hsv[5]), imgThresholded); //Threshold the image

    //morphological opening (removes small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    //morphological closing (removes small holes from the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    logger->trace("Thresholded input");

    //take inverted image of imgThresholded
    if(invertThresholded){
        Mat imgThresholdedInv;
        threshold( imgThresholded, imgThresholdedInv, 70, 255,1);
        //imshow("imgThresholdedInv", imgThresholdedInv);
        imgThresholded = imgThresholdedInv;

    }

    //place a 5 pixel border around the image to help with contour detection
    for (int y = 0; y < imgThresholded.cols; y++) {
        for (int x = 0; x < 5; x++) {
            imgThresholded.at<uchar>(x,y) = 0;
            imgThresholded.at<uchar>(imgThresholded.rows-1-x, y) = 0;
        }
    }
    for (int y = 0; y < imgThresholded.rows; y++) {
        for (int x = 0; x < 5; x++) {
            imgThresholded.at<uchar>(y,x) = 0;
            imgThresholded.at<uchar>(y, imgThresholded.cols-1-x) = 0;
        }
    }
    // Use Canny instead of threshold to catch squares with gradient shading
    //canny algorithm is an ubedge detector
    cv::Mat bw;
    cv::Canny(imgThresholded, bw, 10, 50, 5);
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    bool rectangleFound = false;
    if (contours.size()>0){
        logger->debug("found contours");
        // sort contours
        std::sort(contours.begin(), contours.end(), compareContourAreas);

        std::vector<cv::Point> approx;

        // Get the moments
        vector<Moments> mu(contours.size() );
        for( size_t i = 0; i < contours.size(); i++ )
          { mu[i] = moments( contours[i], false ); }

        //  Get the mass centers:
        vector<Point2f> mc( contours.size() );
        for( size_t i = 0; i < contours.size(); i++ ) {
            mc[i] = Point2f( static_cast<float>(mu[i].m10/mu[i].m00) , static_cast<float>(mu[i].m01/mu[i].m00) );
        }

        /**
         *Do you see a shape
         *
         */
        for (int i = 0; i < contours.size(); i++){
          // Approximate contour with accuracy proportional
          // to the contour perimeter
          cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

          // Skip small or non-convex objects CS is this actually working???
          if (std::fabs(cv::contourArea(contours[i])) < 100 || !cv::isContourConvex(approx))
            continue;

          if (approx.size() == 4 || approx.size() == 5 || approx.size() == 6){
            rectangleFound = true;
            poly = approx;
            center = mc[i];

            cv::Mat imgLines = img.clone();
            for (int i = 0; i < approx.size(); i++) {
                line(imgLines, approx[i], approx[(i+1)%approx.size()], Scalar(0,0,255), 2);
            }
            imshow("contour", imgLines);
            break;
          }//if loop end

        }//for loop
    }
    return rectangleFound;
}

//given contour, find angle of longest side(s)
double PathTask::rectAngle(std::vector<cv::Point> contour) {
    int maxS = maxSide(contour);
    int numVertices=contour.size();
    logger->debug("MAX SIDE point 1 "+ std::to_string(maxS)+ "MAX SIDE point 2 "+std::to_string((maxS +1)%numVertices));
    float dx=contour[maxS].x - contour[(maxS+1)%numVertices].x;
    logger->debug("dx "+ std::to_string(dx));
    float hyp=lineLength(contour[maxS], contour[(maxS+1)%numVertices]);
    logger->debug("hypoteneuse "+ std::to_string(hyp));
    double ajusterAngle=(asin(dx/hyp)*180/(M_PI));

    //then if you know that the difference in
    //y is negative, then reverse the angle.
    if ((contour[maxS].y - contour[(maxS+1)%numVertices].y)<0){
      ajusterAngle=-1*ajusterAngle;
    }

    logger->info("the path is "+ std::to_string(ajusterAngle)+"degrees from the reference");
    //if(abs(ajusterAngle) < 2.5) angleThresholdMet = true; // threshold to get out of loop

    return ajusterAngle;
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
  //everything assumes that (0,0) is in top left
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
  Timer timer;
  float ajusterAngle=0;
  int accumulator = 0;
  int centeredAccumulator = 0;
  float accMassX = 0;
  float accMassY = 0;
  float accAngle = 0;


  // Load properties
  PropertyReader* propReader;
  Properties* settings;
  propReader = new PropertyReader("settings/path_task_settings.txt");
  settings = propReader->load();
  ImgData* data = dynamic_cast<ImgData*> (dynamic_cast<CameraState*>(cameraModel->getState())->getDeepState("raw"));
  //namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control", basically an ajustable HSV filter

  invertThresholded = std::stoi(settings->getProperty("INVERT_THRESHOLD"));
  //parameters to distinguish orange from other colors: hardcoded.

  vector<vector<int>> hsv_settings;
  hsv_settings.resize(3);

  hsv_settings[0].push_back(std::stoi(settings->getProperty("LOWH1")));
  hsv_settings[0].push_back(std::stoi(settings->getProperty("HIGHH1")));
  hsv_settings[0].push_back(std::stoi(settings->getProperty("LOWS1")));
  hsv_settings[0].push_back(std::stoi(settings->getProperty("HIGHS1")));
  hsv_settings[0].push_back(std::stoi(settings->getProperty("LOWV1")));
  hsv_settings[0].push_back(std::stoi(settings->getProperty("HIGHV1")));

  hsv_settings[1].push_back(std::stoi(settings->getProperty("LOWH2")));
  hsv_settings[1].push_back(std::stoi(settings->getProperty("HIGHH2")));
  hsv_settings[1].push_back(std::stoi(settings->getProperty("LOWS2")));
  hsv_settings[1].push_back(std::stoi(settings->getProperty("HIGHS2")));
  hsv_settings[1].push_back(std::stoi(settings->getProperty("LOWV2")));
  hsv_settings[1].push_back(std::stoi(settings->getProperty("HIGHV2")));

  hsv_settings[2].push_back(std::stoi(settings->getProperty("LOWH3")));
  hsv_settings[2].push_back(std::stoi(settings->getProperty("HIGHH3")));
  hsv_settings[2].push_back(std::stoi(settings->getProperty("LOWS3")));
  hsv_settings[2].push_back(std::stoi(settings->getProperty("HIGHS3")));
  hsv_settings[2].push_back(std::stoi(settings->getProperty("LOWV3")));
  hsv_settings[2].push_back(std::stoi(settings->getProperty("HIGHV3")));

  //if we prefer the inverted thresholds
  if (invertThresholded){

      int LowH1 = std::stoi(settings->getProperty("LOWH_I"));
      int HighH1 = std::stoi(settings->getProperty("HIGHH_I"));

      int LowS1 =std::stoi(settings->getProperty("LOWS_I"));
      int HighS1 = std::stoi(settings->getProperty("HIGHS_I"));

      int LowV1 = std::stoi(settings->getProperty("LOWV_I"));
      int HighV1 = std::stoi(settings->getProperty("HIGHV_I"));

  }


  int timeout = std::stoi(settings->getProperty("TIMEOUT"));
  alignThreshold = std::stoi(settings->getProperty("ALIGN_THRESHOLD"));
  forwardSpeed = std::stoi(settings->getProperty("FORWARD_SPEED"));
  rect_angle_threshold = std::stoi(settings->getProperty("RECTANGLE_ANGLE_THRESHOLD"));
  int MIN_RECT_AREA = std::stoi(settings->getProperty("MIN_RECT_AREA"));


  //Capture a temporary image from the camera
  Mat imgTmp;
  //     cap.read(imgTmp);
  cout << "about to threshold image\n";
  /*cv::namedWindow("imgThresholded",CV_WINDOW_AUTOSIZE);
  cv::moveWindow("imgThresholded", 400, 500);
  cv::namedWindow("HSV",CV_WINDOW_AUTOSIZE);
  cv::moveWindow("HSV", 100, 200);*/

  rotate(0);

  bool angleThresholdMet = false;
  cout << "entering the while loop\n";
  bool centeredOnce = false;
  timer.start();
  while (!angleThresholdMet && timer.getTimeElapsed() < timeout){
    this_thread::yield();
    delete data;
    data = dynamic_cast<ImgData*> (dynamic_cast<CameraState*>(cameraModel->getState())->getDeepState("raw"));


    Mat imgOriginal;
    logger->trace("Got image from camera");
    imgOriginal = data->getImg();


    Mat imgClone = imgOriginal.clone();
    char buffer[80];
    strftime(buffer, 80, "%I:%M:%S", timer.getTimeStamp());
    cv::putText(imgClone, buffer, cv::Point(0,cv::getTextSize(buffer, cv::FONT_HERSHEY_PLAIN, 1, 2, 0).height), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255), 2);
    cv::imwrite(foldername+"/frame"+std::to_string(counter++)+".bmp", imgClone);

    //imshow("original", imgOriginal);
    cv::Size s = imgOriginal.size();
    imgWidth = s.width;
    imgHeight = s.height;
    float imgHeightF = static_cast<float>(imgHeight);
    float imgWidthF = static_cast<float>(imgWidth);


    Point2f rectCenter;
    vector<Point> largestRect;
    double largestArea = 0;
    bool rectFound = false;

    for (int i = 0; i < 3; i++) {
        Point2f center;
        vector<Point> rect;
        //filter image with each set of hsv settings
        //Mat imgThresholded;
        //HSVFilter hsvfilter(hsv_settings[i][0], hsv_settings[i][1], hsv_settings[i][2], hsv_settings[i][3], hsv_settings[i][4], hsv_settings[i][5]);
        //imgThresholded = hsvfilter.filter(imgOriginal);
        //imshow("thresh", imgThresholded);
        bool found = filterRect(imgOriginal, center, rect, hsv_settings[i]);
        if (found) {
            double area = fabs(contourArea(Mat(rect)));
            if (area > MIN_RECT_AREA) {
                if (!rectFound || area > largestArea) {
                    logger->debug("filter " + std::to_string(i) + " rect found location: x="+std::to_string(center.x)+" y="+std::to_string(center.y));
                    logger->debug("filter " + std::to_string(i) + " rectangle area is " + std::to_string(area));
                    rectCenter = center;
                    largestRect = rect;
                    largestArea = area;

                    rectFound = true;
                }
            } else {
                logger->debug("found rect but too small, area = " +std::to_string(area));
            }
        }
    }
    if (!rectFound) {
        setSpeed(forwardSpeed);
        continue;
    }

    /**
     * Are we close enough?
     *
     */
    bool closeEnough=false;
    cv::Point origin( imgWidthF/2, imgHeightF/2);
    if (lineLength(rectCenter, origin)<50) {
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
      accumulator++;
      accMassX += rectCenter.x;
      accMassY += rectCenter.y;
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
      ajusterAngle = rectAngle(largestRect);
      centeredOnce = true;
      accAngle += ajusterAngle;
      centeredAccumulator++;
      if (centeredAccumulator == 15){
        centeredAccumulator = 0;
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

    this_thread::yield();
  }
  logger->info("EXITING");
  return;
}
