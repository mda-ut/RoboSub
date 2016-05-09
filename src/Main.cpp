/*
 * Main.cpp
 *
 *  Created on: Jan 7, 2015
 *      Author: mda
 */

#include <QApplication>
#include <string>
#include "util/Logger.h"
#include "scripts.h"
#include "Properties.h"
#include "PropertyReader.h"
#include "SubFactory.h"
#include "Stage.h"
#include "StateTester.h"


int main(int argc, char** argv) {

    Timer* logTimer = new Timer();
    QApplication app(argc, argv);
    Logger logger("Main");
    Logger::initialize(Logger::Level::TRACE, true, true, logTimer);
    logger.trace("Logger initialized.");

    char cwd[1024];
    char buf[1024];
    getcwd(cwd, sizeof(cwd));
    sprintf(buf, "Current working directory: %s", cwd);
    logger.info(buf);

    PropertyReader* propReader;
    Properties* settings;
    if (argc > 1) {
        propReader = new PropertyReader(argv[1]);
    } else {
        propReader = new PropertyReader("settings/settings.txt");
    }
    // Set logging level
    settings = propReader->load();
    std::string loggingLevel = settings->getProperty("LOGGING_LEVEL");
    if (loggingLevel == "OFF") {
        Logger::setLoggingLevel(Logger::Level::OFF);
    } else if (loggingLevel == "TRACE") {
        Logger::setLoggingLevel(Logger::Level::TRACE);
    } else if (loggingLevel == "INFO") {
        Logger::setLoggingLevel(Logger::Level::INFO);
    } else if (loggingLevel == "DEBUG") {
        Logger::setLoggingLevel(Logger::Level::DEBUG);
    } else if (loggingLevel == "WARN") {
        Logger::setLoggingLevel(Logger::Level::WARN);
    } else if (loggingLevel == "ERROR") {
        Logger::setLoggingLevel(Logger::Level::ERROR);
    }

    // Set default HSV Filter values
    if (!settings->getProperty("LOW_HUE").empty()) {
        HSVFilter::defaultLowH = std::stoi(settings->getProperty("LOW_HUE"));
    }
    if (!settings->getProperty("HIGH_HUE").empty()) {
        HSVFilter::defaultHighH = std::stoi(settings->getProperty("HIGH_HUE"));
    }
    if (!settings->getProperty("LOW_SATURATION").empty()) {
        HSVFilter::defaultLowS = std::stoi(settings->getProperty("LOW_SATURATION"));
    }
    if (!settings->getProperty("HIGH_SATURATION").empty()) {
        HSVFilter::defaultHighS = std::stoi(settings->getProperty("HIGH_SATURATION"));
    }
    if (!settings->getProperty("LOW_VALUE").empty()) {
        HSVFilter::defaultLowV = std::stoi(settings->getProperty("LOW_VALUE"));
    }
    if (!settings->getProperty("HIGH_VALUE").empty()) {
        HSVFilter::defaultHighV = std::stoi(settings->getProperty("HIGH_VALUE"));
    }

    std::string mode = settings->getProperty("MODE");

    if (mode == "HSVTEST") {
        //VideoTesting vt(0); //camera id or file name
        //vt.run(1); //0 = video; 1 = webcam; 2 = image
    } else {
        init_signal_handler();

        SubFactory* subFactory = new SubFactory(settings);
        Stage* mainStage = new Stage(NULL, subFactory);

        mainStage->setViewContent(mode);
        mainStage->initialize();

        delete propReader;
    }
    return app.exec();

}
