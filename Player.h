//
// Created by Alejandro on 30/07/16.
//

#ifndef TEST_SERVER_PLAYER_H
#define TEST_SERVER_PLAYER_H


#include <SFML/Graphics.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Projectile.h"
#include "reliable_message.h"
#include "Entity.h"
#include "OutputSocket.h"


class Player : public Entity
{
public:
    //Socket
    const char* ip;

    //Connection data
    const int16_t playerId;
    std::map<int32_t, reliable_message_t> reliable_queue;
    bool hasNotAckedId = true;

    //Only High bits used
    int16_t playerInfo;

    //Physics
    float speed;
    float rotation;
    float cross_thickness;
    sf::FloatRect vert_rect;
    sf::FloatRect horz_rect;

    int health;

    //Events
    int controls = 0;

    //Weapons
    std::vector<Projectile> projectiles;

    Player(int16_t playerId, sf::Vector2f position, OutputSocket socket, int team);
    void send(const char* outbuffer, size_t size, int32_t reliableId = -1);
    void update(sf::Time elapsedTime);

    int getTeam();

    int serialize(char *buffer, int position);
    void intersectedWith(Entity *other, sf::FloatRect intersection);

private:

    sf::Time timeSinceLastShot;

    void updateMovement(sf::Time elapsedTime);

    void updateProjectiles(sf::Time elapsedTime);

    void updateCross();

    OutputSocket socket;
};


#endif //TEST_SERVER_PLAYER_H
