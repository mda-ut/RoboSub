#include "SimFPGA.h"

InputHandler::InputHandler(bool u){
    for (irr::u32 i=0; i<irr::KEY_KEY_CODES_COUNT; ++i)
        KeyIsDown[i] = false;

    acc = irr::core::vector3df(0,0,0);
    rot = irr::core::vector3df(0,0,0);
    useKey = u;
}

void InputHandler::update(irr::f32 dt, irr::core::vector3df dir){
    acc.X = 0;
    acc.Y = 0;
    acc.Z = 0;
    bool keyDown = IsKeyDown(irr::KEY_KEY_W) || IsKeyDown(irr::KEY_KEY_A) ||
            IsKeyDown(irr::KEY_KEY_S) || IsKeyDown(irr::KEY_KEY_D) ||
            IsKeyDown(irr::KEY_KEY_Q) || IsKeyDown(irr::KEY_KEY_E) ||
            IsKeyDown(irr::KEY_SPACE) || IsKeyDown(irr::KEY_LSHIFT);
    if (useKey && keyDown){
        //input processing
        irr::core::vector3df forward;
        forward.X = -cos(dir.Y*3.141589f/180.0f);
        if (fabs(dir.Y) > 0){
            float dZ = sin(dir.Y*3.141589f/180.0f);
            forward.Z = dZ;
        }
        forward.normalize();
        if(IsKeyDown(irr::KEY_KEY_W)){
            acc = forward*5;
        } else if(IsKeyDown(irr::KEY_KEY_S)) {
            acc = -forward*5;
        }

        if(IsKeyDown(irr::KEY_KEY_A)) {
            acc.X = -forward.Z*5;
            acc.Z = forward.X*5;
        } else if(IsKeyDown(irr::KEY_KEY_D)) {
            acc.X = forward.Z*5;
            acc.Z = -forward.X*5;
        }
        if (IsKeyDown(irr::KEY_SPACE))
            acc.Y = 5 ;
        else if (IsKeyDown(irr::KEY_LSHIFT))
            acc.Y = -5;

        if (IsKeyDown(irr::KEY_KEY_Q)){
            rot.Y += -50*dt;
        }else if (IsKeyDown(irr::KEY_KEY_E)){
            rot.Y += 50*dt;
        }
    }else{
        irr::core::vector3df forward;
        forward.X = -cos(dir.Y*3.141589f/180.0f);
        if (fabs(dir.Y) > 0){
            float dZ = sin(dir.Y*3.141589f/180.0f);
            forward.Z = dZ;
        }
        forward.normalize();

        const float maxAcc = 3;
        if (targetVel.getLengthSQ() == 0){
            acc *= 0;
        }
        else if (vel.getLengthSQ() < targetVel.getLengthSQ()){
            if (acc.getLengthSQ() == 0){
                acc.X = targetVel.X;
                acc.Y = targetVel.Y;
                acc.Z = targetVel.Z;
            }
                acc.normalize();
                acc *= maxAcc;
        }

        acc = forward * acc.getLength();
    }

}

void InputHandler::setDepth(float targetDepth){
    this->targetDepth = 500 - targetDepth;
    if (this->targetDepth < 150){
        this->targetDepth = 150;
    }
}

float InputHandler::getDepth (){
    return targetDepth;
}

void InputHandler::setTargetSpeed(irr::core::vector3df target){
    if (!target.equals(targetVel)){
        targetVel = target;
    }
}
void InputHandler::setTargetSpeed(float x, float y, float z){
    irr::core::vector3df temp(x, y, z);
    setTargetSpeed(temp);
}

void InputHandler::setCurrentVel(irr::core::vector3df vel){
    this->vel = vel;
}

void InputHandler::setRot(irr::core::vector3df r){
    rot = r;
}
void InputHandler::setRot(float x, float y, float z){
    irr::core::vector3df temp(x, y, z);
    setRot(temp);
}

irr::core::vector3df InputHandler::getAcc(){
    return acc;
}
irr::core::vector3df InputHandler::getRot(){
    return rot;
}
