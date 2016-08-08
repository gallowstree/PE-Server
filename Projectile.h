//
// Created by daniel on 5/08/16.
//

#ifndef TEST_SERVER_PROJECTILE_H
#define TEST_SERVER_PROJECTILE_H

#include <SFML/Graphics.hpp>

class Projectile {
    sf::Vector2f position;
    sf::Vector2f origin;
    sf::Vector2f velocity;
    float speed = 500;
    float angle = 0;
    float range = 200;
    sf::RectangleShape rect;


    public:
        int16_t playerId;
        Projectile(sf::Vector2f position, float speed, float angle, float range);
        void update(sf::Time elapsedTime);

    int serialize(char *buffer, int position);
    bool valid;
};


#endif //TEST_SERVER_PROJECTILE_H
