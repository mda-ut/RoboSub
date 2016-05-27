#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H
#include "irrlicht.h"
#include "SimLogger.h"

//TODO: Rename InputHandler to SimFPGA
class InputHandler : public irr::IEventReceiver
{
public:
    //TODO, MAKE ALL THIS INTO THE .CPP FILE
    InputHandler(bool);

    virtual bool OnEvent(const irr::SEvent& event){
        if (event.EventType == irr::EET_KEY_INPUT_EVENT)
            KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
        return false;
    }
    virtual bool IsKeyDown(irr::EKEY_CODE keyCode) const
    {
        return KeyIsDown[keyCode];
    }

    void update(irr::f32, irr::core::vector3df);

    bool usingKeyboard();
    void setTargetSpeed(irr::core::vector3df);
    void setTargetSpeed(float x, float y, float z);
    void setCurrentVel(irr::core::vector3df);
    irr::core::vector3df getAcc();

    //TODO: IMPLEMENT FUNCTION
    void setDepth(float targetDepth);
    float getDepth();

    void setRot(irr::core::vector3df);
    void setRot(float x, float y, float z);
    irr::core::vector3df getRot();
private:
    bool KeyIsDown[irr::KEY_KEY_CODES_COUNT];

    bool useKey;
    irr::core::vector3df acc;
    irr::core::vector3df vel;
    irr::core::vector3df rot;
    float targetDepth;
    irr::core::vector3df targetVel;
};

#endif // INPUTHANDLER_H
