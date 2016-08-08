//
// Created by daniel on 5/08/16.
//

#include "Projectile.h"

Projectile::Projectile(sf::Vector2f position, float speed):
position(position),
velocity(sf::Vector2f(0.0f, 0.0f)),
speed(speed),
projectile(sf::Vector2f(6,6))
{
    projectile.setFillColor(sf::Color(0, 255, 0));
}