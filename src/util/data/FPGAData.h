/*
 * FPGAData.h
 *
 *  Created on: Mar 29, 2015
 *      Author: ahsueh1996
 */

#ifndef FPGADATA_H_
#define FPGADATA_H_
#include "Data.h"

/**
 * Wrapper class for data from the FPGA. This class will store
 * FPGA data in 3 ints:
 * power - 0 if sub is off, 1 if sub is on
 * yaw - the angle the sub is facing, in degrees
 * depth - the depth of the sub in cm, value increases as sub moves down
 *
 * This class will allow Controller to get the values and perform
 * algorithm off them. It will not serve as a relay to send
 * commands to the FPGA thus, much like ImgData, the setters
 * are reserved for privileged classes.
 */
class FPGAData: public Data {

	/*	=========================================================================
	 *	FRIEND CLASSES
	 *	=========================================================================
	 */
	friend class Filter;
	friend class HwInterface;

private:

	/* ==========================================================================
	 * CLASS VARS
	 * ==========================================================================
	 */

	/*
     * power, yaw, depth
	 */
    int power, yaw, depth;
public:

	/* ==========================================================================
	 * CONSTRUCTOR & DESTRUCTOR
	 * ==========================================================================
	 */

	/**
	 * Constructor for FPGAData.
	 *
     * @param dataID
     * @param power
     * @param yaw
     * @param depth
	 */
    FPGAData(std::string dataID, int power, int yaw, int depth);

	/**
	 * Destructor stub.
	 */
	virtual ~FPGAData();

	/* ==========================================================================
     * FPGA GETTER FUNCS
	 * ==========================================================================
	 */

	/**
     * Getter for the power status.
	 *
     * @return 	int value. 1 if power on, 0 if power off
	 */
    int getPower();

	/**
     * Getter for the yaw
	 *
     * @return 	int value of yaw, in degrees
	 */
    int getYaw();

    /**
     * Getter for depth
	 *
     * @return 	int value of depth, in "cm"
	 */
    int getDepth();

};

#endif /* FPGADATA_H_ */
