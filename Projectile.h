//
// Created by daniel on 5/08/16.
//

#ifndef TEST_SERVER_PROJECTILE_H
#define TEST_SERVER_PROJECTILE_H

#include <SFML/Graphics.hpp>
#include "Entity.h"

static int16_t nextProjectileId;

class Projectile : public Entity
{
    //sf::Vector2f position;
    sf::Vector2f origin;
    sf::Vector2f velocity;
    float speed = 500;
    float angle = 0;
    float range = 400;
    sf::RectangleShape rect;


    public:
        int16_t playerId;
        int16_t projectileId;
        bool valid;
        bool acked;



        Projectile(sf::Vector2f position, float speed, float angle, float range, int16_t projectileType, int16_t playerId);
        void update(sf::Time elapsedTime);
        int serialize(char *buffer, int position);

        int16_t projectileType;

    void intersectedWith(Entity *other, sf::FloatRect intersection);
};


#endif //TEST_SERVER_PROJECTILE_H
