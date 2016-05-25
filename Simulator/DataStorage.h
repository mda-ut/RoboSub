#ifndef DATASTORAGE_H
#define DATASTORAGE_H

#include <map>
#include <string>
#include "irrlicht.h"

/*
 * Storage class for the various stuff that the program will need
 */
class DataStorage
{
public:
    DataStorage();
    static std::map<std::string, irr::video::ITexture*> textures;

    /*
     * Load all the required textures into the texture map
     * The IVideoDriver is needed to load the textures
     */
    static void loadTextures(irr::video::IVideoDriver*);
};

#endif // DATASTORAGE_H
