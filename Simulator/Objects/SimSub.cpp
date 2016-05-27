#include "SimSub.h"

SimSub::SimSub(std::string name, irr::scene::ISceneNode* n, InputHandler* ih):SimObject(name, n)
{
    this->ih = ih;
}

void SimSub::setCam(irr::scene::ICameraSceneNode *c){
    cam = c;
}

void SimSub::update(float dt)
{
    SimObject::update(dt);

    //do the target depth stuff
    //TODO: put this in the simFPGA
    if (fabs(ih->getDepth() - targetDepth) > 5){
        targetDepth = ih->getDepth();
    }
    if (targetDepth != -1 && fabs(node->getPosition().Y - targetDepth) > 3){
        irr::core::vector3df pos = node->getPosition();
        pos.Y = targetDepth;
        node->setPosition(pos);
    }
//    SimLogger::Log("Sim pos: ", node->getPosition());
}

irr::scene::ICameraSceneNode* SimSub::getSceneNode(){
    return cam;
}
