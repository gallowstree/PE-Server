//
// Created by Alejandro on 30/07/16.
//

#include <arpa/inet.h>
#include "Player.h"
#include "serialization.h"
#include <string.h>



Player::Player(int16_t playerId, const char* ip, int port, sf::Vector2f position) :
playerId(playerId),
position(position),
lastMsgNum(0),
ip(ip),
velocity(sf::Vector2f(0.0f, 0.0f)),
timeSinceLastShot(sf::Time::Zero),
port(port)
{
    initSocket();

}

void Player::initSocket()
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

void Player::send(const char *outbuffer, size_t size, int32_t reliableId)
{
    sendto(udpSocket,outbuffer,size,0,(struct sockaddr *)&serverAddr, addr_size);
    int i = errno;
    //printf("Sending %s!\n",strerror(errno));

    if (reliableId != -1)
    {
        //Copiar el buffer a uno confiable
        char* copyBuffer = (char *)malloc(size);
        memcpy(copyBuffer, outbuffer, size);

        //Might need unique_ptrs
        reliable_message_t message;
        message.buffer = copyBuffer;
        message.messageId = reliableId;
        //Insertar mensaje confiable
        //reliable_queue.insert(std::pair<int32_t, reliable_message_t*>(reliableId, &message));
        reliable_queue[reliableId] = message;
    }


    for (auto const& message: reliable_queue)
    {
        if (message.first != reliableId)
        {
            sendto(udpSocket, message.second.buffer, message.second.size, 0,(struct sockaddr *)&serverAddr, addr_size);
        }
    }

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

    timeSinceLastShot += elapsedTime;

    int i = 0;
    for (auto &projectile : projectiles)
    {
        if (projectile.valid)
        {
            projectile.update(elapsedTime);
            i++;
        }
        else
        {
            projectiles.erase(projectiles.begin() + i);
        }
    }

    if (controls & 0x10)
    {
        if(timeSinceLastShot.asMilliseconds() > 20)
        {
            timeSinceLastShot = sf::Time::Zero;
            projectiles.push_back(Projectile(this->position, 800, this->rotation, 700, 0));
        }
    }

    //printf("player %i has %i projectiles \n", playerId, projectiles.size());

    position += movement * elapsedTime.asSeconds();
}

//Escribe la data del player al array de bytes en la posición indicada
//Devuelve la cantidad de bytes escritos
int Player::serialize(char * buffer, int position)
{
    int pos = position;
    Serialization::shortToChars(this->playerId, buffer, pos); //Player id 6 - 7
    pos += 2;
    Serialization::floatToChars(this->position.x, buffer, pos); //Pos x 8 - 12
    pos += 4;
    Serialization::floatToChars(this->position.y, buffer, pos); //Pos y 13 - 17
    pos += 4;
    Serialization::floatToChars(this->rotation, buffer, pos); //Angle in rads 13 - 17
    pos += 4;

    return pos - position;
}


