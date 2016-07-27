#include "BuoyTask.h"


BuoyTask::BuoyTask(Model* camModel, TurnTask* tk, SpeedTask* st, DepthTask* dt)
{
    this->camModel = dynamic_cast<CameraModel*>(camModel);
    this->tk = tk;
    this->st = st;
    this->dt = dt;

    // Load properties file
    PropertyReader* propReader;
    propReader = new PropertyReader("../src/settings/buoy_task_settings.txt");
    settings = propReader->load();
    travelDist = std::stoi(settings->getProperty("travelDist"));
    moveSpeed = std::stoi(settings->getProperty("moveSpeed"));
    deltaAngle = 0;
    closeRad = std::stoi(settings->getProperty("closeRad"));

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
    }
}

void BuoyTask::changeDepth(float h){
    printf("Changing depth [%f]\n", h);
    dt->setDepthDelta(h);
    dt->execute();
//    usleep(5000000);
    sleep(1);
}

void BuoyTask::rotate(float angle){
    println("Rotating sub by " + std::to_string(angle) + " degrees");
    tk->setYawDelta(angle);
    tk->execute();
    deltaAngle += angle;
//    usleep(1000000);
    sleep(1);
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

    green = HSVFilter(std::stoi(settings->getProperty("g1")),
                    std::stoi(settings->getProperty("g2")),
                    std::stoi(settings->getProperty("g3")),
                    std::stoi(settings->getProperty("g4")),
                    std::stoi(settings->getProperty("g5")),
                    std::stoi(settings->getProperty("g6")));

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

    int retreat = 0;
    int moveDist = std::stoi(settings->getProperty("moveDist"));
    int rotateAng = std::stoi(settings->getProperty("rotateAng"));
    int movementDelay = std::stoi(settings->getProperty("movementDelay"));
    int deltaDist = 0;
    float deltaT = 0;

    bool done = false;
    cv::namedWindow("HSV", CV_WINDOW_AUTOSIZE);
    cv::moveWindow("HSV", 1500, 400);
    cv::namedWindow("Center", CV_WINDOW_AUTOSIZE);
    cv::moveWindow("Center", 1500, 100);
    cv::namedWindow("circles", CV_WINDOW_AUTOSIZE);
    cv::moveWindow("circles", 1100, 400);

    cvCreateTrackbar("LH", "HSV", &redHSV[0], 179);
    cvCreateTrackbar("HH", "HSV", &redHSV[1], 179);
    cvCreateTrackbar("LS", "HSV", &redHSV[2], 255);
    cvCreateTrackbar("HS", "HSV", &redHSV[3], 255);
    cvCreateTrackbar("LV", "HSV", &redHSV[4], 255);
    cvCreateTrackbar("HV", "HSV", &redHSV[5], 255);

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
            println("Doing red");
//            hsvFiltered = filterRed(frame);
            red.setValues(redHSV[0], redHSV[1], redHSV[2], redHSV[3], redHSV[4], redHSV[5]);
            hsvFiltered = red.filter(frame);
            cv::imshow("HSV", hsvFiltered);
        } else if (!hitGreen){
            //println("Filtering green");
            //green.filter(data);
            done = true;
            continue;
        } else if (hitGreen && hitRed) {
            done = true;
            println("Done task");
            continue;
        }

        //after hitting a color, move the sub back to look for the other one
        //TODO: CALIBRATE THIS STEP

        if (sf.findCirc(hsvFiltered) && sf.getRad()[0] > closeRad){
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
                    println("Moving " + std::to_string(d) + "cm in " + std::to_string(t) + "s");
                    move(moveSpeed);
                    sleep(std::stoi(settings->getProperty("MOVE_TIME")));
                    if (!hitRed){
                        hitRed = true;
                        println("Hit red");
                        retreat = true;
                    } else {
                        hitGreen = true;
                        println("Hit green");
                        retreat = true;
                    }
                    logger->info("Stopping");
                    move(0);
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
                        deltaY = -2; //rise a bit
                    } else {
                        deltaY = 2; //sink a bit
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
                rotate (2);
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
            if (retreat) {
                if (moveWithSpeed){
                    println("Retreating");
                    move(-moveSpeed);
                    sleep(3);
                    //usleep(deltaT * 500);
                    move(0);
                    rotate(-deltaAngle);
                    sleep(3);
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
//                continue;
            } else {
                ///tries to look for any colors and move towards it
                logger->info("Looking for coloured masses to turn towards");
                std::vector<cv::Point2f> mc = sf.findMassCenter(data);
                if (mc.size() > 0 && false) {
                    logger->info("Found coloured mass");
                    float dir = mc[0].x - imgWidth/2;
                    dir /= std::abs(dir);
                    rotate(5 * dir);
                } else {
                    ///if it dosnt see any colors, move forwards
                    logger->debug("No coloured masses found.  Moving forward for 1s");
                    move(moveSpeed);
                    sleep(1);
                    move(-1);
                    sleep(1);
                    move(0);
                }
            }
        }
        delete data;
        data = 0;
        usleep(33000);
    }
}

cv::Mat BuoyTask::filterRed(cv::Mat frame){
    cv::Mat hsvFiltered = red.filter(frame);
    cv::imshow("HSV", hsvFiltered);
}
