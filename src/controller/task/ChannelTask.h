
/*
 * ChannelTask.h
 *
 *  Created on: Jul 28, 2016
 *      Author: jwong
 */

#ifndef CHANNELTASK_H
#define CHANNELTASK_H


#include "Task.h"
#include "DepthTask.h"
#include "TurnTask.h"
#include "SpeedTask.h"
#include "Logger.h"
#include <unistd.h>

class ChannelTask : public Task
{
private:
    Logger* logger = new Logger("ChannelTask");
    Model* fpgaModel;
    DepthTask* depthTask;
    TurnTask* turnTask;
    SpeedTask* speedTask;

    void rotate(float angle);
public:
    /**
     * Contructor
     */
    ChannelTask(Model* fpgaModel, DepthTask* depthTask, TurnTask* turnTask, SpeedTask* speedTask);

    void execute();

    /**
     * Destructor
     */
    virtual ~ChannelTask();

};

#endif // CHANNEL_H



