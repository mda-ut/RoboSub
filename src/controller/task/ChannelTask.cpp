/*
 *  ChannelTask.cpp
 *
 *  Created on: Jul 28 2016
 *  Modified from jwong's gate code, and path code
 *
 */

#include "ChannelTask.h"
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

ChannelTask::ChannelTask(Model* fpgaModel, DepthTask* depthTask, TurnTask* turnTask, SpeedTask* speedTask) {
    this->fpgaModel = fpgaModel;
    this->depthTask = depthTask;
    this->turnTask = turnTask;
    this->speedTask = speedTask;
}


/**
 * James' function for rotating using Turntask
 * takes input in ******degrees********
 *
 */
void ChannelTask::rotate(float angle) {
  logger->debug("Rotating sub by " + std::to_string(angle) + " degrees");
  turnTask->setYawCurrentDelta(angle);
  turnTask->execute();
  sleep(abs(angle)/180*10);
}

void ChannelTask::execute() {
    // Load properties file
    PropertyReader* propReader;
    Properties* settings;
    propReader = new PropertyReader("settings/channel_task_settings.txt");
    settings = propReader->load();


    logger->trace("Starting Channel Task");
    Timer timer;
    timer.start();
    FPGAData* temp = dynamic_cast<FPGAData*>(fpgaModel->getStateData("raw"));
    depthTask->setDepthAbsolute(temp->getDepth());
//    depthTask->setDepthAbsolute(std::stoi(settings->getProperty("POOL_SURFACE_HEIGHT")));


    //what does this do??
    logger->info("Set starting yaw to " + std::to_string(temp->getYaw()));
    turnTask->setYawAbsolute(temp->getYaw());
    turnTask->execute();
    while (timer.getTimeElapsed() < 5) {
        std::this_thread::yield();
    }

    //Set depth
    //TODO: adjust to reflect going down ~31 ft below surface
    //38 ft is depth of pool. Channel is 4-6 ft above.
    //Definitely need to clear the channel so probably won't risk going deeper

    depthTask->execute();
    logger->info("Set surface depth to " + std::to_string(temp->getDepth()));
    depthTask->setDepthDelta(std::stoi(settings->getProperty("START_DEPTH_DELTA")));    // Use relative depth movements
    depthTask->setDepthAbsolute(std::stoi(settings->getProperty("START_DEPTH_DELTA")) + temp->getDepth());
    depthTask->execute();
    timer.start();

    while (timer.getTimeElapsed() < std::stoi(settings->getProperty("DEPTH_WAIT_TIME"))) {
        std::this_thread::yield();
    }
    temp = dynamic_cast<FPGAData*>(fpgaModel->getStateData("raw"));
    logger->info("Sank to depth " + std::to_string(temp->getDepth()));

    //go forward by correct amount
    //seems to be about 1.5x the distance between the launchpad and gate...
    speedTask->setTargetSpeed(std::stoi(settings->getProperty("FORWARD_SPEED")));
    speedTask->execute();
    logger->info("Moving forward at speed " + std::stoi(settings->getProperty("FORWARD_SPEED")));
    timer.start();
    while (timer.getTimeElapsed() < std::stoi(settings->getProperty("FORWARD_TIME"))) {
        std::this_thread::yield();
    }
    speedTask->setTargetSpeed(0);
    speedTask->execute();
    logger->info("Stopped");

    //rotate by 90 degrees each time, to a total of 720
    int numTurns=8;
    for (int i = 0; i < numTurns; i++){
        rotate(90);
        sleep(2);
    }


    logger->info("Channel Task complete");
}

ChannelTask::~ChannelTask() {
    delete logger;
}

