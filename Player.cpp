//
// Created by Alejandro on 30/07/16.
//

#include <arpa/inet.h>
#include "Player.h"
#include "serialization.h"
#include <string.h>
#include <math.h>


Player::Player(int16_t playerId, sf::Vector2f position, OutputSocket socket) :
playerId(playerId),
timeSinceLastShot(sf::Time::Zero),
speed(500),
rotation(0),
cross_thickness(5),
socket(socket),
health(100),
playerInfo(0),
ip(socket.ip)
{
    boundingBox = BoundingBox(position.x, position.y, 50, 50);
    updateCross();
    horz_rect.width = boundingBox.width;
    vert_rect.height = boundingBox.height;
    type = EntityType::Player_T;
    setTeam(0);
    setValid(0);
}

void Player::setValid(int16_t valid)
{
    valid = (int16_t) ((valid & 0x1) << 9);
    playerInfo |= valid;
}

void Player::setTeam(int16_t team)
{
    team = (int16_t) ((team & 0x1) << 8);
    playerInfo |= team;
}


void Player::send(const char *outbuffer, size_t size, int32_t reliableId)
{
    socket.send(outbuffer, size, reliableId);
}

void Player::updateMovement(sf::Time elapsedTime)
{
    if (health <= 0)
        return;

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
    boundingBox.left += movement.x;
    boundingBox.top += movement.y;
    updateCross();
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
        if(timeSinceLastShot.asMilliseconds() > 120)
        {
            timeSinceLastShot = sf::Time::Zero;

            auto pos = this->boundingBox.getPosition() + sf::Vector2f(boundingBox.width /2, boundingBox.height / 2);
            projectiles.push_back(Projectile(pos, 800, this->rotation, 700, 0, playerId));
        }
    }
}

void Player::update(sf::Time elapsedTime)
{
    updateMovement(elapsedTime);
    updateProjectiles(elapsedTime);
}

//Escribe la data del player al array de bytes en la posición indicada
//Devuelve la cantidad de bytes escritos
int Player::serialize(char * buffer, int position)
{
    int pos = position;

    printf("sexo %04x\n", playerInfo);

    Serialization::shortToChars(this->playerId | this->playerInfo, buffer, pos); //Player id 6 - 7
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
            }
            if (intersection.top < boundingBox.top + boundingBox.height / 2)
            {
                boundingBox.top += intersection.height;
            }
        }

        if (intersection.intersects(horz_rect))
        {
            if (intersection.left < boundingBox.left + boundingBox.width / 2)
            {
                boundingBox.left += intersection.width;
            }
            if (intersection.left > boundingBox.left + boundingBox.width / 2)
            {
                boundingBox.left -= intersection.width;
            }
        }
        updateCross();
    }
    else if (other->type == EntityType::Projectile_T)
    {
        if (((Projectile*) other)->valid)
        {
            health -= 25;
            printf("player %i hit by projectile %i, health %i\n", playerId, other->entityId, health);
        }
    }
}

int Player::getTeam() {
    auto team = (playerInfo & (0x0100)) >> 8;
    return team;
}

int Player::getValid() {
    return (playerInfo & (0x0200)) >> 9;
}



