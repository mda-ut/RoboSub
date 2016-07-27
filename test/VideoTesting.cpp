#include "VideoTesting.h"

VideoTesting::VideoTesting(const std::string fileName){
        cap.open(fileName);
        if( !cap.isOpened()){
             std::cout << "Cannot open the video file" << std::endl;
             return;
        }
}

VideoTesting::VideoTesting(int deviceID) {
    cap.open(deviceID);
    if( !cap.isOpened()) {
        std::cout << "Cannot open webcam " << deviceID << std::endl;
        std::exit(-1);
    }
}

//================get next frame from camera=====================================

/*
cv::Mat getNextCameraFrame(){
    IplImage* frame = cvQueryFrame(capture); //Create image frames from capture
    cv::Mat temp = cv::cvarrToMat(frame,true,true,0);
    //delete frame;
    return temp;
}*/

cv::Mat VideoTesting::getNextCameraFrame(){
    cv::Mat frame;
    cap.read(frame);
    return frame;
}
//================HSV filter==========================================
cv::Mat HSVFilter(cv::Mat mat, int lowH, int highH, int lowS, int highS, int lowV, int highV){
    //cv::Mat* mat = data->getImg();
    cv::Mat imgHSV;
    //cv::Mat imgThresh = cv::Mat(mat.clone());
    cv::Mat imgThresh = mat.clone();

    cv::cvtColor(mat, imgHSV, cv::COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

    cv::inRange(imgHSV, cv::Scalar(lowH, lowS, lowV),
            cv::Scalar(highH, highS, highV), imgThresh); //Threshold the image

    //morphological opening (remove small objects from the foreground)
    cv::erode(imgThresh, imgThresh, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
    cv::dilate(imgThresh, imgThresh, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );

    //morphological closing (fill small holes in the foreground)
    cv::dilate(imgThresh, imgThresh, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );
    cv::erode(imgThresh, imgThresh, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)) );

    return imgThresh;
}
//================================= rgb filter =================================
cv::Mat RGBFilter(cv::Mat img, int r, int g, int b){

    assert(img.type() == CV_8UC3);

    cv::Mat pic;
    inRange(img, cv::Scalar(0, 0, 0), cv::Scalar(g, b, r), pic);

    return pic;
}

//=============================
cv::Mat HCT(cv::Mat img){
    cv::Mat src = cv::Mat::zeros(img.size(), CV_8UC3);
    cv::Mat src_gray = img.clone();

    /// Convert it to gray
    //cvtColor(img, src_gray, CV_BGR2GRAY );

    /// Reduce the noise so we avoid false circle detection
    GaussianBlur( src_gray, src_gray, cv::Size(9, 9), 2, 2 );

    std::vector<cv::Vec3f> circles;

    /// Apply the Hough Transform to find the circles
    HoughCircles( src_gray, circles, CV_HOUGH_GRADIENT, 1, src_gray.rows/8, 200, 100, 0, 0 );

    /// Draw the circles detected
    for( size_t i = 0; i < circles.size(); i++ )
    {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        // circle center
        circle( src, center, 3, cv::Scalar(0,255,0), -1, 8, 0 );
        // circle outline
        circle( src, center, radius, cv::Scalar(0,0,255), 3, 8, 0 );
    }

    return src;
}
//============================== moments ================================
cv::Mat Moments(cv::Mat img){
    cv::Mat canny_output;
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    int thresh = 100;
    /// Detect edges using canny
    Canny(img, canny_output, thresh, thresh*2, 3 );
    /// Find contours
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

    /// Get the moments
    std::vector<cv::Moments> mu(contours.size() );
    for( uint64_t i = 0; i < contours.size(); i++ )
    { mu[i] = moments( contours[i], false ); }

    ///  Get the mass centers:
    std::vector<cv::Point2f> mc( contours.size() );
    for( uint64_t i = 0; i < contours.size(); i++ )
    { mc[i] = cv::Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }


    /// Draw contours
    cv::Mat drawing = cv::Mat::zeros( canny_output.size(), CV_8UC3 );
    for( uint64_t i = 0; i< contours.size(); i++ )
    {
        cv::Scalar color = cv::Scalar(0, 255, 0);
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, cv::Point() );
        circle( drawing, mc[i], 4, color, -1, 8, 0 );
    }


    /// Calculate the area with the moments 00 and compare with the result of the OpenCV function
    //printf("\t Info: Area and Contour Length \n");
    for( uint64_t i = 0; i< contours.size(); i++ )
    {
        //printf(" * Contour[%d] - Area (M_00) = %.2f - Area OpenCV: %.2f - Length: %.2f \n", i, mu[i].m00, contourArea(contours[i]), arcLength( contours[i], true ) );
        cv::Scalar color = cv::Scalar(255, 0, 0);
        drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, cv::Point() );
        circle( drawing, mc[i], 4, color, -1, 8, 0 );
    }
    return drawing;
}
//============blur filter============================================
cv::Mat blur(cv::Mat src, int max){
    //cv::Mat* dst = new cv::Mat(src->clone());
    cv::Mat dst(cv::Mat::zeros(src.size(), CV_8UC3));
    max = max*2+1;

    GaussianBlur(src, dst, cv::Size(max, max), 0,0);
    //blur(*src, *dst, cv::Size(3,3), cv::Point(-1,-1));
    //medianBlur(*src, *dst, max);
    //bilateralFilter(*src, *dst, max, max*2, max/2);
    return dst;
}

//=========== buoy =============================================
using namespace cv;
using namespace std;
int thresh = 100;

void thresh_callback(Mat all_colour_image)
{
    RNG rng(12345);
    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // Detect edges and contours
    Canny( all_colour_image, canny_output, thresh, thresh*2, 3 );
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    std::vector<cv::Moments> mu(contours.size() );
    for( int i = 0; i < contours.size(); i++ )
    {
        mu[i] = moments( contours[i], false );
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
            rad = pow(contourArea(contours[i])/M_PI,0.6);
            }
    }
//    cout<<(std::to_string(maxmass) + " radius\n");
//    cout<<("mass center: " + to_string(masscenter.x) + " " + to_string(masscenter.y)+"\n");
    circle( drawing, masscenter, rad, Scalar(255,0,255), 1);

    imshow("circles", drawing);

}

cv::Mat getCutoutOfOrginial(cv::Mat org, cv::Mat binary){
    cv::Mat cutout = org.clone();
    for (int x = 0; x < binary.rows; x++){
        for (int y = 0; y < binary.cols; y++){
            if (binary.at<uchar>(x, y) == 0){
                cutout.at<cv::Vec3b>(x,y)[0]=0;
                cutout.at<cv::Vec3b>(x,y)[1]=0;
                cutout.at<cv::Vec3b>(x,y)[2]=0;
            }
        }
    }
    return cutout;
}

std::string findColor(cv::Vec3b color){
    //0 = b; 1 = g; 2 = r
    int bg = color[0] - color[1];
    int br = color[0] - color[2];
    int gb = color[1] - color[0];
    int gr = color[1] - color[2];
    int rb = color[2] - color[0];
    int rg = color[2] - color[1];
    if (bg > 10 && br > 10)         return "Blue";
    else if (gb > 10 && gr > 10)    return "Green";
    else if (rb > 10 && rg > 10)    return "Red";
    else                            return "idk, yellow?";
}

cv::Mat matIntersect(cv::Mat m1, cv::Mat m2){
    cv::Mat intersect = cv::Mat::zeros(m1.size(), CV_8UC1);
    for (int y = 0; y < m1.cols; y++){
        for (int x = 0; x < m1.rows; x++){
            if (m1.at<uchar>(x, y) == m2.at<uchar>(x, y)){
                intersect.at<uchar>(x, y) = m2.at<uchar>(x, y);
            }
        }
    }
    return intersect;
}


//==============================================================
void VideoTesting::run(int Type){
    vector<string> windows;
    windows.push_back("HSV");
    windows.push_back("HSV2");
    windows.push_back("Orginal");
    windows.push_back("Line Filtered");
    windows.push_back("Canny");
    windows.push_back("circles");
    windows.push_back("bc");
    const int CONTROL_WINDOWS = 2;
    const string CONTROL_IMG_NAME = "Image ";
    const string CONTROL_WINDOW_NAME = "Control";

    const int WINDOW_SIZE_X = 480;      // 640 480 360
    const int WINDOW_SIZE_Y = 360;      // 480 360 240

    for (int i = 0; i < windows.size(); i++){
        //using CV_WINDOW_NORMAL breaks the rgb values in the bottom of the window. Use AUTOSIZE if thats needed
        cv::namedWindow(windows[i], CV_WINDOW_NORMAL);
        cv::resizeWindow(windows[i], WINDOW_SIZE_X, WINDOW_SIZE_Y);
    }
    for (int i = 1; i <= CONTROL_WINDOWS; i++){
        cv::namedWindow(CONTROL_IMG_NAME+std::to_string(i), CV_WINDOW_NORMAL);
        cv::resizeWindow(CONTROL_IMG_NAME+std::to_string(i)
                         , WINDOW_SIZE_X, WINDOW_SIZE_Y);
        cv::namedWindow(CONTROL_WINDOW_NAME+std::to_string(i), CV_WINDOW_AUTOSIZE);
    }

    int rowMax = screenY/(WINDOW_SIZE_Y+30) - 1;
    int rowCur = 0;
    int curY = 50;
    int curX = screenX - WINDOW_SIZE_X;
    for (int i = 0; i < windows.size(); i++){
        cv::moveWindow(windows[i], curX, curY);
        rowCur++;
        if (rowCur > rowMax){
            curX -= WINDOW_SIZE_X;
            curY = 50;
            rowCur = 0;
        }else
            curY += WINDOW_SIZE_Y;
    }
    for (int i = 0; i < CONTROL_WINDOWS; i++){
        cv::moveWindow(CONTROL_IMG_NAME+to_string(i+1), i*(WINDOW_SIZE_X+75), 200);
        cv::moveWindow(CONTROL_WINDOW_NAME+to_string(i+1), i*(WINDOW_SIZE_X+75), WINDOW_SIZE_Y + 250);
    }

    cv::Mat frame;
    cv::Mat filtered;
    cv::Mat hsvFiltered;
    cv::Mat hsvFiltered2;
    cv::Mat lineFiltered;
    cv::Mat contour;
    cv::Mat brightContra;
    int frameCount;
if (Type == 0){
    printf("type == 0\n");
        frameCount = cap.get(CV_CAP_PROP_POS_FRAMES);
}
    int ialpha = 100, ibeta = 100;

    int iLowH = 0;
    int iHighH = 111;
    int iLowS = 0;
    int iHighS = 184;
    int iLowV = 82;
    int iHighV = 183;
    //Create trackbars in "Control" window
    cvCreateTrackbar("LowH", "Control1", &iLowH, 179); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control1", &iHighH, 179);
    cvCreateTrackbar("LowS", "Control1", &iLowS, 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control1", &iHighS, 255);
    cvCreateTrackbar("LowV", "Control1", &iLowV, 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "Control1", &iHighV, 255);
    cvCreateTrackbar("contrast", "Control1", &ialpha, 300);
    cvCreateTrackbar("brightness", "Control1", &ibeta, 200);
if (Type == 0){
        cvCreateTrackbar("Frame", "Control1", &frameCount, cap.get(CV_CAP_PROP_FRAME_COUNT));
}
    int hsv2Params[6];
    hsv2Params[0] = 0;          //low H
    hsv2Params[1] = 179;        //high H
    hsv2Params[2] = 196;          //low S
    hsv2Params[3] = 255;        //high S
    hsv2Params[4] = 0;          //low V
    hsv2Params[5] = 206;        //high V

    cvCreateTrackbar("LowH", "Control2", &hsv2Params[0], 179); //Hue (0 - 179)
    cvCreateTrackbar("HighH", "Control2", &hsv2Params[1], 179);
    cvCreateTrackbar("LowS", "Control2", &hsv2Params[2], 255); //Saturation (0 - 255)
    cvCreateTrackbar("HighS", "Control2", &hsv2Params[3], 255);
    cvCreateTrackbar("LowV", "Control2", &hsv2Params[4], 255); //Value (0 - 255)
    cvCreateTrackbar("HighV", "Control2", &hsv2Params[5], 255);

    //HSVFilter hf(25, 179, 0, 255, 0,255);
    LineFilter lf;
    ShapeFilter sf(1, 3);
    BlurFilter bf(2, 0.2f);
    BlurFilter bf2(1, 0.4f);
    if (Type == 2){
        cap.read(frame);
        if (frame.data == NULL){
            printf("Failed to read image\n");
            return;
        }
    }
    cv::Scalar color = cv::Scalar(255, 0, 0);
    bool pause = false;
    while (1){
        if (!pause){
            contour = cv::Mat::zeros(frame.size(), CV_8UC3);
            if (Type == 0)
                frame = this->getNextFrame(); //video
            else if (Type == 1)
                frame = getNextCameraFrame(); //webcam
            if(frame.data == NULL){
                printf("no more frames\n");
                break;       //exit when there is no next fraame
            }
            contour = frame.clone();
if (Type == 0){
                frameCount++;
                cv::setTrackbarPos("Frame", "Control1", frameCount);
}
        }
        brightContra = frame.clone();
        float alpha = ialpha/100.0f;
        int beta = ibeta-100;
        for (int i = 0; i < frame.cols; i++){
            for (int j = 0; j < frame.rows; j++){
//                brightContra.at<Vec3b>(j, i)[0] = 0;
//                brightContra.at<Vec3b>(j, i)[1] = 0;
//                brightContra.at<Vec3b>(j, i)[0] = frame.at<Vec3b>(j, i)[0]/10;
//                brightContra.at<Vec3b>(j, i)[1] = frame.at<Vec3b>(j, i)[1]/9;
//                if (brightContra.at<Vec3b>(j, i)[1] <= 0)  brightContra.at<Vec3b>(j, i)[0] = 1;
//                for( int c = 0; c < 3; c++ )
//                    { brightContra.at<Vec3b>(j,i)[c] =
//                        saturate_cast<uchar>( alpha*( brightContra.at<Vec3b>(j,i)[c] ) + beta ); }
            }
        }
        imshow("bc", brightContra);

        hsvFiltered = HSVFilter(brightContra, iLowH, iHighH, iLowS, iHighS, iLowV, iHighV);
        hsvFiltered2 = HSVFilter(brightContra, hsv2Params[0], hsv2Params[1], hsv2Params[2], hsv2Params[3], hsv2Params[4], hsv2Params[5]);
//        hsvFiltered2 = getCutoutOfOrginial(frame, filtered);
        filtered = hsvFiltered - hsvFiltered2;
        imshow("HSV", filtered);
        imshow("HSV2", getCutoutOfOrginial(frame, filtered));
        //filtered = blur(filtered,max);
        //filtered = bf.filter(filtered2);
        //filtered = bf2.filter(filtered);
        lineFiltered = lf.filter(filtered, 0);
        //lineFiltered = Moments(filtered);
        thresh_callback(filtered);

        //draw rectangle
        if (sf.findRect(filtered)){
            std::vector<cv::RotatedRect> rect = sf.getRect();
            for (cv::RotatedRect rekt: rect){
                cv::Point2f rect_points[4]; rekt.points( rect_points );
                for( int j = 0; j < 4; j++ )
                   line( contour, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
                //delete rect;
//                std::cout<<rekt.angle<<std::endl;
                cv::Point2f ps[4];
                rekt.points(ps);
//                std::cout << ps[0] << " " << ps[1] << " " << ps[2] << " " << ps[3] << std::endl;
            }
        }

        //draw circle
        if (sf.findCirc(filtered)){
            auto rad = sf.getRad();
            auto cent = sf.getCenter();
            //radius
            for (unsigned int i = 0; i < rad.size(); i++){
                if (findColor(frame.at<Vec3b>(cent[i])) == "Red")
                cv::circle(lineFiltered, cent[i], rad[i], cv::Scalar(255,0,0) );
            }
            //center
            for (unsigned int i = 0; i < rad.size(); i++){
                if (findColor(frame.at<Vec3b>(cent[i])) == "Red"){
                    cv::circle(lineFiltered, cent[i], 2, cv::Scalar(0,255,0));
                }
            }
        }

        //gets the mass center; testing purposes only
        auto t = sf.findMassCenter(filtered);
        for (cv::Point2f p: t){
//            cv::circle(lineFiltered, p, 2, cv::Scalar(0,0,255));
        }

        imshow("Orginal", frame);
        imshow(CONTROL_IMG_NAME+"1", hsvFiltered);
        imshow(CONTROL_IMG_NAME+"2", hsvFiltered2);
        imshow("Line Filtered", lineFiltered);
        imshow("Canny", contour);
if (Type == 0){
        if (frameCount != cap.get(CV_CAP_PROP_POS_FRAMES)){
            cap.set(CV_CAP_PROP_POS_FRAMES, frameCount);
            frame = getNextFrame();
            contour = frame.clone();
        }
}
        int key;
        if (pause){
            key = cv::waitKey(10);
            if (key == 91){         //91 = [
                cap.set(CV_CAP_PROP_POS_FRAMES, cap.get(CV_CAP_PROP_POS_FRAMES)-2);
                frame = getNextFrame();
                contour = frame.clone();
                frameCount--;
                if (frameCount <= 0)    frameCount = 0;
                cv::setTrackbarPos("Frame", "Control1", frameCount);
            }else if (key == 93){   //93 = ]
                cap.set(CV_CAP_PROP_POS_FRAMES, cap.get(CV_CAP_PROP_POS_FRAMES));
                frame = getNextFrame();
                contour = frame.clone();
                frameCount++;
                cv::setTrackbarPos("Frame", "Control1", frameCount);
            }
        }
        else
            key = cv::waitKey(100);        //wait for 33ms, ~= 30fps;
        //std::cout<<key<<std::endl;
        if (key == 27) break;               //if user press esc, break the loop
        else if(key == 98) {
            if (Type == 0){
//            cv::waitKey(0);  //"b" key, "pasues" play
                pause = !pause;
            }
        }
        /*delete filtered;
        delete filtered2;
        delete lineFiltered;*/
    }
    //cvReleaseCapture(&capture);
    if (cap.isOpened())
        cap.release();
    std::cout << "End of video feed" << std::endl;
}

cv::Mat VideoTesting::getNextFrame(){
    cv::Mat frame;
    bool success = cap.read(frame);
    if (!success){
      std::cout << "Cannot read  frame " << std::endl;
      //return 0;
    }
    return frame;
}
