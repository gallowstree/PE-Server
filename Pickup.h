//
// Created by Alejandro on 1/09/16.
//

#ifndef TEST_SERVER_PICKUP_H
#define TEST_SERVER_PICKUP_H


#include "Entity.h"

enum PickupType
{
    Ammo_T,
    Health_T,
};

class Pickup : public Entity
{
public:
    Pickup(int l, int t, int w, int h, int type, int seconds, int initialState);
    PickupType pickupType;
    int16_t pickupId;
    bool enabled;
	static int16_t nextPickupId;
};


#endif //TEST_SERVER_PICKUP_H
