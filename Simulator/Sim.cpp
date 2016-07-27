#include "Sim.h"
#include <unistd.h>


//Sim::Sim(cv::Mat* frame, InputHandler* in, bool* r, bool* ta)
Sim::Sim(void* c)
{
    cap = (dataCap*) c;
    this->frontFrame = cap->frontSC->getFrame();
    this->downFrame = cap->downSc->getFrame();
    this->ih = cap->ih;
    this->runSim = cap->runSim;
    this->threadAlive = cap->threadAlive;
    /**
     * Setup Irrlicht stuff
     */
    //device = createDevice( video::EDT_OPENGL, dimension2d<u32>(1280, 960), 16,
      //      false, true, false, &ih);
    SIrrlichtCreationParameters params = SIrrlichtCreationParameters();
    //params.AntiAlias = 8;
    params.DriverType = video::EDT_OPENGL;
    params.WindowSize = core::dimension2d<u32>(640, 480);
    params.EventReceiver = ih;
    device = createDeviceEx(params);
    if (!device){
        SimLogger::Log("FATAL- Could not create device");
        return;
    }
    device->setWindowCaption(L"MDA Simulator 1.0");

    driver = device->getVideoDriver();
    smgr = device->getSceneManager();
    guienv = device->getGUIEnvironment();

    /**
     * Load textures into the map
     */
    DataStorage::loadTextures(driver);

    /**
     * Loads the nodes into the scene
     */
    ISceneNode *b = smgr->addSphereSceneNode();
    if (!b){
        SimLogger::Log("Could not create sphere node");
    }else{
        Buoy *ball = new Buoy("ball", b);
        objs.push_back(ball);
    }

    ISceneNode *s = smgr->addCubeSceneNode();
    SimSub *simSub = new SimSub("SimSub", s, ih);
    objs.push_back(simSub);

    //Light and Fog
    ILightSceneNode* light1 = smgr->addLightSceneNode( 0, core::vector3df(0,500,0), video::SColorf(0.3f,0.3f,0.3f), 50000.0f, 1 );
    driver->setFog(video::SColor(0, 120,140,160), video::EFT_FOG_LINEAR, 20, 250, .001f, false, false);
    ISceneNode * scenenode = smgr->getRootSceneNode();
    scenenode->setMaterialFlag(EMF_FOG_ENABLE, true);
    light1->setMaterialFlag(EMF_FOG_ENABLE, true);

    //Load obstacles (need to be separated so that buoys can move)
    IAnimatedMesh* obsMesh = smgr->getMesh("assets/obstacles.3ds");
    IMeshSceneNode * obstacles = 0;
    scene::ITriangleSelector* obsSelector = 0;
    if (obsMesh) {
        obstacles = smgr->addOctreeSceneNode(obsMesh->getMesh(0), 0);
        obstacles->setMaterialFlag(EMF_FOG_ENABLE, true);
        obstacles->setMaterialFlag(EMF_LIGHTING, true);
        obstacles->setScale(core::vector3df(20,20,20));
        obstacles->setPosition(core::vector3df(0,0,0));
        obsSelector = smgr->createOctreeTriangleSelector(
                obstacles->getMesh(), obstacles, 128);
        obstacles->setTriangleSelector(obsSelector);
    }

    IAnimatedMesh* roomMesh = smgr->getMesh("assets/stadium.3ds");
    IMeshSceneNode * roomNode = 0;
    scene::ITriangleSelector* roomSelector = 0;
    if (roomMesh) {
        roomNode = smgr->addOctreeSceneNode(roomMesh->getMesh(0), 0);
        roomNode->setMaterialFlag(EMF_FOG_ENABLE, true);
        roomNode->setMaterialFlag(EMF_LIGHTING, true);
        roomNode->setScale(core::vector3df(20,20,20));
        roomSelector = smgr->createOctreeTriangleSelector(roomNode->getMesh(),
                                                          roomNode, 128);
        roomNode->setTriangleSelector(roomSelector);
    }
    scene::IMetaTriangleSelector* selector = smgr->createMetaTriangleSelector();
    if (roomSelector && obsSelector) {
        selector->addTriangleSelector(roomSelector);
        selector->addTriangleSelector(obsSelector);
    }


    //ICameraSceneNode* camera = smgr->addCameraSceneNodeFPS(0, 100.0f, 0.05f);
    ICameraSceneNode* camera = smgr->addCameraSceneNode(s, vector3df(0,10,0), vector3df(0,0,0));
    //Logger::Log(s->getPosition());
    cameras[0] = smgr->addCameraSceneNode(s, s->getPosition());
    cameras[0]-> bindTargetAndRotation(true);
    cameras[1] = smgr->addCameraSceneNode(s);
    cameras[2] = smgr->addCameraSceneNode(s, s->getPosition(), vector3df(0,0,0));
    camChilds[1] = smgr->addEmptySceneNode(cameras[1]);
    camChilds[1]->setPosition(vector3df(0,-1,0));
    //cameras[1]->setUpVector(vector3df(-1,0,0));
    s->setPosition(vector3df(-200, 212, 443));

    //First vector input is the radius of the collidable object
    anim = smgr->createCollisionResponseAnimator(
    selector, s, vector3df(6.5f,6.5f,6.5f),
    vector3df(0,0,0), vector3df(0,0,0));

    if (selector)
    {
        selector->drop(); // As soon as we're done with the selector, drop it.
        s->addAnimator(anim);
        //camera->addAnimator(anim);
        //anim->drop();  // And likewise, drop the animator when we're done referring to it.
    }
    //node->drop();

}

//Sim::~Sim(){

//}

int Sim::start(){
    //if the device cannot be created, just exit the program
    if (!device)
        return 1;    

    guienv->addStaticText(L"Hello World!",
        rect<s32>(10,10,260,22), true);

    //Add camera node to a static position. Need to change later to have this on the sub
    cameras[3] = smgr->addCameraSceneNode(0, vector3df(0,30,-40), vector3df(0,5,0));

    bool collision;

    // In order to do framerate independent movement, we have to know
    // how long it was since the last frame
    u32 then = device->getTimer()->getTime();

    while(device->run() && (bool)cap->runSim)
    {
        // Work out a frame delta time.
        const u32 now = device->getTimer()->getTime();
        const f32 frameDeltaTime = (f32)(now - then) / 1000.f; // Time in seconds
        then = now;


        //Logger::Log("FDT");
        //Logger::Log(frameDeltaTime);
        for (SimObject *so: objs){

            if (so->getName() == "SimSub"){
                SimSub *ss = (SimSub*) so;
                ih->setCurrentVel(ss->getVel());
                ih->update(frameDeltaTime, ss->getRot());

                ss->setRot(ih->getRot());
                //Logger::Log(so->getRot());
                ss->setAcc(ih->getAcc());
                ss->update(frameDeltaTime);

                if (ih->IsKeyDown(irr::KEY_KEY_R)){
                   ss->reset();
                }

                ///offsets for camera stuff
                //vector3df temp = so->getPos();
                vector3df tempPos, tempDir;

                tempDir.X = -cos(ss->getRot().Y*3.141589f/180.0f);
                if (fabs(ss->getRot().Y) > 0){
                    float dZ = sin(ss->getRot().Y*3.141589f/180.0f);
                    tempDir.Z = dZ;
                    //Logger::Log(dZ);
                }
                tempDir.normalize();
                tempPos = ss->getPos();

                //camera attached to sub
                //this call sets the position relative to sub
                cameras[0]->setPosition(tempDir*5);
                cameras[0]->setTarget(tempPos + tempDir*100);
                //cameras[0]->setRotation(so->getRot());
                //cameras[0]->setRotation(vector3df(0,0,0));
                irr::core::vector3df tempBottom;
                tempBottom = ss->getPos();
                tempBottom.Y -= 10;
                tempBottom.X += tempDir.X/2;
                tempBottom.Z += tempDir.Z/2;

                cameras[1]->setTarget(tempBottom);

                //temp = so->getPos();
                //temp.Z += 20;
                cameras[2]->setTarget(tempPos);

                //camera 3 not attached to sub so need to add tempPos to the position var
                vector3df tempTP; //third person view cam

                tempTP.X = tempPos.X - tempDir.X*30;
                tempTP.Z = tempPos.Z - tempDir.Z*30;
                tempTP.Y = tempPos.Y + 20;
                cameras[3]->setPosition(tempTP);
                cameras[3]->setTarget(tempPos);
                //Logger::Log(std::to_string(so->getAcc().X));
            } else {
                so->update(frameDeltaTime);
            }

            //ih->setAcc();
        }

        driver->setViewPort(rect<s32>(0,0,resX, resY));
        driver->beginScene(true, true, SColor(255,100,101,140));

        smgr->setActiveCamera(cameras[0]);
        driver->setViewPort(rect<s32>(0,0,resX/2,resY/2));
        smgr->drawAll();
        smgr->setActiveCamera(cameras[1]);
        driver->setViewPort(rect<s32>(resX/2,0,resX,resY/2));
        smgr->drawAll();
        smgr->setActiveCamera(cameras[2]);
        driver->setViewPort(rect<s32>(0,resY/2,resX/2,resY));
        //smgr->drawAll();
        smgr->setActiveCamera(cameras[3]);
        driver->setViewPort(rect<s32>(resX/2,resY/2,resX,resY));
        smgr->drawAll();
        guienv->drawAll();

        driver->endScene();

        //convert Irrlicht render into OpenCV Mat
        IImage* image = driver->createScreenShot();
        for(int y = 0; y < sizeY; y++){
            for(int x = 0; x < sizeX; x++){
                SColor color = image->getPixel(x, y).color;
                if (color.getBlue()+150 > 255)
                    color.setBlue(255);
                else
                    color.setBlue(color.getBlue()+150);
                cv::Vec3b CVColor(color.getRed(), color.getGreen(), color.getBlue());
                frontFrame->at<cv::Vec3b>(y,x) = CVColor;
            }
            for (int x = sizeX; x < sizeX*2; x++){
                SColor color = image->getPixel(x, y).color;
                /*
                if (color.getBlue()+150 > 255)
                    color.setBlue(255);
                else
                    color.setBlue(color.getBlue()+150);*/
                cv::Vec3b CVColor(color.getBlue(), color.getGreen(), color.getRed());
                downFrame->at<cv::Vec3b>(y,x-sizeX) = CVColor;
            }
        }
//        cv::imshow("front", *frontFrame);
//        cv::imshow("down", *downFrame);
//        cv::waitKey(1);
        delete image;
        usleep(60000);

    } //end of while loop

    smgr->drop();
    device->closeDevice();
    cap->threadAlive = (bool*)false;
    return 0;
}
