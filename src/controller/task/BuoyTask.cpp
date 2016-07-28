#include "BuoyTask.h"


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
    sinkHeight= std::stoi(settings->getProperty("sinkHeight"));
}
BuoyTask::~BuoyTask(){
    delete logger;
}

void BuoyTask::println(std::string s){
//    logger->debug(s);
    std::cout<<s<<std::endl;
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
    printf("Changing depth [%f]\n", h);
    dt->setDepthDelta(h);
    dt->execute();
//    usleep(5000000);
    sleep(sinkTime);
}

void BuoyTask::rotate(float angle){
    println("Rotating sub by " + std::to_string(angle) + " degrees");
    tk->setYawCurrentDelta(angle);
    tk->execute();
    deltaAngle += angle;
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

void BuoyTask::execute() {

    int greenHSV[6];
    for (int i = 0; i < 6; i++){
        greenHSV[i] = std::stoi(settings->getProperty("g"+std::to_string(i+1)));
    }

    green = HSVFilter(greenHSV[0], greenHSV[1], greenHSV[2], greenHSV[3], greenHSV[4], greenHSV[5]);

    int redHSV[6];
    for (int i = 0; i < 6; i++){
        redHSV[i] = std::stoi(settings->getProperty("r"+std::to_string(i+1)));
    }

    red = HSVFilter(redHSV[0], redHSV[1], redHSV[2], redHSV[3], redHSV[4], redHSV[5]);
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
    cv::namedWindow("HSV", CV_WINDOW_AUTOSIZE);
    cv::moveWindow("HSV", 1500, 400);
    cv::namedWindow("HSV2", CV_WINDOW_AUTOSIZE);
    cv::moveWindow("HSV2", 1500, 100);
    cv::namedWindow("Center", CV_WINDOW_AUTOSIZE);
    cv::moveWindow("Center", 1100, 100);
    cv::namedWindow("circles", CV_WINDOW_AUTOSIZE);
    cv::moveWindow("circles", 1100, 400);

    cvCreateTrackbar("LH", "HSV", &redHSV[0], 179);
    cvCreateTrackbar("HH", "HSV", &redHSV[1], 179);
    cvCreateTrackbar("LS", "HSV", &redHSV[2], 255);
    cvCreateTrackbar("HS", "HSV", &redHSV[3], 255);
    cvCreateTrackbar("LV", "HSV", &redHSV[4], 255);
    cvCreateTrackbar("HV", "HSV", &redHSV[5], 255);

    cvCreateTrackbar("LH", "HSV2", &greenHSV[0], 179);
    cvCreateTrackbar("HH", "HSV2", &greenHSV[1], 179);
    cvCreateTrackbar("LS", "HSV2", &greenHSV[2], 255);
    cvCreateTrackbar("HS", "HSV2", &greenHSV[3], 255);
    cvCreateTrackbar("LV", "HSV2", &greenHSV[4], 255);
    cvCreateTrackbar("HV", "HSV2", &greenHSV[5], 255);

    while (!done) {
        std::string s = "raw";
        ImgData* data = dynamic_cast<ImgData*>
                (dynamic_cast<CameraState*>
                 (camModel->getState())->getDeepState(s));
        imgWidth = data->getImg().size().width;
        imgHeight = data->getImg().size().height;
        cv::Mat frame = data->getImg();
        cv::Mat hsvFiltered;
//        cv::imshow("hsv",data->getImg());

        //filter for a color depending if the other color is hit or not
        if (!hitRed){
//            printf("Doing red [%d] [%d] [%d] [%d] [%d] [%d]\n", redHSV[0], redHSV[1], redHSV[2], redHSV[3], redHSV[4], redHSV[5]);
//            hsvFiltered = filterRed(frame);
            red.setValues(redHSV[0], redHSV[1], redHSV[2], redHSV[3], redHSV[4], redHSV[5]);
            hsvFiltered = red.filter(frame);
            cv::imshow("HSV", hsvFiltered);
        } else if (!hitGreen){
//            println("Filtering green");
            green.setValues(greenHSV[0], greenHSV[1], greenHSV[2], greenHSV[3], greenHSV[4], greenHSV[5]);
            hsvFiltered = green.filter(frame);
            cv::imshow("HSV2", hsvFiltered);
        } else if (hitGreen && hitRed) {
            done = true;
            println("Done task");
            continue;
        }

        if (alligned){
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

        else if (sf.findCirc(hsvFiltered) && sf.getRad()[0] > closeRad){
            retreat = false;
            cv::Point2f cent = sf.getCenter()[0];
            cv::circle(frame, cent, 10, cv::Scalar(255,0,0));
            cv::imshow("Center",frame);
            cv::waitKey(1);
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
            ///CIRCLES NOT FOUND
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
        delete data;
        data = 0;
//        usleep(33000);
    }
}

cv::Mat BuoyTask::filterRed(cv::Mat frame){
    cv::Mat hsvFiltered = red.filter(frame);
    cv::imshow("HSV", hsvFiltered);
}
