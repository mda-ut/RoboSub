/*
 * Sub.h
 *
 *  Created on: Jan 8, 2015
 *      Author: mda
 */

#ifndef SUB_H_
#define SUB_H_

#include "Model.h"
#include "Controller.h"
#include "View.h"
#include <vector>

/**
 * The Sub class holds the Model, View, and
 * Controller components of the submarine
 */
class Sub {
private:
    std::vector<Model*> models; //One sub can have multiple models (ie one fpga, two cameras)
    View* view;
    Controller* controller;

public:
    /**
     * Constructor for the Sub class
     * @param models_ list of models the Sub interfaces with
     * @param view_ visual output for the Sub
     * @param controller_ object that controls the Sub's actions
     */
    Sub(std::vector<Model*> models_, View* view_, Controller* controller_);

    /**
     * Sub destructor
     */
	virtual ~Sub();

    /**
     * Initializes each of the objects within Sub
     */
    void initialize();

    /**
     * @brief getView
     * @return Sub's View object
     */
    View* getView();

};

#endif /* SUB_H_ */
