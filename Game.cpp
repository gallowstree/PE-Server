//
// Created by Alejandro on 18/08/16.
//

#include <arpa/inet.h>
#include "Game.h"
#include "serialization.h"
#include "command.h"

const int16_t s_players_command = 0;
const int16_t s_projectiles_command = 1;
const int16_t  s_player_id_command = 2;
const int16_t s_gameover_command = 3;
const int16_t c_input_command = 0;
const int16_t c_join_game_command = 2;
const int16_t c_info_command = 3;

Game::Game() :
        TimePerFrame(sf::seconds(1/120.0f)),
        TimePerNetworkUpdate(sf::seconds(1/30.0f)),
        world(),
        maxPlayers(4)
{
    pthread_mutex_init(&commandQueueMutex, NULL);
    reset();
}


void Game::reset()
{
    pthread_mutex_lock(&commandQueueMutex);
    std::queue<command_t>().swap(commandQueue);
    players.clear();
    inLobby.clear();

    world.init("maps/level1.txt", &players);
    message_number = 0;
    currentFrame = 0;
    pthread_mutex_unlock(&commandQueueMutex);
}

void Game::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Time timeSinceLastNetworkUpdate = sf::Time::Zero;
    while (!checkForWinner())
    {
        sf::Time elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        timeSinceLastNetworkUpdate += elapsedTime;
        while (timeSinceLastUpdate > TimePerFrame)
        {
            currentFrame++;
            timeSinceLastUpdate -= TimePerFrame;
            processEvents();
            world.update(TimePerFrame);
        }

        while (timeSinceLastNetworkUpdate > TimePerNetworkUpdate)
        {
            timeSinceLastNetworkUpdate -= TimePerNetworkUpdate;
            networkUpdate();
        }
    }

    char buffer[8];
    message_number++;
    int16_t winner = 2;

    if (alivePlayers[0] > 0) winner = 0;
    if (alivePlayers[1] > 0) winner = 1;

    while (true)
    {
        for (auto& player : players)
        {
            Serialization::intToChars(message_number, buffer, 0);
            Serialization::shortToChars(s_gameover_command, buffer, 4);
            Serialization::shortToChars(winner, buffer, 6);
            player.send(buffer, 8);
        }

        sleep(1);
    }
}

void Game::receiveMessage(char *buffer, size_t nBytes, sockaddr_in *clientAddr)
{
    command_t command;
    Serialization::charsToShort(buffer, command.commandType, 0);
    command.commandType = command.commandType;
    command.client_ip = inet_ntoa(clientAddr->sin_addr);
    //printf("received command type: %i\n", command.commandType);
    switch (command.commandType)
    {
        case c_input_command:
            deserializeInputCmd(command, buffer);
            break;
        case c_join_game_command:
            printf("Received join command\n");
            Serialization::charsToShort(buffer, command.team, 2);
        case c_info_command:
            break;
    }

    pthread_mutex_lock(&commandQueueMutex);
    commandQueue.push(command);
    pthread_mutex_unlock(&commandQueueMutex);
}



void Game::processEvents()
{
    pthread_mutex_lock(&commandQueueMutex);
    while (!commandQueue.empty())
    {
        command_t command = commandQueue.front();
        commandQueue.pop();

        if (command.commandType == c_input_command && command.playerId != -1 && players.size() >= command.playerId && !players.empty())
        {
            processInputCmd(command);
        }
        else if (command.commandType == c_join_game_command)
        {
            processJoinCmd(command);
        }
        else if (command.commandType == c_info_command)
        {
            processInfoCommand(command);
        }
    }
    pthread_mutex_unlock(&commandQueueMutex);
}

void Game::processJoinCmd(const command_t &command)
{
    auto playerIndex = findPlayerIndexByIp(command.client_ip);

    if(playerIndex == -1)
    {
        int16_t new_player_id = (int16_t) players.size();
        char * c_ip = (char *)malloc(strlen(command.client_ip)+1);
        strcpy(c_ip,command.client_ip);

        Player newPlayer(new_player_id, sf::Vector2f(20.0f, 20.0f), OutputSocket(c_ip, 50421), command.team);

        deleteFromLobby(command.client_ip);

        newPlayer.movementBounds = sf::FloatRect(0.0f, 0.0f, world.bounds.width, world.bounds.height);
        players.push_back(newPlayer);
        printf("Inserted player %d team: %d\n", new_player_id, command.team);
    }
    else
    {
        players[playerIndex].hasNotAckedId = true;
    }
}

void Game::processInputCmd(const command_t &command)
{
    players[command.playerId].controls = command.controls;
    players[command.playerId].rotation = command.rotation;
    players[command.playerId].hasNotAckedId = false;

    for (int i = 0; i < command.numberOfAcks; i++)
    {
        auto ack = command.messageAcks[i];
        auto pending = pendingMessageAcks[ack];
        pending.erase(remove(pending.begin(), pending.end(), command.playerId), pending.end());

        if (pending.size() == 0)
        {
            pendingMessageAcks.erase(ack);
            if (projectilesInMessage.count(ack) > 0)
            {
                for (auto &p : projectilesInMessage[ack])
                {
                    if (p != nullptr)
                        p->acked = true;
                }
                projectilesInMessage.erase(ack);
            }
        }
    }
    if (command.numberOfAcks > 0)
    {
        delete[](command.messageAcks);
    }
}

void Game::networkUpdate()
{
    char outbuffer[512];
    char projectiles[1500];

    size_t pos = 0, projectile_pos = 0;
    Serialization::intToChars(message_number++, outbuffer, pos); //idmsg 0 - 3
    pos += 4;
    Serialization::shortToChars(s_players_command, outbuffer, pos); //Command type 4 - 5
    pos += 2;

    const auto projectile_msgno = message_number++;
    Serialization::intToChars(projectile_msgno, projectiles, projectile_pos);
    projectile_pos += 4;
    Serialization::intToChars(s_projectiles_command, projectiles, projectile_pos);
    projectile_pos += 2;

    for (auto &player : players)
    {
        pos += player.serialize(outbuffer, pos);

        for (auto &projectile : player.projectiles)
        {
            if (!projectile.acked)
            {
                projectile_pos += projectile.serialize(projectiles, projectile_pos);
                if (projectilesInMessage.count(projectile_msgno) == 0)
                {
                    projectilesInMessage[projectile_msgno] = std::vector<Projectile*>();
                }
                projectilesInMessage[projectile_msgno].push_back(&projectile);
            }
        }
    }

    Serialization::shortToChars(-1, outbuffer, pos);
    pos += 2;

    Serialization::shortToChars(-1, projectiles, projectile_pos);
    projectile_pos += 2;

    //Maybe move this to another thread?
    for (auto &player : players)
    {
        player.send(outbuffer, pos);
        if (projectile_pos > 10)
        {
            player.send(projectiles, projectile_pos);
            if (pendingMessageAcks.count(projectile_msgno) == 0)
            {
                pendingMessageAcks[projectile_msgno] = std::vector<int16_t >();
            }
            pendingMessageAcks[projectile_msgno].push_back(player.playerId);
        }

        if (player.hasNotAckedId)
        {
            char playerIdBuffer[100];
            size_t pId_pos = 0;

            Serialization::intToChars(message_number++, playerIdBuffer, pId_pos);
            pId_pos += 4;
            Serialization::shortToChars(s_player_id_command, playerIdBuffer, pId_pos);
            pId_pos += 2;
            Serialization::shortToChars(player.playerId, playerIdBuffer, pId_pos);
            pId_pos += 2;

            player.send(playerIdBuffer, pId_pos);
        }
    }
}

void Game::deserializeInputCmd(command_t &command, const char *buffer)
{
    Serialization::charsToShort(buffer, command.playerId, 2);
    Serialization::charsToInt(buffer, command.msgNum, 4);
    Serialization::charsToInt(buffer, command.controls, 8);
    Serialization::charsToFloat(buffer, command.rotation, 12);
    Serialization::charsToInt(buffer, command.numberOfAcks, 16);

    size_t ack_pos = 20;
    if (command.numberOfAcks > 0)
    {
        command.messageAcks = new int[command.numberOfAcks];
        for (int i = 0; i < command.numberOfAcks; i++)
        {
            Serialization::charsToInt(buffer, command.messageAcks[i], ack_pos);
            ack_pos += 4;
        }
    }
}

void Game::processInfoCommand(command_t& command)
{
    lobbyPlayer_t * player = nullptr;

    findLobbyPlayer(command.client_ip, player);

    if (player != nullptr)
    {
        sendGameInfo(player->socket);
    }
    else if (players.size() + inLobby.size() < maxPlayers)
    {
        player = new lobbyPlayer_t;
        player->socket = new OutputSocket(command.client_ip, 50421);
        player->timeLeft = sf::seconds(30);
        inLobby.push_back(*player);
        sendGameInfo(player->socket);
    }
    else
    {
        char out[2];
        Serialization::shortToChars(2, out, 0);
        OutputSocket(command.client_ip, 50421).send(out, 2);
    }
}


int16_t Game::findPlayerIndexByIp(const char * ip)
{
    int16_t playerIndex = -1;
    for(auto &player : players)
    {
        if(strcmp(player.ip,ip) == 0)
            playerIndex = player.playerId;
    }
    return  playerIndex;
}

int Game::findLobbyPlayer(const char * ip, lobbyPlayer_t* &found)
{
    int i = 0;
    for (auto &player : inLobby)
    {
        if(strcmp(player.socket->ip, ip) == 0)
        {
            found = &player;
            return i;
        }
        i++;
    }

    return -1;
}

void Game::sendGameInfo(const OutputSocket* socket)
{
    char out[6];
    Serialization::shortToChars(1, out, 0);
    Serialization::shortToChars(noPlayers[0], out, 2);
    Serialization::shortToChars(noPlayers[1], out, 4);

    socket->send(out, 6);
}

void Game::deleteFromLobby(const char *ip)
{
    lobbyPlayer_t *lobbyP = nullptr;
    auto lobbyIndex = findLobbyPlayer(ip, lobbyP);
    if (lobbyIndex != -1)
    {
        inLobby.erase(inLobby.begin() + lobbyIndex);
        if (lobbyP != nullptr && lobbyP->socket != nullptr)
        {
            delete lobbyP->socket;
            lobbyP->socket = nullptr;
        }
        else if (lobbyP != nullptr)
        {
            delete lobbyP;
            lobbyP = nullptr;
        }
    }
}

bool Game::checkForWinner()
{
    alivePlayers[0] = 0;
    alivePlayers[1] = 0;
    noPlayers[0] = 0;
    noPlayers[1] = 0;

    for (auto& player : players)
    {
        noPlayers[player.getTeam()]++;
        if (player.health > 0)
            alivePlayers[player.getTeam()]++;
    }

    return (noPlayers[0] > 0 && noPlayers[1] > 0) && (alivePlayers[0] == 0 || alivePlayers[1] == 0);
}









