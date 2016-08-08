//
// Created by Alejandro on 30/07/16.
//

#include <arpa/inet.h>
#include "Player.h"
#include <string.h>



Player::Player(int playerId, const char* ip, int port, sf::Vector2f position) :
playerId(playerId),
position(position),
lastMsgNum(0),
ip(ip),
velocity(sf::Vector2f(0.0f, 0.0f)),
port(port)
{
    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*Bind socket with address struct*/
    //bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverAddr;
}

void Player::send(const char *outbuffer, size_t size)
{
    sendto(udpSocket,outbuffer,size,0,(struct sockaddr *)&serverAddr, addr_size);
    int i = errno;
    //printf("Sending %s!\n",strerror(errno));
}

void Player::update(sf::Time elapsedTime)
{
    sf::Vector2f movement (0,0);
    if (controls & 0x1)
        movement.y -= speed;
    if (controls & 0x2)
        movement.y += speed;
    if (controls & 0x4)
        movement.x -= speed;
    if (controls & 0x8)
        movement.x += speed;

    position += movement * elapsedTime.asSeconds();
}





