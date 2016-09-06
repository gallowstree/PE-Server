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
    size_t maxPlayers;
    sf::Time timeSinceGameEnded;

    //Send updates after game ended for this time
    const sf::Time limboTime;

    int16_t noPlayers[2];
    int16_t alivePlayers[2];

    std::vector<Player*> players;
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
    void processInputCmd(const command_t &command);
    void processTeamCmd(const command_t &command);
    void processJoinCmd(command_t &command);
    int16_t findPlayerIndexByIp(const char * ip);
    void sendGameInfo(const char * c_ip);
    bool checkForGameEnd();
    void broadcastResult();

    bool gameEnded;

    size_t constructPlayersMessage(char *outbuffer);

    size_t constructProjectileMessage(char *projectiles, int16_t* projMsgNum);

    size_t constructPickupMessage(char *outbuffer);
};


#endif //TEST_SERVER_GAME_H
