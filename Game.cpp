//
// Created by Alejandro on 18/08/16.
//

#include "Game.h"

void Game::run()
{

}

Game::Game() :
    TimePerFrame(sf::seconds(1/120.0f)),
    TimePerNetworkUpdate(sf::seconds(1/30.0f))
{

}



void Game::receiveMessage(char *buffer, size_t nBytes, sockaddr_in *clientAddr)
{
    printf("Received! \n");
}


