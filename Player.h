//
// Created by Alejandro on 30/07/16.
//

#ifndef TEST_SERVER_PLAYER_H
#define TEST_SERVER_PLAYER_H


#include <SFML/Graphics.hpp>

class Player
{
public:
    int playerId;
    int lastMsgNum;
    sf::Vector2f position;
    sf::Vector2f velocity;


    Player(int playerId, sf::Vector2f position);

};


#endif //TEST_SERVER_PLAYER_H
