//
// Created by Alejandro on 1/09/16.
//

#ifndef TEST_SERVER_PICKUP_H
#define TEST_SERVER_PICKUP_H


#include "Entity.h"

enum PickupType
{
	Ammo_T = 0,
	Health_T = 1,
	Portal_T = 2,
	Invisible_T = 3
};

class Pickup : public Entity
{
public:
    Pickup(int l, int t, int w, int h, int type, int seconds, int initialState);
    PickupType pickupType;
    int16_t pickupId;
    bool enabled;
	static int16_t nextPickupId;
    const int seconds;
    void update(sf::Time elapsedTime);
    sf::Time elapsedTime;
};


#endif //TEST_SERVER_PICKUP_H
