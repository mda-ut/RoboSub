/*
 * Data.cpp
 *
 *  Created on: Mar 28, 2015
 *      Author: ahsueh1996
 */

#include "Data.h"

/* ==========================================================================
 * FUNC ACCESSIBLE BY CHILDREN
 * ==========================================================================
 */

void Data::track(std::string task, int error, int type) {
	if (type == 0)
		this->tracker += task + "; ";
	else
        this->tracker += task + "-" + std::to_string(error) + "; ";
}

void Data::resetTracker() {
	this->tracker = "";
}

/* ==========================================================================
 * CONSTRUCTOR & DESTRUCTOR
 * ==========================================================================
 */

Data::Data(std::string dataID) {
	this->dataID = dataID;
	this->msg = "";
	this->tracker = "";
}


Data::~Data() {
}

/* ==========================================================================
 * PUBLIC FUNCS COMMON TO ALL CHILDREN
 * ==========================================================================
 */

std::string Data::getID(){
	return this->dataID;
}

std::string Data::getMsg(){
	return this->msg;
}

void Data::setID(std::string newID) {
	this->dataID = newID;
}


int Data::setMsg(std::string newMsg){
	if (this->msg != "")
	{
		this->msg = newMsg;
		return 1;
	}
	this->msg = newMsg;
	return 0;
}

void Data::addMsg(std::string newMsg){
	if (this->msg == "")
		this->setMsg(newMsg);
	else
		this->msg += "||"+newMsg;
}

std::string Data::getTrack() {
	return this->tracker;
}

