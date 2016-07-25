#include "BuoyTask.h"

using namespace cv;
using namespace std;

BuoyTask::BuoyTask(Model *camModel, TurnTask *tk, SpeedTask *st, DepthTask *dt){
    this->camModel = dynamic_cast<CameraModel*>(camModel);
    this->tk = tk;
    this->st = st;
    this->dt = dt;// Load properties file
    propReader = new PropertyReader("settings/buoy_task_settings.txt");
    settings = propReader->load();
    erosion_size = std::stoi(settings->getProperty("erosion_size"));
    dilation_size = std::stoi(settings->getProperty("dilation_size"));
    thresh = std::stoi(settings->getProperty("thresh"));
    max_thresh = std::stoi(settings->getProperty("max_thresh"));
    alignRadius = std::stoi(settings->getProperty("alignRadius"));
    lowRedH = std::stoi(settings->getProperty("lowRedH"));
    lowRedS = std::stoi(settings->getProperty("lowRedS"));
    lowRedV = std::stoi(settings->getProperty("lowRedV"));
    highRedH = std::stoi(settings->getProperty("highRedH"));
    highRedS = std::stoi(settings->getProperty("highRedS"));
    highRedV = std::stoi(settings->getProperty("highRedV"));
    lowRedH2 = std::stoi(settings->getProperty("lowRedH2"));
    lowRedS2 = std::stoi(settings->getProperty("lowRedS2"));
    lowRedV2 = std::stoi(settings->getProperty("lowRedV2"));
    highRedH2 = std::stoi(settings->getProperty("highRedH2"));
    highRedS2 = std::stoi(settings->getProperty("highRedS2"));
    highRedV2 = std::stoi(settings->getProperty("highRedV2"));
    lowGreenH = std::stoi(settings->getProperty("lowGreenH"));
    lowGreenS = std::stoi(settings->getProperty("lowGreenS"));
    lowGreenV = std::stoi(settings->getProperty("lowGreenV"));
    highGreenH = std::stoi(settings->getProperty("highGreenH"));
    highGreenS = std::stoi(settings->getProperty("highGreenS"));
    highGreenV = std::stoi(settings->getProperty("highGreenV"));
    lowGreenH2 = std::stoi(settings->getProperty("lowGreenH2"));
    lowGreenS2 = std::stoi(settings->getProperty("lowGreenS2"));
    lowGreenV2 = std::stoi(settings->getProperty("lowGreenV2"));
    highGreenH2 = std::stoi(settings->getProperty("highGreenH2"));
    highGreenS2 = std::stoi(settings->getProperty("highGreenS2"));
    highGreenV2 = std::stoi(settings->getProperty("highGreenV2"));

}

BuoyTask ::~BuoyTask(){
    delete logger;
}

void BuoyTask::execute() {
    /**
     * The colour of the buoy the sub is currently moving towards.
     * The buoy is not in the state of transitioning between buoys and the task is not done.
     */
    std::string stage = "RED";
    bool transition = false;
    bool done = false;

    // Initialize pointers for the dimensions and coordinates of the buoy and the angle of the path the sub is on
    float * radius = new float();//NULL; ///Pointer for Circle Radius
    float * centerX = new float();//NULL; ///Pointer for X Coor of Circle Center
    float * centerY = new float();//NULL; ///Pointer for Y Coor of Circle Center
    int delta = 0; ///Change in Movement Needed
    int deltaNext = 0; ///Moving to next buoy
    int deltaTurn = 0; ///Turning to next buoy
    int deltaGreen = 0; ///Moving to the green buoy
    int deltaYellow = 0; ///Moving to the yellow buoy

    // Set up data the sub is receiving
    std::string s = "raw";
    ImgData *data;
    int imgWidth, imgHeight;

    cout << "getting state " << camModel->getState() << endl;

    st->setTargetSpeed(30);
    st->execute();
    sleep(1);

    st->setTargetSpeed(0);
    st->execute();
    sleep(1);
    // Detect buoys while the sub is not done the task
    while (!done){
        data = dynamic_cast<ImgData*>(dynamic_cast<CameraState*>(camModel->getState())->getDeepState(s));
        imgWidth = data->getImg().size().width;
        imgHeight = data->getImg().size().height;
        int closeRadius = round(imgWidth/3.5);
        int thresholdRadius = round(imgWidth/1.5);

        // Find the buoy parameters
        logger->info("detecting buoys");
        detectBuoy(data, stage, centerX, centerY, radius);

        // Do nothing if no buoys have been found
        if (*radius == 0) {
            if (stage == "GREEN") {
                tk->setYawDelta(25);
                tk->execute();
                sleep(3);
            }
            else {
                logger->info("none found");
                tk->setYawDelta(45);
                tk->execute();
                sleep(5);
                data = dynamic_cast<ImgData*>(dynamic_cast<CameraState*>(camModel->getState())->getDeepState(s));
                detectBuoy(data, stage, centerX, centerY, radius);
                if (*radius == 0) {
                    logger->info("none found");
                    tk->setYawDelta(-90);
                    tk->execute();
                    sleep(5);
                    data = dynamic_cast<ImgData*>(dynamic_cast<CameraState*>(camModel->getState())->getDeepState(s));
                    detectBuoy(data, stage, centerX, centerY, radius);
                    if (*radius == 0) {
                        tk->setYawDelta(45);
                        tk->execute();
                        sleep(5);
                        st->setTargetSpeed(20);
                        st->execute();
                        sleep(5);
                        st->setTargetSpeed(0);
                        st->execute();
                        sleep(1);
                    }
                    else {
                        st->setTargetSpeed(20);
                        sleep(1);
                        st->setTargetSpeed(0);
                        st->execute();
                        sleep(1);
                    }
                }
                else {
                    st->setTargetSpeed(20);
                    st->execute();
                    sleep(1);
                    st->setTargetSpeed(0);
                    st->execute();
                    sleep(1);
                }
            }
        }
        // A buoy has been detected in the distance
        else if ((0 < *radius) && (*radius < thresholdRadius) && (!transition))
        {
            logger->info("found");
            logger->info(std::to_string(imgWidth));
            logger->info(std::to_string(*centerX) + "");

            if (*radius < thresholdRadius && !transition) {
//                st->setTargetSpeed(0);
//                st->execute();
                // Move towards the buoy until contact is made
                if (!(std::abs(*centerX - imgWidth/2) < imgWidth/25)){
                    // If the buoy is not within the middle 10% of the camera image
                    if (*centerX > imgWidth/2) {
                        delta = 8;
                    } else if (*centerX < imgWidth/2) {
                        delta = -8;
                    }
                    // Align the sub to the x-coordinate of the detected buoy
                    logger->info("aligning");
                    logger->info("Rotating sub by " + std::to_string(delta) + " degrees.");
                    tk->setYawDelta(delta);
                    tk->execute();
                    sleep(5);
                    continue;
                }

                if (!(std::abs(*centerY - imgHeight/2) < imgHeight/25)){
                    // If the buoy is not within the middle 10% of the camera image
                    // usleep(1000000);
                    if (*centerY > imgHeight/2) {
                        delta = 5;
                        if (*radius > closeRadius) {
                            delta = 2;
                        }
                    } else if (*centerY < imgHeight/2) {
                        delta = -5;
                    }
                    // Align the sub to the y-coordinate of the detected buoy
                    logger->info("aligning");
                    logger->info("Changing depth by " + std::to_string(delta) + " units.");
                    dt->setDepthDelta(delta);
                    dt->execute();
                    sleep(10);
                    continue;
                }
                st->setTargetSpeed(20);
                st->execute();
                sleep(2);
                st->setTargetSpeed(0);
                st->execute();
                sleep(2);
                data = dynamic_cast<ImgData*>(dynamic_cast<CameraState*>(camModel->getState())->getDeepState(s));
                detectBuoy(data, stage, centerX, centerY, radius);
            }

        // Transition to the next buoy if the sub is sufficiently close to the current buoy
        } else if (*radius >= thresholdRadius && !transition){
            st->setTargetSpeed(10);
            st->execute();
            sleep(2);
            st->setTargetSpeed(-5);
            st->execute();
            sleep(2);
            st->setTargetSpeed(0);
            st->execute();
            logger->info("The" + stage + "buoy has been hit");
            if (stage == "RED"){
                stage = "GREEN";
                transition = true;
                sleep(10);
            }
            // The sub has hit the last buoy
            else{
                logger->info("The buoy task has been completed");
                done = true;
            }
            // Move away from the buoy
            // st->setTargetSpeed(-50);
            // st->execute();
        }

        // Move on to next buoy
        if (transition) {

            delta = deltaNext;
            logger->debug("Changing speed by " + std::to_string(delta));
            st->setTargetSpeed(delta);//delta
            st->execute();
            usleep(2000000);
            logger->debug("Rotating sub by " + std::to_string(deltaTurn) + "to look for next buoy");
            tk->setYawDelta(deltaTurn);
            tk->execute();
            sleep(5);

            // Orient the sub to the next buoy
            if (stage == "GREEN") {
                delta = deltaGreen;
                logger->debug("Orienting to GREEN");
                tk->setYawDelta(delta);
                tk->execute();
                sleep(5);
            }
        }
    }
}

void BuoyTask::thresh_callback(int, Mat all_colour_image, float *centerX, float *centerY, float *radius )
{
    RNG rng(12345);
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // Detect edges and contours
    Canny( all_colour_image, canny_output, thresh, thresh*2, 3 );
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    vector<Moments> mu(contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    { mu[i] = moments( contours[i], false );
    }

    // Get the mass centers:
    vector<Point2f> mc( contours.size() );
    for( int i = 0; i < mu.size(); i++ )
    {
        mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );
    }

    // Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
        circle( drawing, mc[i], 4, color, -1, 8, 0 );
    }

    // Calculate the area with the moments 00 and compare with the result of the OpenCV function

    // printf("\t Info: Area and Contour Length \n");
    for( int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
        circle( drawing, mc[i], 4, color, -1, 8, 0 );
    }

    imshow("drawing", drawing);

    // Find largest mass
    float maxmass = 0;
    float rad = 0;
    Point2f masscenter(0, 0);
    for ( int i = 0; i < contours.size(); i++)
    {
        float cur = cv::contourArea(contours[i]);
            if ((maxmass < cur))
            {
            masscenter = cv::Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 );;
            maxmass = cur;
            rad = pow(contourArea(contours[i])/M_PI,1/2);
            }
    }
    logger->info(std::to_string(maxmass) + " radius");
    logger->info("mass center: " + to_string(masscenter.x) + " " + to_string(masscenter.y));
    *centerX = masscenter.x;
    *centerY = masscenter.y;
    *radius = rad;
    circle( drawing, masscenter, 200, Scalar(0,0,255), 5);
}

void BuoyTask::detectBuoy(ImgData* data, std::string stage, float *centerX, float *centerY, float *radius)
{
    logger->info("DETECTING BUOY");
    Mat src, src_HSV;
    src = data->getImg();

    // Convert image to HSV and blur it
    cvtColor( src, src_HSV, CV_BGR2HSV );
    blur( src_HSV, src_HSV, Size(3,3) );

    Mat colour_image1, colour_image2, colour_image;

    imshow("regular", data->getImg());

    if (stage == "RED")
    {
        // Filter the image for red
        // Lower red colour range
        inRange(src_HSV, Scalar(lowRedH, lowRedS, lowRedV),
                Scalar(highRedH, highRedS, highRedV), colour_image1);

        inRange(src_HSV, Scalar(lowRedH2, lowRedS2, lowRedV2),
                Scalar(highRedH2, highRedS2, highRedV2), colour_image2);
        // Upper red colour range
        addWeighted(colour_image1, 1.0, colour_image2, 1.0, 0.0, colour_image);

    } else if (stage == "GREEN")
    {
        // Filter for green
        // Lower green colour range
        inRange(src_HSV, Scalar(lowGreenH, lowGreenS, lowGreenV),
                Scalar(highGreenH, highGreenS, highGreenV), colour_image1);

        inRange(src_HSV, Scalar(lowGreenH2, lowGreenS2, lowGreenV2),
                Scalar(highGreenH2, highGreenS2, highGreenV2), colour_image2);

        addWeighted(colour_image1, 1.0, colour_image2, 1.0, 0.0, colour_image);
    }

    // Erosion and Dilation
    int dilation_type;
    int erosion_type;
    int erosion_elem = 0;
    int dilation_elem = 0;

    if( erosion_elem == 0 ){ erosion_type = MORPH_RECT; }
    else if( erosion_elem == 1 ){ erosion_type = MORPH_CROSS; }
    else if( erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }

    Mat element1 = getStructuringElement( erosion_type,
                                           Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                                           Point( erosion_size, erosion_size ) );

    Mat dilated;
    if( dilation_elem == 0 ){ dilation_type = MORPH_RECT; }
    else if( dilation_elem == 1 ){ dilation_type = MORPH_CROSS; }
    else if( dilation_elem == 2) { dilation_type = MORPH_ELLIPSE; }

    Mat element = getStructuringElement( dilation_type,
                                           Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                           Point( dilation_size, dilation_size ) );
    // Apply the dilation operation
    dilate( colour_image, dilated, element );

    // Apply the erosion operation
    erode( dilated, colour_image, element1 );

    imshow("filtered", colour_image);

    waitKey(1);
    printf("center: %f, %f, %f\n", centerX, centerY, radius);
    // Find the largest mass
    thresh_callback( 0, colour_image, centerX, centerY, radius);
}
