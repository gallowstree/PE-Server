//
// Created by Alejandro on 18/08/16.
//

#include <arpa/inet.h>
#include "Game.h"
#include "serialization.h"
#include "command.h"

const int16_t s_players_command = 0;
const int16_t s_projectiles_command = 1;
int16_t const s_player_id_command = 2;
const int16_t c_input_command = 0;
const int16_t c_join_game_command = 2;

void Game::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Time timeSinceLastNetworkUpdate = sf::Time::Zero;
    while (true)
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
}

Game::Game() :
    TimePerFrame(sf::seconds(1/120.0f)),
    TimePerNetworkUpdate(sf::seconds(1/30.0f)),
    world()
{
    reset();
}

void Game::receiveMessage(char *buffer, size_t nBytes, sockaddr_in *clientAddr)
{
    command_t command;
    Serialization::charsToShort(buffer, command.commandType, 0);
    command.commandType = command.commandType;
    command.client_ip = inet_ntoa(clientAddr->sin_addr);

    switch (command.commandType)
    {
        case c_input_command:
            deserializeInputCmd(command, buffer);
            break;
        case c_join_game_command:
            break;
    }

    pthread_mutex_lock(&commandQueueMutex);
    commandQueue.push(command);
    pthread_mutex_unlock(&commandQueueMutex);
}

void Game::reset()
{
    pthread_mutex_lock(&commandQueueMutex);
    std::queue<command_t>().swap(commandQueue);
    players.clear();
    world.init("maps/level1.txt", &players);
    message_number = 0;
    currentFrame = 0;
    pthread_mutex_unlock(&commandQueueMutex);
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
        command.messageAcks = new int[command.numberOfAcks];//(int32_t *) malloc(sizeof(int32_t) * command.numberOfAcks);
        for (int i = 0; i < command.numberOfAcks; i++)
        {
            Serialization::charsToInt(buffer, command.messageAcks[i], ack_pos);
            ack_pos += 4;
        }
    }
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
            players[command.playerId].controls = command.controls;
            players[command.playerId].rotation = command.rotation;
            players[command.playerId].hasNotAckedId = false;

            for (int i = 0; i < command.numberOfAcks; i++)
            {
                auto ack = command.messageAcks[i];
                auto pending = pendingMessageAcks[ack];
                pending.erase(std::remove(pending.begin(), pending.end(), command.playerId), pending.end());

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
        else if (command.commandType == c_join_game_command)
        {
            int16_t playerIndex = -1;
            for(auto &player : players)
            {
                if(strcmp(player.ip,command.client_ip) == 0)
                    playerIndex = player.playerId;
            }
            printf("Processing join request %s %d\n",command.client_ip,playerIndex);
            if(playerIndex == -1)
            {
                int16_t new_player_id = (int16_t)players.size();
                char * c_ip = (char *)malloc(strlen(command.client_ip)+1);
                strcpy(c_ip,command.client_ip);
                Player newPlayer(new_player_id, c_ip, 50421, sf::Vector2f(20.0f,20.0f));
                newPlayer.movementBounds = sf::FloatRect(0.0f, 0.0f, 2400.0f, 2400.0f);
                players.push_back(newPlayer);
                printf("Inserted player %d\n", new_player_id);
            }
            else
            {
                players[playerIndex].hasNotAckedId = true;
            }
        }
    }
    pthread_mutex_unlock(&commandQueueMutex);
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
            if (projectile.valid && !projectile.acked)
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


