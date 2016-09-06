//
// Created by Alejandro on 30/07/16.
//

#include <arpa/inet.h>
#include "Player.h"
#include "serialization.h"
#include "Pickup.h"
#include <string.h>
#include <math.h>


Player::Player(int16_t playerId, sf::Vector2f position, OutputSocket socket, char * nick) :
playerId(playerId),
timeSinceLastShot(sf::Time::Zero),
speed(500),
rotation(0),
cross_thickness(5),
socket(socket),
health(100),
playerInfo(0),
ip(socket.ip),
nick(nick),
ammo(10)
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

    if (controls & 0x10  && health > 0)
    {
        if(timeSinceLastShot.asMilliseconds() > 200 && ammo > 0)
        {
            timeSinceLastShot = sf::Time::Zero;
            auto pos = this->boundingBox.getPosition() + sf::Vector2f(boundingBox.width /2, boundingBox.height / 2);
            projectiles.push_back(Projectile(pos, 1000, this->rotation, 1000, 0, playerId));
            ammo--;
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
int Player::serialize(char * buffer, int start)
{
    int pos = start;

    Serialization::shortToChars(this->playerId | this->playerInfo, buffer, pos); //Player id 6 - 7
    pos += 2;
    Serialization::floatToChars(this->boundingBox.getPosition().x, buffer, pos); //Pos x 8 - 12
    pos += 4;
    Serialization::floatToChars(this->boundingBox.getPosition().y, buffer, pos); //Pos y 13 - 17
    pos += 4;
    Serialization::floatToChars(this->rotation, buffer, pos); //Angle in rads 13 - 17
    pos += 4;
    Serialization::shortToChars(this->health, buffer, pos); //Health 17 - 19
    pos += 2;
    strcpy(buffer + pos, nick);
    pos += strlen(nick) + 1;
    memcpy(buffer + pos, &ammo, 1);
    pos++;
    return pos - start;
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
        intersectedWall(intersection);
    }
    else if (other->type == EntityType::Projectile_T)
    {
        auto proj = ((Projectile*) other);
        intersectedProjectile(proj);
    }
    else if (other->type == EntityType::Pickup_T)
    {
        auto pickup = ((Pickup*) other);
        if (pickup->enabled)
            intersectedPickup(pickup);
    }
}

void Player::intersectedProjectile(Projectile *proj) {
    if (proj->valid)
    {
        health -= proj->getPower();
        if (health < 0)
            health = 0;
        //printf("player %i hit by projectile %i, health %i\n", playerId, other->entityId, health);
    }
}

void Player::intersectedWall(const sf::FloatRect &intersection)
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

int Player::getTeam() {
    auto team = (playerInfo & (0x0100)) >> 8;
    return team;
}

int Player::getValid() {
    return (playerInfo & (0x0200)) >> 9;
}

void Player::sendPlayerId(int16_t message_number, int16_t command_type)
{
    char playerIdBuffer[100];
    size_t pId_pos = 0;

    Serialization::intToChars(message_number, playerIdBuffer, pId_pos);
    pId_pos += 4;
    Serialization::shortToChars(command_type, playerIdBuffer, pId_pos);
    pId_pos += 2;
    Serialization::shortToChars(playerId, playerIdBuffer, pId_pos);
    pId_pos += 2;

    send(playerIdBuffer, pId_pos);
}

void Player::intersectedPickup(Pickup *pPickup)
{
    if (pPickup->pickupType == Health_T && health < 100)
    {
        health = 100;
        pPickup->enabled = false;
        pPickup->elapsedTime = sf::Time::Zero;
    }
    else if (pPickup->pickupType == Ammo_T && ammo < 255)
    {
        ammo = (unsigned char) std::min(ammo + 15, 255);
        pPickup->enabled = false;
        pPickup->elapsedTime = sf::Time::Zero;
    }
}







