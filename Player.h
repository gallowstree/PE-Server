//
// Created by Alejandro on 30/07/16.
//

#ifndef TEST_SERVER_PLAYER_H
#define TEST_SERVER_PLAYER_H


#include <SFML/Graphics.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Projectile.h"


class Player
{
public:
    //Socket
    const int port;
    const char* ip;

    //Connection data
    const int16_t playerId;
    int lastMsgNum;
    

    //Physics
    float speed = 500;
    float rotation = 0;
    sf::Vector2f position;
    sf::Vector2f velocity;

    //Events
    int controls;

    //Weapons
    std::vector<Projectile> projectiles;

    Player(int16_t playerId, const char* ip, int port, sf::Vector2f position);
    void send(const char* outbuffer, size_t size);
    void update(sf::Time elapsedTime);

    int serialize(char *buffer, int position);

private:
    int udpSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
    sf::Time timeSinceLastShot;


    void initSocket();

};


#endif //TEST_SERVER_PLAYER_H