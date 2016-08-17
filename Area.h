//
// Created by Alejandro on 13/08/16.
//

#ifndef GAMEPLAY_PROTOTYPE_AREA_H
#define GAMEPLAY_PROTOTYPE_AREA_H


#include <SFML/Graphics.hpp>

class Area
{
public:
    Area(float x, float y, float width, float height);
    void draw(sf::RenderTarget &window, bool debugGrid);
    sf::FloatRect rect;
};


#endif //GAMEPLAY_PROTOTYPE_AREA_H
