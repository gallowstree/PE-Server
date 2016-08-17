//
// Created by Alejandro on 15/08/16.
//

#ifndef GAMEPLAY_PROTOTYPE_BOUNDINGBOX_H
#define GAMEPLAY_PROTOTYPE_BOUNDINGBOX_H


#include <SFML/Graphics.hpp>

class BoundingBox : public sf::FloatRect
{
public:
    BoundingBox(float left = 0, float top = 0, int width = 0, int height = 0);


    sf::Vector2f getPosition();

    sf::Vector2f getSize();
};


#endif //GAMEPLAY_PROTOTYPE_BOUNDINGBOX_H
