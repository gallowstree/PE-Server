//
// Created by daniel on 5/08/16.
//

#include "Projectile.h"
#include "serialization.h"
#include <math.h>

Projectile::Projectile(sf::Vector2f position, float speed, float angle, float range, int16_t type):
position(position),
speed(speed),
velocity(sf::Vector2f(speed * cosf(angle), speed * sinf(angle))),
rect(sf::Vector2f(6,6)),
range(range),
origin(position),
valid(true),
projectileId(nextProjectileId),
projectileType(type)
{
    nextProjectileId++;
    update(sf::seconds(0.07));
}

void Projectile::update(sf::Time elapsedTime)
{
    this->position += velocity * elapsedTime.asSeconds();
    sf::Vector2f distance = position - origin;

    if (sqrtf(distance.x*distance.x + distance.y*distance.y) >= range)
    {
        this->valid = false;
    }
}

int Projectile::serialize(char * buffer, int position)
{
    int pos = position;
    Serialization::shortToChars(this->projectileId, buffer, pos);
    pos += 2;
    Serialization::shortToChars(this->projectileType, buffer, pos);
    pos += 2;
    Serialization::floatToChars(this->position.x, buffer, pos);
    pos += 4;
    Serialization::floatToChars(this->position.y, buffer, pos);
    pos += 4;
    Serialization::floatToChars(this->origin.x, buffer, pos);
    pos += 4;
    Serialization::floatToChars(this->origin.y, buffer, pos);
    pos += 4;

    return pos - position;
}

