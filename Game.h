//
// Created by Alejandro on 18/08/16.
//

#ifndef TEST_SERVER_GAME_H
#define TEST_SERVER_GAME_H


#include "Player.h"
#include "ServerSocket.h"
#include "World.h"
#include "command.h"

class Game : public SocketListener
{
public:
    Game();
    void run();
    void reset();
private:
    uint currentFrame;
    int message_number;

    std::vector<Player> players;
    std::queue<command_t> commandQueue;
    std::map<int32_t, std::vector<Projectile*>> projectilesInMessage;
    std::map<int32_t, std::vector<int16_t>> pendingMessageAcks;

    const sf::Time TimePerFrame;
    const sf::Time TimePerNetworkUpdate;
    World world;
    pthread_mutex_t commandQueueMutex;

    void receiveMessage(char *buffer, size_t nBytes, sockaddr_in *clientAddr);

    void deserializeInputCmd(commamd &command, const char *buffer);
    void processEvents();
    void networkUpdate();
};


#endif //TEST_SERVER_GAME_H
