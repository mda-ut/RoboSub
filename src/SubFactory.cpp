/*
 * SubFactory.cpp
 *
 *  Created on: May 25, 2015
 *      Author: carl
 */

#include "SubFactory.h"
#include "CameraModel.h"
#include "SimCameraInterface.h"
#include "CameraInterface.h"
#include "CameraState.h"
#include "FPGAModel.h"
#include "FPGAInterface.h"
#include "FPGAState.h"
#include "MenuView.h"
#include "GUIView.h"
#include "CompetitionView.h"
#include "Controller.h"
#include "SimFPGA.h"
#include "SimFPGAInterface.h"
#include "SimBufferWindow.h"
#include <vector>
#include <iostream>

SubFactory::SubFactory(Properties* settings) {
    this->settings = settings;
    stage = nullptr;
}

SubFactory::~SubFactory() {
    delete settings;
    delete logger;
}

void SubFactory::setStage(Stage* newStage) {
    stage = newStage;
}

Sub* SubFactory::makeSub(std::string subType) {
    std::vector<Model*> models;
    std::vector<State*> states;
    View* view;
    Controller* controller;
    //TODO: Add error checking for property file reading
    logger->trace(std::to_string(std::stoi(settings->getProperty("CAM_BUFFER_SIZE"))));
    int camBufferSize = std::stoi(settings->getProperty("CAM_BUFFER_SIZE"));
    double camPollFrequency = std::stoi(settings->getProperty("CAM_POLL_FREQUENCY"));
    int fpgaBufferSize = std::stoi(settings->getProperty("FPGA_BUFFER_SIZE"));
    double fpgaPollFrequency = std::stod(settings->getProperty("FPGA_POLL_FREQUENCY"));

    if (subType == "MENU") {
        //MENU is just a window that lets you pick which sub to create graphically
        controller = new Controller(models);
        view = new MenuView(stage);

    } else if (subType == "GUI") {
        logger->trace("Creating GUI sub");

        states.push_back(new CameraState(FRONTCAM, camBufferSize));
        states.push_back(new CameraState(DOWNCAM, camBufferSize));
        states.push_back(new FPGAState(FPGA, fpgaBufferSize));

        int frontCamPos = std::stoi(settings->getProperty("FRONT_CAM"));
        int downCamPos = std::stoi(settings->getProperty("DOWN_CAM"));
        HwInterface* frontCamInt = new CameraInterface(frontCamPos);
        HwInterface* downCamInt = new CameraInterface(downCamPos);
        HwInterface* fpgaInt = new FPGAInterface(settings);

        models.push_back(new CameraModel(states[0], frontCamInt, camPollFrequency));
        models.push_back(new CameraModel(states[1], downCamInt, camPollFrequency));
        models.push_back(new FPGAModel(states[2], fpgaInt, fpgaPollFrequency));

        controller = new Controller(models);
        view = new GUIView(stage, controller, states);
        controller->setView(view);

        for (auto& state : states) {
            state->addViewer(view);
        }
    } else if (subType == "SIMULATOR") {
        logger->trace("Creating simulation sub");
        Qt3D::QEntity* rootEntity = new Qt3D::QEntity();
        SimulatedSub* simSub = new SimulatedSub(rootEntity);
        SimulatedEnvironment* simEnv = new SimulatedEnvironment(rootEntity);

        states.push_back(new CameraState(FRONTCAM, camBufferSize));
        states.push_back(new CameraState(DOWNCAM, camBufferSize));
        states.push_back(new FPGAState(FPGA, fpgaBufferSize));

        SimFPGA* simFPGA = new SimFPGA(settings, simSub);
        SimBufferWindow* bufferWindow = new SimBufferWindow(simSub, simEnv, rootEntity, settings);
        int frontCamPos = std::stoi(settings->getProperty("FRONT_CAM"));
        int downCamPos = std::stoi(settings->getProperty("DOWN_CAM"));

        HwInterface* frontCamInt = new SimCameraInterface(frontCamPos, bufferWindow);
        HwInterface* downCamInt = new SimCameraInterface(downCamPos, bufferWindow);
        HwInterface* fpgaInt = new SimFPGAInterface(settings, simFPGA);

        models.push_back(new CameraModel(states[0], frontCamInt, camPollFrequency));
        models.push_back(new CameraModel(states[1], downCamInt, camPollFrequency));
        models.push_back(new FPGAModel(states[2], fpgaInt, fpgaPollFrequency));

        controller = new Controller(models);
        view = new GUIView(stage, controller, states);

        controller->setView(view);

        for (auto& state : states) {
            state->addViewer(view);
        }


        bufferWindow->initialize();
    } else if (subType == "AUTONOMOUS") {
        logger->trace("Creating autonomous sub");
        states.push_back(new CameraState(FRONTCAM, camBufferSize));
        states.push_back(new CameraState(DOWNCAM, camBufferSize));
        states.push_back(new FPGAState(FPGA, fpgaBufferSize));

        int frontCamPos = std::stoi(settings->getProperty("FRONT_CAM"));
        int downCamPos = std::stoi(settings->getProperty("DOWN_CAM"));
        HwInterface* frontCamInt = new CameraInterface(frontCamPos);
        HwInterface* downCamInt = new CameraInterface(downCamPos);
        HwInterface* fpgaInt = new FPGAInterface(settings);

        models.push_back(new CameraModel(states[0], frontCamInt, camPollFrequency));
        models.push_back(new CameraModel(states[1], downCamInt, camPollFrequency));
        models.push_back(new FPGAModel(states[2], fpgaInt, fpgaPollFrequency));

        controller = new Controller(models);
        view = new CompetitionView(stage, controller);
        controller->setView(view);
        for (auto& state : states) {
            state->addViewer(view);
        }
    } else {
        logger->error("Unrecognized sub type " + subType);
	}

    Sub* sub = new Sub(models, view, controller);
    return sub;
}

