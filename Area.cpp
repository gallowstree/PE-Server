//
// Created by Alejandro on 13/08/16.
//

#include "Area.h"


void Area::draw(sf::RenderTarget &window, bool debugGrid)
{
    if (debugGrid)
    {
        auto shape = sf::RectangleShape(sf::Vector2f(rect.width, rect.height));
        shape.setPosition(rect.left, rect.top);
        shape.setFillColor(sf::Color(200, 200, 200));
        shape.setOutlineColor(sf::Color(180, 100, 120));

        shape.setOutlineThickness(5);
        window.draw(shape);
    }
}

Area::Area(float d, float d1, float d2, float d3)
{
    rect = sf::FloatRect(d, d1, d2, d3);
}



