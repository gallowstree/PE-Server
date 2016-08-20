//
// Created by Alejandro on 30/07/16.
//

#include <arpa/inet.h>
#include "Player.h"
#include "serialization.h"
#include <string.h>
#include <math.h>


Player::Player(int16_t playerId, const char* ip, int port, sf::Vector2f position) :
playerId(playerId),
lastMsgNum(0),
ip(ip),
timeSinceLastShot(sf::Time::Zero),
speed(500),
rotation(0),
cross_thickness(5),
port(port)
{
    boundingBox = BoundingBox(position.x, position.y, 50, 50);
    updateCross();
    initSocket();
    horz_rect.width = boundingBox.width;
    vert_rect.height = boundingBox.height;
    type = EntityType::Player_T;
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

void Player::updateMovement(sf::Time elapsedTime)
{
    sf::Vector2f movement (0,0);
    float distance = speed * elapsedTime.asSeconds();

    if ((controls & 0x1) && boundingBox.getPosition().y - distance > movementBounds.top)
        movement.y -= speed;
    if ((controls & 0x2) && boundingBox.getPosition().y + distance < movementBounds.top + movementBounds.height)
        movement.y += speed;
    if ((controls & 0x4) && boundingBox.getPosition().x - distance > movementBounds.left)
        movement.x -= speed;
    if ((controls & 0x8) && boundingBox.getPosition().x + distance < movementBounds.top + movementBounds.width)
        movement.x += speed;

    movement *= elapsedTime.asSeconds();
    boundingBox.top += movement.y;
    boundingBox.left += movement.x;
    updateCross();

    printf("%f, %f\\n", boundingBox.height, boundingBox.width);

    //position += movement * elapsedTime.asSeconds();
}

void Player::updateProjectiles(sf::Time elapsedTime)
{
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
        if(timeSinceLastShot.asMilliseconds() > 150)
        {
            timeSinceLastShot = sf::Time::Zero;

            auto pos = this->boundingBox.getPosition() + sf::Vector2f(boundingBox.width /2, boundingBox.height / 2);
            projectiles.push_back(Projectile(pos, 800, this->rotation, 700, 0));
        }
    }
}

void Player::update(sf::Time elapsedTime)
{
    updateMovement(elapsedTime);

    updateProjectiles(elapsedTime);

    //printf("player %i has %i projectiles \n", playerId, projectiles.size());
}

//Escribe la data del player al array de bytes en la posición indicada
//Devuelve la cantidad de bytes escritos
int Player::serialize(char * buffer, int position)
{
    int pos = position;
    Serialization::shortToChars(this->playerId, buffer, pos); //Player id 6 - 7
    pos += 2;
    Serialization::floatToChars(this->boundingBox.getPosition().x, buffer, pos); //Pos x 8 - 12
    pos += 4;
    Serialization::floatToChars(this->boundingBox.getPosition().y, buffer, pos); //Pos y 13 - 17
    pos += 4;
    Serialization::floatToChars(this->rotation, buffer, pos); //Angle in rads 13 - 17
    pos += 4;

    return pos - position;
}

void Player::updateCross()
{
    vert_rect.left = boundingBox.left + boundingBox.width / 2 - cross_thickness/2;
    vert_rect.top = boundingBox.top;
    vert_rect.width = cross_thickness;
    horz_rect.left = boundingBox.left;
    horz_rect.top = boundingBox.top  + boundingBox.height / 2 - cross_thickness/2;
    horz_rect.height = cross_thickness;
}


void Player::intersectedWith(Entity* other, sf::FloatRect intersection)
{
    Entity::intersectedWith(other, intersection);

    if (other->type == EntityType::Wall_T)
    {
        if (intersection.intersects(vert_rect))
        {
            if (intersection.top > boundingBox.top + boundingBox.height / 2)
            {
                boundingBox.top -= intersection.height;
                printf("resolving\n");
            }

            if (intersection.top < boundingBox.top + boundingBox.height / 2)
            {
                boundingBox.top += intersection.height;
                printf("resolving\n");
            }
        }

        if (intersection.intersects(horz_rect))
        {
            if (intersection.left < boundingBox.left + boundingBox.width / 2)
            {
                boundingBox.left += intersection.width;
                printf("resolving\n");
            }

            if (intersection.left > boundingBox.left + boundingBox.width / 2)
            {
                boundingBox.left -= intersection.width;
                printf("resolving\n");
            }
        }
        updateCross();
    }
    else
    {

    }
}

