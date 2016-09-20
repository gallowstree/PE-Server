//
// Created by Alejandro on 1/09/16.
//

#include "Pickup.h"

Pickup::Pickup(int l, int t, int w, int h, int type, int seconds, int initialState) :
seconds(seconds),
elapsedTime(sf::Time::Zero)
{
    boundingBox.top = t;
    boundingBox.left = l;
    boundingBox.height = h;
    boundingBox.width = w;
    pickupId = nextPickupId++;
    isStatic = true;
    enabled = initialState == 1;
    this->type = Pickup_T;
    pickupType = static_cast<PickupType>(type);
}

void Pickup::update(sf::Time elapsedTime)
{
    this->elapsedTime += elapsedTime;

    if (this->elapsedTime.asSeconds() > seconds)
    {
        this->enabled = true;
        this->elapsedTime = sf::Time::Zero;
    }

}



