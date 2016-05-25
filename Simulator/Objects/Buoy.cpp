#include "Buoy.h"

Buoy::Buoy(std::string name, irr::scene::ISceneNode* n):SimObject(name, n)
{
    node->setMaterialTexture(0, DataStorage::textures["wall.bmp"]);
}
