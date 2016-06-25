#include "BuoyTask.h"

using namespace cv;
using namespace std;

BuoyTask::BuoyTask(Model *fpgaModel, TurnTask *tk, SpeedTask *st, DepthTask *dt){
    this->camModel = dynamic_cast<CameraModel*>(camModel);
    this->tk = tk;
    this->st = st;
    this->dt = dt;
    
    // Load properties file
    PropertyReader* propReader;
    propReader = new PropertyReader("../src/settings/buoy_task_settings.txt");
    settings = propReader->load();
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
    float * radius = NULL; ///Pointer for Circle Radius
    float * centerX = NULL; ///Pointer for X Coor of Circle Center
    float * centerY = NULL; ///Pointer for Y Coor of Circle Center
    int delta = 0; ///Change in Movement Needed
    int deltaNext = 0; ///Moving to next buoy
    int deltaGreen = 0; ///Moving to the green buoy
    int deltaYellow = 0; ///Moving to the yellow buoy
    int thresholdRadis = std::stoi(settings->getProperty("thresholdRadius"));
    int alignRadius = std::stoi(settings->getProperty("alignRadius"));

    // Set up data the sub is receiving
    std::string s = "raw";
    ImgData *data;
    int imgWidth, imgHeight;

    st->setTargetSpeed(std::stoi(settings->getProperty("targetSpeed")));
    st->execute();

    // Detect buoys while the sub is not done the task
    while (!done){
        data = dynamic_cast<ImgData*>(dynamic_cast<CameraState*>(camModel->getState())->getDeepState(s));
        imgWidth = data->getImg().size().width;
        imgHeight = data->getImg().size().height;

        // Find the buoy parameters
        detectBuoy(data, stage, centerX, centerY, radius);

        // Do nothing if no buoys have been found
        if (*radius == -1) {

        // A buoy has been detected in the distance
        } else if (*radius < thresholdRadius && !transition)
        {
                if (!(imgWidth/2 - alignRadius < *centerX < imgWidth/2 + alignRadius)){

                    // Align the sub to the x-coordinate of the detected buoy
                    delta = atan((*centerX - imgWidth/2)/imgHeight/2);
                    logger->debug("Rotating sub by " + std::to_string(delta) + " degrees.");
                    tk->setYawDelta(delta);
                    tk->execute();
                    usleep(1000000);
                }
                if (!(imgHeight/2 - alignRadius < *centerY and *centerY < imgHeight/2 + alignRadius)){

                    // Align the sub to the y-coordinate of the detected buoy
                    delta = (imgHeight/2 - *centerY)/20;
                    logger->debug("Changing height by " + std::to_string(delta));
                    tk->setYawDelta(delta);
                    tk->execute();
                    usleep(5000000);
                }
        // Transition to the next buoy if the sub is sufficiently close to the current buoy
        } else if (*radius >= thresholdRadius && !transition){
                if (stage == "RED"){
                    stage = "GREEN";
                    transition = true;
                    usleep(10000000);
                } else if (stage == "GREEN"){
                    stage = "YELLOW";
                    transition = true;
                    usleep(10000000);

                // The sub has hit the last buoy
                } else{
                    done = true;
                }
        }

        // Move on to next buoy
        if (transition) {
             delta = deltaNext;
             logger->debug("Changing speed by " + std::to_string(delta));
             st->setTargetSpeed(delta);
             st->execute();
             usleep(100000000);

             // Orient the sub to the next buoy
             if (stage == "GREEN") {
                 delta = deltaGreen;
                 logger->debug("Orienting to GREEN");
                 tk->setYawDelta(delta);
                 tk->execute();
                 usleep(10000000);
             } else if (stage == "YELLOW"){
                 delta = deltaYellow;
                 logger->debug("Orienting to YELLOW");
                 tk->setYawDelta(delta);
                 tk->execute();
                 usleep(10000000);
             }
        }
    }
}

void BuoyTask::thresh_callback(int, Mat all_colour_image, float *centerX, float *centerY, float *radius )
{
    int thresh = std::stoi(settings->getProperty("thresh"));
    int max_thresh = std::stoi(settings->getProperty("max_thresh"));
    RNG rng(12345);
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // Detect edges and contours
    Canny( all_colour_image, canny_output, thresh, thresh*2, 3 );
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    vector<Moments> mu(contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    { mu[i] = moments( contours[i], false ); }

    // Get the mass centers:
    vector<Point2f> mc( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }

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
        // printf(" * Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f - Length: %.2f \n", i, mu[i].m00, contourArea(contours[i]), arcLength( contours[i], true ) );
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
        circle( drawing, mc[i], 4, color, -1, 8, 0 );
    }

    // Find largest mass
    float maxmass = 0;
    Point2f masscenter(0, 0);
    for ( int i = 0; i < mc.size(); i++)
    {
        float cur = pow(contourArea(contours[i])/M_PI,1/2);
            if (maxmass < cur)
            {
            masscenter = mc[i];
            maxmass = cur;
            }
    }
    *centerX = masscenter.x;
    *centerY = masscenter.y;
    *radius = maxmass;
}

void BuoyTask::detectBuoy(ImgData* data, std::string stage, float *centerX, float *centerY, float *radius)
{
    Mat src, src_HSV;
    Mat lower_hue_range_image, upper_hue_range_image;
    src = data->getImg();

    // Convert image to HSV and blur it
    cvtColor( src, src_HSV, CV_BGR2HSV );
    blur( src_HSV, src_HSV, Size(3,3) );

    Mat colour_image;

    if (stage == "RED")
    {
        // Filter the image for red
        inRange(src_HSV, Scalar(std::stoi(settings->getProperty("lowRedH")), 
        		std::stoi(settings->getProperty("lowRedS")),
				std::stoi(settings->getProperty("lowRedV"))), 
        		Scalar(std::stoi(settings->getProperty("highRedH")), 
        				std::stoi(settings->getProperty("highRedS")), 
						std::stoi(settings->getProperty("highRedV"))), lower_hue_range_image);
        inRange(src_HSV, Scalar(std::stoi(settings->getProperty("upperLowRedH")),
        		std::stoi(settings->getProperty("upperLowRedS")), 
				std::stoi(settings->getProperty("upperLowRedV"))), 
        		Scalar(std::stoi(settings->getProperty("upperHighRedH")), 
        				std::stoi(settings->getProperty("upperHighRedS")), 
						std::stoi(settings->getProperty("upperHighRedV"))), upper_hue_range_image);
        addWeighted(lower_hue_range_image, 1.0, upper_hue_range_image, 1.0, 0.0, colour_image);
    } else if (stage == "GREEN")
    {
        // Filter for green
    	inRange(src_HSV, Scalar(std::stoi(settings->getProperty("lowGreenH")), 
    	        		std::stoi(settings->getProperty("lowGreenS")),
    					std::stoi(settings->getProperty("lowGreenV"))), 
    	        		Scalar(std::stoi(settings->getProperty("highGreenH")), 
    	        				std::stoi(settings->getProperty("highGreenS")), 
    							std::stoi(settings->getProperty("highGreenV"))), lower_hue_range_image);
    	        inRange(src_HSV, Scalar(std::stoi(settings->getProperty("upperLowGreenH")),
    	        		std::stoi(settings->getProperty("upperLowGreenS")), 
    					std::stoi(settings->getProperty("upperLowGreenV"))), 
    	        		Scalar(std::stoi(settings->getProperty("upperHighGreenH")), 
    	        				std::stoi(settings->getProperty("upperHighGreenS")), 
    							std::stoi(settings->getProperty("upperHighGreenV"))), upper_hue_range_image);
        addWeighted(lower_hue_range_image, 1.0, upper_hue_range_image, 1.0, 0.0, colour_image);
    } else
    {
        // Filter for yellow
        inRange(src_HSV, Scalar(std::stoi(settings->getProperty("lowYellowH")), 
        		std::stoi(settings->getProperty("lowYellowS")),
				std::stoi(settings->getProperty("lowYellowV"))), 
        		Scalar(std::stoi(settings->getProperty("highYellowH")), 
        				std::stoi(settings->getProperty("highYellowS")), 
						std::stoi(settings->getProperty("highYellowV")));
    }

    // Erosion and Dilation
    int dilation_type;
    int erosion_type;
    int erosion_elem = 0;
    int erosion_size = std::stoi(settings->getProperty("erosion_size"));
    int dilation_elem = 0;
    int dilation_size = std::stoi(settings->getProperty("dilation_size"));

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

    // Find the largest mass
    thresh_callback( 0, colour_image, centerX, centerY, radius);
}
