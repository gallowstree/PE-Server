//
// Created by Alejandro on 30/07/16.
//

#ifndef TEST_SERVER_PLAYER_H
#define TEST_SERVER_PLAYER_H


#include <SFML/Graphics.hpp>
#include <sys/socket.h>
#include <netinet/in.h>


class Player
{
public:
    const int playerId;
    int lastMsgNum;
    int lastBulletFrame = 0;
    const int port;
    const char* ip;
    sf::Vector2f position;
    sf::Vector2f velocity;
    float speed = 500;
    int controls;


    Player(int playerId, const char* ip, int port, sf::Vector2f position);
    void send(const char* outbuffer, size_t size);
    void update(sf::Time elapsedTime);
private:
    int udpSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;



};


#endif //TEST_SERVER_PLAYER_H
