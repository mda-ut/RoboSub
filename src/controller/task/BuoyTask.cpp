#include "BuoyTask.h"

#include <opencv2/video/tracking.hpp>


BuoyTask::BuoyTask(Model* camModel, TurnTask* tk, SpeedTask* st, DepthTask* dt)
{
    this->camModel = dynamic_cast<CameraModel*>(camModel);
    this->tk = tk;
    this->st = st;
    this->dt = dt;

    // Load properties file
    PropertyReader* propReader;
    propReader = new PropertyReader("settings/buoy_task_settings.txt");
    settings = propReader->load();
    travelDist = std::stoi(settings->getProperty("travelDist"));
    moveSpeed = std::stoi(settings->getProperty("moveSpeed"));
    deltaAngle = 0;
    closeRad = std::stoi(settings->getProperty("closeRad"));
    moveTime = std::stoi(settings->getProperty("MOVE_TIME"));
    sinkTime = std::stoi(settings->getProperty("sinkTime"));
    rotateTime = std::stoi(settings->getProperty("rotateTime"));
    retreatRotateTime = std::stoi(settings->getProperty("retreatRotateTime"));
    forwardBurstTime = std::stoi(settings->getProperty("forwardBurstTime"));
    stopBackSpeed= std::stoi(settings->getProperty("stopBackSpeed"));
    rotateSpeed= std::stoi(settings->getProperty("rotateSpeed"));
    sinkHeight = std::stoi(settings->getProperty("sinkHeight"));
    timeout = std::stod(settings->getProperty("timeout"));


    char buffer[80];
    strftime(buffer, 80, "PATH%I:%M:%S", timer.getTimeStamp());
    foldername = std::string(buffer);
    system( ("mkdir "+foldername).c_str() );
}
BuoyTask::~BuoyTask(){
    delete logger;
}

void BuoyTask::println(std::string s){
    logger->info(s);
//    std::cout<<s<<std::endl;
}

void BuoyTask::move(float d){
    if (moveWithSpeed){
        st->setTargetSpeed(d);
        st->execute();
    }else{
        float deltaT = 500;
        float maxSpeed = 50;
        float speed = d/(deltaT/1000);

        println("moving at speed: " + std::to_string(speed));
        st->setTargetSpeed(speed);
        st->execute();

        usleep(deltaT*1000);

        st->setTargetSpeed(0);
        st->execute();
        usleep(deltaT*1000);
    }
}

void BuoyTask::changeDepth(float h){
    logger->info("Changing depth " + std::to_string(h));
    dt->setDepthDelta(h);
    dt->execute();
//    usleep(5000000)
    sleep(sinkTime);
}

void BuoyTask::rotate(float angle){
    println("Rotating sub by " + std::to_string(angle) + " degrees");
    tk->setYawCurrentDelta(angle);
    tk->execute();
    deltaAngle += angle;
    println("Delta Angle " + std::to_string(deltaAngle));
//    usleep(1000000);
    sleep(rotateTime);
}
void BuoyTask::slide(float d){
    //distance is all in cm
    float hyp = 50; //50cm
    float theta;
    println(std::to_string(std::abs(d) > hyp));
    if (std::abs(d) > hyp){
        println("--- Starting slide2; d > theta ---");
        println("Rotating -90 degrees");
        rotate(-90);
        println("Moving " + std::to_string(d/3*2) + "cm");
        move(d/3*2);
        println("Rotating 90 degrees");
        rotate(90);
        println("--- Ending slide2 ---");
    }
    else{
        theta = asin(d/hyp) * 180 / M_PI;
        deltaAngle = theta;
        println("--- Starting slide ---");
        println("rotating " + std::to_string(theta) + " degrees");
        rotate(theta);
        println("Moving " + std::to_string(-hyp) + " cm");
        move(-hyp);
        println("rotating " + std::to_string(-theta) + " degrees");
        rotate(-theta);
        println("Moving " + std::to_string(hyp) + " cm");
        move(hyp);
        println("--- Ending slide ---");
    }
}

float BuoyTask::calcDistance(float rad){
    //distance(mm) = realHeight (mm) * imageHeight(px) / objectHeight(px) *
    //      focalLength(mm)/sensorHeight(mm)
    //focalLength/sensorHeight = constant (hopefully)
    float CONSY = 1.0400192063;
    float CONSX = 0.8524426745;

    return (imgHeight/rad * 23 * CONSY +
            imgWidth/rad * 23 * CONSX)/2;
}

#define RED "Red"
#define GRN "Green"

void BuoyTask::execute() {

    if (std::stoi(settings->getProperty("YOLO")) == 1){
        changeDepth(std::stoi(settings->getProperty("YOLODEPTH")));
        rotate(std::stoi(settings->getProperty("YOLOROT")));
        move(std::stoi(settings->getProperty("YOLOSPEED")));
        sleep(std::stoi(settings->getProperty("YOLOTIME")));
        move(0);
        return;
    }

    // >>>> Kalman Filter
   int stateSize = 6;
   int measSize = 4;
   int contrSize = 0;

   unsigned int type = CV_32F;
   cv::KalmanFilter kf(stateSize, measSize, contrSize, type);

   cv::Mat state(stateSize, 1, type);  // [x,y,v_x,v_y,w,h]
   //kf.statePost.create(stateSize, 1, type);
   cv::Mat meas(measSize, 1, type);    // [z_x,z_y,z_w,z_h]
   //cv::Mat procNoise(stateSize, 1, type)
   // [E_x,E_y,E_v_x,E_v_y,E_w,E_h]

   // Transition State Matrix A
   // Note: set dT at each processing step!
   // [ 1 0 dT 0  0 0 ]
   // [ 0 1 0  dT 0 0 ]
   // [ 0 0 1  0  0 0 ]
   // [ 0 0 0  1  0 0 ]
   // [ 0 0 0  0  1 0 ]
   // [ 0 0 0  0  0 1 ]
   cv::setIdentity(kf.transitionMatrix);

   // Measure Matrix H
   // [ 1 0 0 0 0 0 ]
   // [ 0 1 0 0 0 0 ]
   // [ 0 0 0 0 1 0 ]
   // [ 0 0 0 0 0 1 ]
   kf.measurementMatrix = cv::Mat::zeros(measSize, stateSize, type);
   kf.measurementMatrix.at<float>(0) = 1.0f;
   kf.measurementMatrix.at<float>(7) = 1.0f;
   kf.measurementMatrix.at<float>(16) = 1.0f;
   kf.measurementMatrix.at<float>(23) = 1.0f;

   // Process Noise Covariance Matrix Q
   // [ Ex 0  0    0 0    0 ]
   // [ 0  Ey 0    0 0    0 ]
   // [ 0  0  Ev_x 0 0    0 ]
   // [ 0  0  0    1 Ev_y 0 ]
   // [ 0  0  0    0 1    Ew ]
   // [ 0  0  0    0 0    Eh ]
   //cv::setIdentity(kf.processNoiseCov, cv::Scalar(1e-2));
   kf.processNoiseCov.at<float>(0) = 1e-2;
   kf.processNoiseCov.at<float>(7) = 1e-2;
   kf.processNoiseCov.at<float>(14) = 2.0f;
   kf.processNoiseCov.at<float>(21) = 1.0f;
   kf.processNoiseCov.at<float>(28) = 1e-2;
   kf.processNoiseCov.at<float>(35) = 1e-2;

   // Measures Noise Covariance Matrix R
   cv::setIdentity(kf.measurementNoiseCov, cv::Scalar(1e-1));
   // <<<< Kalman Filter

   char ch = 0;
   double ticks = 0;
   bool found = false;
   int notFoundCount = 0;

    int greenHSV[6];
    for (int i = 0; i < 6; i++){
        greenHSV[i] = std::stoi(settings->getProperty("g"+std::to_string(i+1)));
    }

    green = HSVFilter(greenHSV[0], greenHSV[1], greenHSV[2], greenHSV[3], greenHSV[4], greenHSV[5]);

    int redHSV[6];
    int redHSV2[6];
    for (int i = 0; i < 6; i++){
        redHSV[i] = std::stoi(settings->getProperty("r"+std::to_string(i+1)));
        redHSV2[i] = std::stoi(settings->getProperty("rr"+std::to_string(i+1)));
    }

    reds.push_back(HSVFilter(redHSV[0],     redHSV[1],  redHSV[2],  redHSV[3],  redHSV[4],  redHSV[5]));
    HSVFilter temp = HSVFilter(redHSV2[0],    redHSV2[1], redHSV2[2], redHSV2[3], redHSV2[4], redHSV2[5]);
    reds.push_back(temp);
    //only look for 1 circle
    ShapeFilter sf = ShapeFilter(3, 1);

    //assuming the sub is in the correct position
    //first look for red, and then hit it
    //then look and hit green

    bool hitGreen = false;
    bool hitRed = false;
    bool alligned = false;

    int retreat = 0;
    int moveDist = std::stoi(settings->getProperty("moveDist"));
    int rotateAng = std::stoi(settings->getProperty("rotateAng"));
    int movementDelay = std::stoi(settings->getProperty("movementDelay"));
    int deltaDist = 0;
    float deltaT = 0;

    bool done = false;
    const int SIZEX = 320, SIZEY = 240;
    cv::namedWindow(RED, CV_WINDOW_NORMAL);
    cv::resizeWindow(RED, SIZEX, SIZEY+200);
    cv::moveWindow(RED, 1500, 400);
//    cv::namedWindow(GRN, CV_WINDOW_NORMAL);
//    cv::resizeWindow(GRN, SIZEX, SIZEY+200);
//    cv::moveWindow(GRN, 1500, 100);
    cv::namedWindow("Center", CV_WINDOW_NORMAL);
    cv::resizeWindow("Center", SIZEX, SIZEY);
    cv::moveWindow("Center", 1100, 100);
    cv::namedWindow("circles", CV_WINDOW_NORMAL);
    cv::resizeWindow("circles", SIZEX, SIZEY);
    cv::moveWindow("circles", 1100, 400);

    cvCreateTrackbar("LH", RED, &redHSV[0], 179);
    cvCreateTrackbar("HH", RED, &redHSV[1], 179);
    cvCreateTrackbar("LS", RED, &redHSV[2], 255);
    cvCreateTrackbar("HS", RED, &redHSV[3], 255);
    cvCreateTrackbar("LV", RED, &redHSV[4], 255);
    cvCreateTrackbar("HV", RED, &redHSV[5], 255);

//    cvCreateTrackbar("LH", GRN, &greenHSV[0], 179);
//    cvCreateTrackbar("HH", GRN, &greenHSV[1], 179);
//    cvCreateTrackbar("LS", GRN, &greenHSV[2], 255);
//    cvCreateTrackbar("HS", GRN, &greenHSV[3], 255);
//    cvCreateTrackbar("LV", GRN, &greenHSV[4], 255);
//    cvCreateTrackbar("HV", GRN, &greenHSV[5], 255);

    timer.start();
    while (!done && timer.getTimeElapsed() < timeout) {

        double precTick = ticks;
        ticks = (double) cv::getTickCount();

        double dT = (ticks - precTick) / cv::getTickFrequency(); //seconds

        // Frame acquisition
        std::string s = "raw";
        ImgData* data = dynamic_cast<ImgData*>
                (dynamic_cast<CameraState*>
                 (camModel->getState())->getDeepState(s));
        imgWidth = data->getImg().size().width;
        imgHeight = data->getImg().size().height;
        cv::Mat frame = data->getImg();
        cv::Mat hsvFiltered;
//        cv::imshow("hsv",data->getImg());

//        char buffer[80];
//        strftime(buffer, 80, "%I:%M:%S", timer.getTimeStamp());
//        cv::putText(frame, buffer, cv::Point(0,cv::getTextSize(buffer, cv::FONT_HERSHEY_PLAIN, 1, 2, 0).height), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255), 2);
        cv::imwrite(foldername+"/rawframe"+std::to_string(counter++)+".bmp", frame);


        //filter for a color depending if the other color is hit or not
        if (!hitRed){
//            printf("Doing red [%d] [%d] [%d] [%d] [%d] [%d]\n", redHSV[0], redHSV[1], redHSV[2], redHSV[3], redHSV[4], redHSV[5]);
            hsvFiltered = filterRed(frame);
            cv::imshow(RED, hsvFiltered);
        } else if (!hitGreen){
            //not gonna do green
            done = true;
            println("Done task");
            continue;

//            println("Filtering green");
            green.setValues(greenHSV[0], greenHSV[1], greenHSV[2], greenHSV[3], greenHSV[4], greenHSV[5]);
            hsvFiltered = green.filter(frame);
//            cv::imshow(GRN, hsvFiltered);
        } else if (hitGreen && hitRed) {
            done = true;
            println("Done task");
            continue;
        }

        if (alligned){
//            continue;//REMOVE
            move(moveSpeed);
            sleep(moveTime);
            if (!hitRed){
                hitRed = true;
                println("Hit red");
                retreat = true;
                alligned = false;
            } else {
                hitGreen = true;
                println("Hit green");
                retreat = true;
            }
            move(0);
        }else if (retreat){
//            continue;//REMOVE
            if (moveWithSpeed){
                println("Retreating");
                move(-moveSpeed);
                sleep(moveTime);
                //usleep(deltaT * 500);
                move(0);
                rotate(-deltaAngle);
                sleep(retreatRotateTime);
            } else {
                println("Retreating " + std::to_string(-deltaDist - 20) + "cm");
                move(-deltaDist - 20);      //move 20cm more than i needed

                //sleep(movementDelay);
                //move the sub back to the middle
                if (deltaAngle != -1){
                    println("Retreat rotating " + std::to_string(deltaAngle) + " degrees");
                    rotate(deltaAngle);
                    println("Retreat to middle by " + std::to_string(deltaDist-20) + "cm");
                    move (deltaDist-20);   //move 20cm less than i need
                    rotate(-deltaAngle);
                }
            }
            retreat = false;
        }

        //after hitting a color, move the sub back to look for the other one
        //TODO: CALIBRATE THIS STEP

        else if ( (sf.findCirc(hsvFiltered) && sf.getRad()[0] > closeRad)/* || (sf.findCirc(red2.filter(frame)) && sf.getRad()[0] > closeRad)*/ ){
            retreat = false;
            cv::Point2f cent = sf.getCenter()[0];

            // Get prediction from Kalman Filter and use as result
            // >>>> Matrix A
            kf.transitionMatrix.at<float>(2) = dT;
            kf.transitionMatrix.at<float>(9) = dT;
            // <<<< Matrix A

            std::cout << "dT:" << std::endl << dT << std::endl;

            state = kf.predict();
            std::cout << "State post:" << std::endl << state << std::endl;
            cv::Rect predRect;
            predRect.width = state.at<float>(4);
            predRect.height = state.at<float>(5);
            predRect.x = state.at<float>(0) - predRect.width / 2;
            predRect.y = state.at<float>(1) - predRect.height / 2;
            cent.x = state.at<float>(0);
            cent.y = state.at<float>(1);
            cv::circle(frame, cent, 2, CV_RGB(254,255,0), -1);
            cv::rectangle(frame, predRect, CV_RGB(254,255,0), 2);

            // Update Kalman Filter
            notFoundCount = 0;

//            meas.at<float>(0) = ballsBox[0].x + ballsBox[0].width / 2;
//            meas.at<float>(1) = ballsBox[0].y + ballsBox[0].height / 2;
//            meas.at<float>(2) = (float)ballsBox[0].width;
//            meas.at<float>(3) = (float)ballsBox[0].height;

            meas.at<float>(0) = sf.getCenter()[0].x;
            meas.at<float>(1) = sf.getCenter()[0].y;
            //TODO: Get actual contour bounding box from shape filter

            std::vector<std::vector<cv::Point> > balls;
            std::vector<cv::Rect> ballsBox;
            std::vector<std::vector<cv::Point> > contours = sf.getContours();

            for (size_t i = 0; i < contours.size(); i++)       {
                cv::Rect bBox;
                bBox = cv::boundingRect(contours[i]);
                float ratio = (float) bBox.width / (float) bBox.height;
                if (ratio > 1.0f)
                  ratio = 1.0f / ratio;

               // Searching for a bBox almost square
               if (ratio > 0.75 && bBox.area() >= 400)
               {
                  balls.push_back(contours[i]);
                  ballsBox.push_back(bBox);
               }
            }

            if (ballsBox.size() > 0) {
                meas.at<float>(2) = (float)ballsBox[0].width;
                meas.at<float>(3) = (float)ballsBox[0].height;
            } else {
                meas.at<float>(2) = (float)0;
                meas.at<float>(3) = (float)0;
            }

            if (!found) // First detection!
            {
               // >>>> Initialization
               kf.errorCovPre.at<float>(0) = 1; // px
               kf.errorCovPre.at<float>(7) = 1; // px
               kf.errorCovPre.at<float>(14) = 1;
               kf.errorCovPre.at<float>(21) = 1;
               kf.errorCovPre.at<float>(28) = 1; // px
               kf.errorCovPre.at<float>(35) = 1; // px

               state.at<float>(0) = meas.at<float>(0);
               state.at<float>(1) = meas.at<float>(1);
               state.at<float>(2) = 0;
               state.at<float>(3) = 0;
               state.at<float>(4) = meas.at<float>(2);
               state.at<float>(5) = meas.at<float>(3);
               // <<<< Initialization

               found = true;
            }
            else
               kf.correct(meas); // Kalman Correction

//            std::cout << "Measure matrix:" << std::endl << meas << std::endl;

            cv::circle(frame, cent, 10, cv::Scalar(255,0,0));
//            cv::imshow("Center", frame);continue;
            if (std::abs(cent.x - imgWidth/2) < imgWidth/100*3){
                //in the middle 20% of the screen horizontally
                if (std::abs(cent.y - imgHeight / 2) < imgHeight / 100 * 3) {
                    //in the middle 20% vertically
                    float d = calcDistance(sf.getRad()[0]) * 1.2;
                    float t = d/moveSpeed;
                    alligned = true;
//                    logger->info("Stopping");
//                    move(0);
                    /*
                    //if the radius of the circle is huge, so the sub will hit it
                    if (sf.getRad()[0] > imgWidth*imgHeight/3){
                        if (!hitGreen){
                            hitGreen = true;
                            println("Hit green");
                        }else{
                            hitRed = true;
                            println("Hit red");
                        }

                        //return the sub back to its orginal position
                        move(0);
                        retreat = true;
                        println("Retreat enabled");
                    }else{
                        //move straight and hit it
                        float dist = calcDistance(sf.getRad()[0]);
                        deltaDist = dist*1.2;
                        println("Moving forward " + std::to_string(deltaDist) + "cm to hit buoy");
                        move(deltaDist);
                    }*/
                } else {
                    float deltaY;
                    if (cent.y > imgHeight/2) {
                        deltaY = -sinkHeight; //rise a bit
                    } else {
                        deltaY = sinkHeight; //sink a bit
                    }
                    //float deltaY = (cent.y - imgHeight/2)/sf.getRad()[0];
                    println("Rising " + std::to_string(deltaY*100)+"cm");
                    changeDepth(deltaY);
                }
            } else {
                println("Center: " + std::to_string(cent.x) + " " + std::to_string(imgWidth/2));
                //float ang = atan2(cent.x-imgWidth/2, 0) * 180 / M_PI;
                //float dX = cent.x-imgWidth/2;
                //dX * 23/sf.getRad()[0];
                //float ang = atan(dX/calcDistance(sf.getRad()[0])) * 180 / M_PI;
                //println("Rotating " + std::to_string(ang) + " degrees");
                //rotate(ang);
                float dir = cent.x-imgWidth/2;
                println("Rotating 2 degrees " + std::to_string(dir));
                dir /= std::abs(dir);
                rotate (2*dir);
                /*
                float scale = 23/sf.getRad()[0];
                float dist = sf.getCenter()[0].x - imgWidth/2;

                //slide 2/3 of the way
                float deltaX = dist*scale/3*2;
                println("Sliding " + std::to_string(deltaX)+"cm");
                slide(deltaX);*/
            }
        } else {
//            continue;//REMOVE
            ///CIRCLES NOT FOUND

            notFoundCount++;
            std::cout << "notFoundCount:" << notFoundCount << std::endl;
            if( notFoundCount >= 10 )
            {
               found = false;
            }
            else {

                kf.statePost = state;

            }

            ///ROTATE/MOVE SUB
            //rotate(rotateAng);
            if (sf.getRad().size() > 0) printf("Size [%f]\n", sf.getRad()[0]);
            println("Circle not found or too small, moving forward");
            //move(moveDist);

                ///tries to look for any colors and move towards it
                logger->info("Looking for coloured masses to turn towards");
                std::vector<cv::Point2f> mc = sf.findMassCenter(hsvFiltered);
                if (mc.size() > 0) {
                    logger->info("Found coloured mass");
                    printf("mass x[%f] y[%f]\n", mc[0].x, mc[0].y);
                    float dir = mc[0].x - imgWidth/2;
                    dir /= std::abs(dir);
                    rotate(rotateSpeed * dir);
                } else {
                    ///if it dosnt see any colors, move forwards
                    logger->debug("No coloured masses found.  Moving forward for 1s");
                    move(moveSpeed);
                    sleep(forwardBurstTime);
                    move(stopBackSpeed);
                    sleep(forwardBurstTime/2);
                    move(0);
                }
        }

        char buffer[80];
        strftime(buffer, 80, "%I:%M:%S", timer.getTimeStamp());
        cv::putText(frame, buffer, cv::Point(0,cv::getTextSize(buffer, cv::FONT_HERSHEY_PLAIN, 1, 2, 0).height), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255,255,255), 2);

        cv::imwrite(foldername+"/frame"+std::to_string(counter)+".bmp", frame);

        cv::imshow("Center",frame);
        cv::waitKey(1);

        delete data;
        data = 0;
//        usleep(33000);
    }

    println("Rotating final " + std::to_string(-deltaAngle));

    rotate(-deltaAngle);
    sleep(retreatRotateTime);
    move(moveSpeed);
    sleep(forwardBurstTime);
    move(stopBackSpeed);
    sleep(forwardBurstTime/2);
    move(0);
}


std::string findColors(cv::Vec3b color){
    //0 = b; 1 = g; 2 = r
    int sum = color[0] + color[1] + color[2];
    if (color[2] > sum/3)   return "Red";
    else                    return "Nothing";
//    int bg = color[0] - color[1];
//    int br = color[0] - color[2];
//    int gb = color[1] - color[0];
//    int gr = color[1] - color[2];
//    int rb = color[2] - color[0];
//    int rg = color[2] - color[1];
//    if (bg > 10 && br > 10)         return "Blue";
//    else if (gb > 10 && gr > 10)    return "Green";
//    else if (rb > 10 && rg > 10)    return "Red";
//    else                            return "idk, yellow?";
}

cv::Mat BuoyTask::filterRed(cv::Mat frame){
    return reds[0].filter(frame);
    std::vector<cv::Mat> filtered;
    std::vector<float> sizes;

    ShapeFilter tmpSf(3,1);

    for (int i = 0; i < reds.size(); i++){
        filtered.push_back(reds[i].filter(frame));
        if (tmpSf.findCirc(filtered[i])){
            bool added = false;
            for (int n = 0; n < tmpSf.getRad().size(); n++){
                if (findColors(tmpSf.colors[n]) == "Red"){
//                    printf("COLOR: %d %d %d\n", tmpSf.colors[n][0], tmpSf.colors[n][1], tmpSf.colors[n][2]);
                    sizes.push_back(tmpSf.getRad()[n]);
                    added = true;
                }
            }
            if (!added) sizes.push_back(0);
        }else{
            sizes.push_back(0);
        }
    }

    int biggestRadIndex = 0;
    for (int i = 0; i < sizes.size(); i++){
        if (sizes[i] > sizes[biggestRadIndex]){
            biggestRadIndex = i;
        }
    }


    return filtered[biggestRadIndex];
}
