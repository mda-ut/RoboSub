#ifndef SIM_H
#define SIM_H

#include "irrlicht.h"
#include "Objects/SimObject.h"
#include "Objects/Buoy.h"
#include "Objects/SimSub.h"
#include "SimLogger.h"
#include "SimFPGA.h"
#include "DataStorage.h"
#include "SimCam.h"
#include <vector>
#include <opencv2/opencv.hpp>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

struct dataCap{
    SimCam *frontSC;
    SimCam *downSc;
    InputHandler* ih;
    bool* runSim;
    bool* threadAlive;
};

/*
 * The main class of the Simulator
 */
class Sim
{
private:
    //Irrlicht stuff
    IrrlichtDevice *device = 0;
    IVideoDriver* driver = 0;
    ISceneManager* smgr = 0;
    IGUIEnvironment* guienv = 0;
    ISceneNodeAnimatorCollisionResponse* anim = 0;

    ICameraSceneNode* cameras[4] = {0,0,0,0};
    ISceneNode* camChilds[4] = {0,0,0,0};
    static const int resX = 640;
    static const int resY = 480;

    InputHandler* ih = 0;                //handles input from user
    std::vector<SimObject*> objs;   //list of simulated objects

    //current frame in cv::Mat
    cv::Mat *frontFrame;
    cv::Mat *downFrame;
    dataCap* cap;
    bool* runSim;
    bool* threadAlive;

public:
    //Sim(cv::Mat*, InputHandler*, bool*, bool*);
    Sim(void*);

    /*
     * Starts running the simulator
     */
    int start();

    static const int sizeX = resX/2;
    static const int sizeY = resY/2;

};

#endif // SIM_H
