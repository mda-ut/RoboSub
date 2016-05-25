#include "SimObject.h"

SimObject::SimObject(std::string name, irr::scene::ISceneNode *n)
{
    this->name = name;
    node = n;
    n->setMaterialFlag(irr::video::EMF_LIGHTING, false);

    fri.X = -1;
    fri.Y = -1;
    fri.Z = -1;
}

void SimObject::update(float dt){
    //Note: when working with acceleration, multiply it by dt (delta time)
    //This is needed because game engine applies phhysics based on how long it took to render the frame

    //rotation----------------------------------------------------
    irr::core::vector3df rot = node->getRotation();

    //position----------------------------------------------------
    irr::core::vector3df pos = node->getPosition();

    //if the velocity vector is > 0, then subject it to friction
    if (vel.getLengthSQ() > 0){

        //if the velocity + current acceleration is less than friction
        if (fabs(vel.X+acc.X*dt) < friction*dt){
            //stop it from moving (due to friction)
            vel.X = 0;
            acc.X = 0;
        }
        //if not and velocity of current dimension is > 0
        else if (fabs(vel.X) > 0){
            //apply friction to it
            //float dirFri = std::copysign(friction*dt, vel.X);
            float frictionDir = friction*dt * (fabs(vel.X)/vel.X);
            vel.X -= frictionDir;
        }

        if (fabs(vel.Y+acc.Y*dt) < friction*dt){
            vel.Y = 0;
            acc.Y = 0;
        }else if (fabs(vel.Y) > 0){
            float dirFri = std::copysign(friction*dt, vel.Y);
            vel.Y -= dirFri;
        }

        if (fabs(vel.Z+acc.Z*dt) < friction*dt){
            vel.Z = 0;
            acc.Z = 0;
        }else if (fabs(vel.Z) > 0){
            //float dirFri = std::copysign(friction*dt, vel.Z);
            float frictionDir = friction*dt * (fabs(vel.Z)/vel.Z);
            vel.Z -= frictionDir;
        }
    }

    //if the velocity is greater than terminal velocity(hard coded to 5 atm)
    if (vel.getLength() > 5){
        //stop it from going any faster
        vel = vel.normalize()*5;
    }
    vel += acc*dt;
    pos += vel;

    //Logger::Log("Acc " + std::to_string(acc.Z*dt));
    //Logger::Log("Vel " + std::to_string(vel.Z));

    node->setPosition(pos);
}

void SimObject::reset(){
    this->acc.X = 0;
    this->acc.Y = 0;
    this->acc.Z = 0;
    this->vel.X = 0;
    this->vel.Y = 0;
    this->vel.Z = 0;
    node->setPosition(irr::core::vector3df(0,0,0));
}

void SimObject::setAcc(irr::core::vector3df a){
    this->acc.X = a.X;
    this->acc.Y = a.Y;
    this->acc.Z = a.Z;
    std::string msg = "Setting Acc: " + std::to_string(acc.X) + ' ' + std::to_string(acc.Y) + ' ' + std::to_string(acc.Z);
    //SimLogger::Log(msg);
}
void SimObject::setRot(irr::core::vector3df r){
    node->setRotation(r);
}

irr::core::vector3df SimObject::getAcc(){
    return acc;
}
irr::core::vector3df SimObject::getVel(){
    return vel;
}
irr::core::vector3df SimObject::getPos(){
    return node->getPosition();
}
irr::core::vector3df SimObject::getRot(){
    return node->getRotation();
}

std::string SimObject::getName(){
    return name;
}
