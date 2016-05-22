/*
 * PowerTask.h
 *
 *  Created on: Jul 3, 2015
 *      Author: jwong
 */

#ifndef POWERTASK_H_
#define POWERTASK_H_

#include "Task.h"
#include "Logger.h"

class PowerTask : public Task {
private:
    Logger* logger = new Logger("PowerTask");
    Model* fpgaModel;

public:
    /**
     * Contructor
     */
    PowerTask(Model* fpgaModel);

    void execute();

    /**
     * Destructor
     */
    virtual ~PowerTask();

};

#endif /* SUBMARINE_SRC_CONTROLLER_TASK_POWERTASK_H_ */
