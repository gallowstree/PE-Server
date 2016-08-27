//
// Created by Alejandro on 18/08/16.
//

#ifndef TEST_SERVER_GAME_H
#define TEST_SERVER_GAME_H


#include "Player.h"
#include "ServerSocket.h"
#include "World.h"
#include "command.h"

typedef struct lobbyPlayer
{
    OutputSocket * socket;
    sf::Time timeLeft;
} lobbyPlayer_t;

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

    int16_t noPlayers[2];
    int16_t alivePlayers[2];

    std::vector<Player> players;
    std::vector<lobbyPlayer_t> inLobby;
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

    void processJoinCmd(const command_t &command);

    void processInfoCommand(command_t &command);

    int16_t findPlayerIndexByIp(const char * ip);

    int findLobbyPlayer(const char *ip, lobbyPlayer_t* &found);

    void sendGameInfo(const OutputSocket *socket);

    void deleteFromLobby(const char *ip);

    bool checkForWinner();

};


#endif //TEST_SERVER_GAME_H
