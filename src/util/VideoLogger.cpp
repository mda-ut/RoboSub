/*
 * VideoLogger.cpp
 *
 *  Created on: Jul 2, 2015
 *      Author: ahsueh1996
 */

#include "VideoLogger.h"


VideoLogger::VideoLogger(std::string filename, int width, int height, int fps) {
    logger = new Logger(filename+"-VideoLogging");

    if (fps < 1) {
        //PropertyReader* propReader = new PropertyReader("../../Sub/src/settings/settings.txt");
        //Properties* settings = propReader->load();
        //this->fps = std::stoi(settings->getProperty("CAM_POLL_FREQUENCY"));
        //delete settings;
    } else
        this->fps = fps;

    //PropertyReader* videoLoggerPropReader = new PropertyReader("../../Sub/src/settings/videoLogger.txt");
    //Properties* videoSettings = videoLoggerPropReader->load();
    //this->pad = std::stoi(videoSettings->getProperty("ZEROPAD"));
    //this->filetype = videoSettings->getProperty("FILETYPE");
    this->pad = 2;
    this->filetype = ".avi";
    //delete videoSettings;

    this->it = 0;
    this->filename = filename;
    this->error = 0;

    timer.start();

    ///----------------------------------------------------------------
    char buffer[80];
    strftime(buffer, 80, "%I:%M:%S_%d-%m-%Y.VIDEO", timer.getTimeStamp());

    std::string logName(buffer);

    /*
    //Create a logs folder if one does not exist and places a log file into
    QString test= QString::fromstdString(Util::getWorkingDirectory());
    QDir dir(test);

    if (!(QDir(QString::fromstdString(Util::getWorkingDirectory()+"/videos")).exists())) {
        dir.mkpath("videos");
    }*/
    folderName = logName;
    ///-----------------------------------------------------------------

    // make the output directory
    //std::system(("mkdir videoLog && cd videoLog && mkdir "+filename).c_str());
}

VideoLogger::~VideoLogger() {
    //std::system(("ffmpeg -i \'videoLog/"+filename+"/%0"+std::to_string(pad)+"d.jpg\' -r "+std::to_string(fps)+" \'videoLog/"+filename+"/"+filename+".avi\'").c_str());
    delete logger;
}

void VideoLogger::write(ImgData frame) {
    write(frame.getImg());
}

void VideoLogger::write(cv::Mat frame) {
    if (error == 0) {
        if (timer.getTimeElapsed()*1000.0 > 1000.0/fps){
            timer.start();
            //cv::imwrite("videoLog/testvid/"+std::to_string(it++)+".jpg",frame);
            imwrite("videoLog/" + folderName + filename + std::to_string(it) + ".png", frame);
            it++;
        }
    }
}
