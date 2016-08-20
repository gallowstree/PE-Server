//
// Created by Alejandro on 13/08/16.
//

#include "Entity.h"

int16_t Entity::lastId;

Entity::Entity() :
boundingBox(),
entityId(-1)
{
    entityId = Entity::lastId++;
}

void Entity::intersectedWith(Entity *other, sf::FloatRect intersection)
{

}