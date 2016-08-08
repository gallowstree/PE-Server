//
// Created by daniel on 5/08/16.
//

#ifndef TEST_SERVER_PROJECTILE_H
#define TEST_SERVER_PROJECTILE_H

#include <SFML/Graphics.hpp>

class Projectile {
    sf::Vector2f position;
    sf::Vector2f velocity ;
    float speed = 500;
    sf::RectangleShape projectile;

    public:
        Projectile(sf::Vector2f position, float speed);
};


#endif //TEST_SERVER_PROJECTILE_H
