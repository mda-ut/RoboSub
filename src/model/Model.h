/*
 * Model.h
 *
 *  Created on: Jan 7, 2015
 *      Author: mda
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <vector>
#include <string>
#include <thread>

#include "interface/HwInterface.h"
#include "FilterManager.h"
#include "FilterFactory.h"
#include "State.h"


/**
 * Model represents the physical attributes of the sub. It accepts input from cameras and FPGA
 * and outputs to FPGA and cameras (strictly speaking). Model contains references to the tools required
 * for the assets to run. Each Model holds an interface which it uses to communicate with the camera or FPGA
 * It also holds a State that contains Data that it receives from said Interface
 * Each Model has a FilterManager controlled by Controller that manages the preprocessing of the data
 *
 * @author	Zack Tzeng
 * @version	0.0
 * @since	2014-03-01
 */
class Model {

private:
    Logger* logger = new Logger("Model");

protected:
    State* state;

    HwInterface* interface;

    /**
     * Poll data from hardware at this frequency (number of polls per second).
     * A.k.a. sampling rate.
     */
    double pollFrequency;

    bool executing;

    /**
     * A separate thread used to poll the interfaces for new data
     */
    std::thread pollThread;

    std::vector<FilterManager*> filterManagerList;

    /**
     * This creates a filter manager and store it in the provided destination.
     * @param	fmID		desired ID for the new filter manager
     * @param	fmType		type of the new filter manager object, 1 for automatic IDing and 0 for default.
     * @return				pointer to the new FilterManager object
     */
    FilterManager* createFM(std::string fmID, int fmType);

    /**
     * This stores a new filter manager to the filter manager list.
     * @param	newFM	the pointer to the new filter manager
     */
    void storeToFMList(FilterManager* newFM);

    /**
     * This function will call the dataTransfer function at set intervals
     * based on the pollFrequency variable
     */
    virtual void pollLoop();

public:

    /**
     * This is the constructor to a parent Model object.
     * @param	inputState	a state pointer used to create a Model
     * @param	inputHwInterface	an interface pointer used to create a Model
     * @param   frequency   set polling frequency for the Model
     */
    Model(State* inputState, HwInterface* inputHwInterface, double frequency);

    /**
     * Model destructor
     */
    virtual ~Model();

    /**
     * Initializes the Model variables and starts the pollLoop thread
     */
    void initialize();


/* **************** Data Management **************** */

    virtual void sendCommand(Attributes attr, int value) = 0;

    virtual std::vector<Data*> constructDataSet(Data* rawData) = 0;

    virtual void storeToState(std::vector<Data*> dataSet) = 0;

    /**
     * Returns a pointer to the Model's state
     * @return the Model's State pointer
     */
    virtual State* getState();

    virtual Data* getStateData(std::string data_ID) = 0;

    /**
     * Called at set intervals to poll the Interface and store
     * the new Data into State
     */
    virtual void dataTransfer();


/* **************** Filter/Filter Chain Management **************** */

    /**
     * This inserts a filter to the end of the filter chain.
     * @param	fmID			ID of the desired filter manager for filter insertion
     * @param 	newFilter		the pointer to a filter object.
     * @param 	newFilterID 	is the unique identifier chosen by the owner of the filter.
     * @return 					0 for success, 2 for ID is not unique.
     */
    int insertFilter(std::string fmID, Filter* newFilter, std::string newFilterID);

    /**
     * This inserts a filter to the target location.
     * @param	fmID			ID of the desired filter manager for filter insertion
     * @param	newFIlter		a pointer to the new filter object
     * @param	newFilterID		ID of the new filter object
     * @param	targetID		the target location of the new filter
     * 								- It can be a filter ID then new filter is inserted before it.
     * 								- It can be BEGIN or END then new filter is inserted at the beginning or the end of the chain.
     * @return 					0 for success, 2 for ID is not unique.
     */
    int insertFilter(std::string fmID, Filter* newFilter, std::string newFilterID, std::string targetID);

    /**
     * This replace an existing filter by a new filter.
     * @param	fmID			ID of the desired filter manager for filter replacement
     * @param	newFIlter		a pointer to the new filter object
     * @param	newFilterID		the ID of the new filter object
     * @param	targetID		ID of the existing filter to be replaced
     * @return 					0 for success, 1 for targetID not found, 2 for ID not unique.
     */
    int replaceFilter(std::string fmID, Filter* newFilter, std::string newFilterID, std::string targetID);

    /**
     * This delete one or all of the filter in the filter chain.
     * @param	fmID		ID of the desired filter manager for filter deletion
     * @param	targetID	ID of the filter to be deleted
     * @return 				0 for success, 1 for targetID not found.
     */
    int deleteFilter(std::string fmID, std::string targetID);

    /**
     * Deletes all filters in filterChain.
     */
    void deleteFilterChain(std::string fmID);

    /**
     * This return the length of the filter chain.
     * @param	fmID	ID of the filter manager to look for the length of the filter chain
     * @return			length of the filter chain
     */
    int getFilterChainSize(std::string fmID);

    /**
     * This returns the vector of all the filter IDs in the chain.
     * @param	fmID	ID of the filter manager
     * @return			vector containing all filter ID strings
     */
    std::vector<std::string> getFilterChainIDs(std::string fmID);


/* ********* Filter Manager / Filter Manager List Management ********* */

    /**
     * This returns a vector containing all filter manager ID strings
     * @return	the vector of all filter manager ID strings
     */
    std::vector<std::string> getFMListIDs();

    /**
     * This creates a new filter manager and store it to the list.
     * @param	fmID		desired ID for the new filter manager
     * @param	fmType		type of the new filter manager object, 1 for automatic IDing and 0 for default.
     */
    void createAndStoreFM(std::string fmID, int fmType);

    /**
     * This deletes one filter manager on the list.
     * @param	fmID	ID of the filter manager to be deleted
     */
    void deleteFM(std::string fmID);

    /**
     * This deletes all the filter managers from the list.
     */
    void deleteFMList();

    /**
     * This return the pointer to a filter manager object on the list by its ID.
     * @param	fmID	ID of the target filter manager
     * @return			pointer to the desired filter manager
     */
    FilterManager* getFM(std::string fmID);
};

#endif /* MODEL_H_ */
