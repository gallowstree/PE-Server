//
// Created by Alejandro on 15/08/16.
//

#include "BoundingBox.h"

sf::Vector2f BoundingBox::getPosition() {
    return sf::Vector2f(left, top);
}

sf::Vector2f BoundingBox::getSize()
{
    return sf::Vector2f(width, height);
}

BoundingBox::BoundingBox(float left, float top, int width, int height) : sf::FloatRect(left, top, width, height) {

}



