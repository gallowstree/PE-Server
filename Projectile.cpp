//
// Created by daniel on 5/08/16.
//

#include "Projectile.h"
#include "serialization.h"
#include <math.h>

Projectile::Projectile(sf::Vector2f position, float speed, float angle, float range):
position(position),
speed(speed),
velocity(sf::Vector2f(speed * cosf(angle), speed * sinf(angle))),
rect(sf::Vector2f(6,6)),
range(range),
origin(position),
valid(true)
{

}

void Projectile::update(sf::Time elapsedTime)
{
    position += velocity * elapsedTime.asSeconds();

    printf("position: %f, %f\n", position.x, position.y);

    sf::Vector2f distance = position - origin;

    if (sqrtf(distance.x*distance.x + distance.y*distance.y) >= range)
    {
        this->valid = false;
        printf("invalid!!!!!!!!!!!!\n");
    }
}

int Projectile::serialize(char * buffer, int position)
{
    int pos = position;
    Serialization::shortToChars(this->playerId, buffer, pos); //Player id 6 - 7
    pos += 2;
    Serialization::floatToChars(this->position.x, buffer, pos); //Pos x 8 - 12
    pos += 4;
    Serialization::floatToChars(this->position.y, buffer, pos); //Pos y 13 - 17
    pos += 4;
    Serialization::floatToChars(this->angle, buffer, pos); //Pos y 13 - 17
    pos += 4;

    return pos - position;
}

