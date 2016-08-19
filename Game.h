//
// Created by Alejandro on 18/08/16.
//

#ifndef TEST_SERVER_GAME_H
#define TEST_SERVER_GAME_H


#include "Player.h"
#include "ServerSocket.h"

class Game : public SocketListener
{
public:
    Game();
    void run();

private:
    std::vector<Player> players;
    const sf::Time TimePerFrame;
    const sf::Time TimePerNetworkUpdate;

    void receiveMessage(char *buffer, size_t nBytes, sockaddr_in *clientAddr);
};


#endif //TEST_SERVER_GAME_H
