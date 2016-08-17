//
// Created by Alejandro on 13/08/16.
//

#include "Entity.h"

int16_t Entity::lastId;

Entity::Entity() :
boundingBox(),
id(-1)
{
    id = Entity::lastId++;
}

void Entity::intersectedWith(Entity *other, sf::FloatRect intersection)
{

}