//
// Created by Alejandro on 13/08/16.
//

#ifndef GAMEPLAY_PROTOTYPE_ENTITY_H
#define GAMEPLAY_PROTOTYPE_ENTITY_H


#include <stdint.h>
#include <SFML/Graphics.hpp>
#include <queue>
#include "BoundingBox.h"

enum EntityType
{
    Wall_T,
    Player_T
};


class Entity
{

public:
    static int16_t lastId;
    BoundingBox boundingBox;
    sf::FloatRect movementBounds;
    int16_t entityId;
    bool isStatic;
    EntityType type;

    Entity();

    virtual void intersectedWith(Entity* other, sf::FloatRect intersection);


};



#endif //GAMEPLAY_PROTOTYPE_ENTITY_H
