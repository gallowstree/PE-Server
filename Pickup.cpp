//
// Created by Alejandro on 1/09/16.
//

#include "Pickup.h"

Pickup::Pickup(int l, int t, int w, int h, int type, int seconds, int initialState) {
    boundingBox.top = t;
    boundingBox.left = l;
    boundingBox.height = h;
    boundingBox.width = w;
    pickupId = nextPickupId++;
    isStatic = true;
    enabled = initialState == 1;
    this->type = Pickup_T;
    pickupType = type == 0 ? Ammo_T : Health_T;
}

