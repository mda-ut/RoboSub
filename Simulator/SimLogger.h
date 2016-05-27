#ifndef LOGGER_H
#define LOGGER_H
#include <string>
#include <iostream>
#include "irrlicht.h"

class SimLogger
{
public:

    /*
     * Logs the message
     * TODO add priority, might copy jon's logger
     */
    static void Log(std::string);
    static void Log(float);
    static void Log(irr::core::vector3df);
    static void Log(std::string, float);
    static void Log(std::string, irr::core::vector3df);
};

#endif // LOGGER_H
