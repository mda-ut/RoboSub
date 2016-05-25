/*
 * SubFactory.h
 *
 *  Created on: May 25, 2015
 *      Author: carl
 */

#ifndef SUBFACTORY_H_
#define SUBFACTORY_H_

#include "Properties.h"
#include "Logger.h"
#include "Sub.h"
#include "../Simulator/Sim.h"

class Stage;

/**
 * The SubType enum
 * Used to distinguish what types of Model, View, Controller
 * we'll be using
 * GUI denotes displaying output to and receiving commands from the GuiView
 * SIM similar to GUI but will interface with simulated peripherals (SimCamera and SimFPGA)
 * AUT autonomous run, will still display output but runs autonomously (runs CompetitionTask)
 */
enum SubType {GUI, SIM, AUT};

class SubFactory {
private:
    /**
     * Logger for the SubFactory class
     */
    Logger* logger =  new Logger("SubFactory");

    /**
     * Stage object that
     */
    Stage* stage;

    /**
     * Holds the settings from the settings.txt file
     */
    Properties* settings;

public:
    /**
     * SubFactory constructor
     * @param settings Properties object that holds general settings like camera ids
     */
    SubFactory(Properties* settings);

    /**
     * SubFactory destructor
     */
    virtual ~SubFactory();

    /**
     * Sets the SubFactory's stage object to newStage
     * @param newStage
     */
    void setStage(Stage* newStage);

    /**
     * Creates a new Sub object based on the settings files and
     * the subType passed in
     * @param subType specifies the type of run we're doing (GUI, SIM, or AUT)
     * @return new Sub object
     */
    Sub* makeSub(std::string subType);



};

#endif /* SUBFACTORY_H_ */
