//
// Created by Alejandro on 13/08/16.
//

#include "Wall.h"

Wall::Wall(float x, float y, float width, float height)
{
    boundingBox.top = y;
    boundingBox.left = x;
    boundingBox.height = height;
    boundingBox.width = width;

    isStatic = true;

    type = EntityType::Wall_T;
}

