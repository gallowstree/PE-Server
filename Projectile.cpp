//
// Created by daniel on 5/08/16.
//

#include "Projectile.h"
#include "serialization.h"
#include <math.h>

Projectile::Projectile(sf::Vector2f position, float speed, float angle, float range, int16_t projectileType, int16_t playerId):
speed(speed),
velocity(sf::Vector2f(speed * cosf(angle), speed * sinf(angle))),
rect(sf::Vector2f(6,6)),
range(range),
origin(position),
valid(true),
acked(false),
projectileId(nextProjectileId),
playerId(playerId),
projectileType(projectileType)
{
    this->type = EntityType ::Projectile_T;
    nextProjectileId++;
    boundingBox = BoundingBox(position.x, position.y, 6, 6);
    update(sf::seconds(0.01));
}

void Projectile::update(sf::Time elapsedTime)
{
    if (!this->valid) return;
    auto movement = velocity * elapsedTime.asSeconds();
    this->boundingBox.left += movement.x;
    this->boundingBox.top += movement.y;

    sf::Vector2f distance = boundingBox.getPosition() - origin;

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
    Serialization::floatToChars(this->boundingBox.left, buffer, pos);
    pos += 4;
    Serialization::floatToChars(this->boundingBox.top, buffer, pos);
    pos += 4;
    Serialization::floatToChars(this->origin.x, buffer, pos);
    pos += 4;
    Serialization::floatToChars(this->origin.y, buffer, pos);
    pos += 4;
    Serialization::shortToChars(this->playerId, buffer, pos);
    pos += 2;


    return pos - position;
}

void Projectile::intersectedWith(Entity *other, sf::FloatRect intersection)
{
    valid = false;
}

int16_t Projectile::getPower() {
    return 25;
}


