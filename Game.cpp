//
// Created by Alejandro on 18/08/16.
//

#include <arpa/inet.h>
#include "Game.h"
#include "serialization.h"
#include "command.h"
#include "Pickup.h"

const int16_t s_players_command = 0;
const int16_t s_projectiles_command = 1;
const int16_t s_player_id_command = 2;
const int16_t s_gameover_command = 3;
const int16_t s_pickups_command = 4;

const int16_t c_input_command = 0;
const int16_t c_team_command = 2;
const int16_t c_join_game_command = 3;

int16_t Projectile::nextProjectileId;
int16_t Pickup::nextPickupId;

Game::Game() :
        TimePerFrame(sf::seconds(1/120.0f)),
        TimePerNetworkUpdate(sf::seconds(1/30.0f)),
        world(),
        maxPlayers(4),
        timeSinceGameEnded(sf::Time::Zero),
        limboTime(sf::seconds(0.5)),
        gameEnded(false)
{
    pthread_mutex_init(&commandQueueMutex, NULL);
    reset();
}


void Game::reset()
{
    pthread_mutex_lock(&commandQueueMutex);

    Projectile::nextProjectileId = 0;
    Pickup::nextPickupId = 0;

    timeSinceGameEnded = sf::Time::Zero;

    //Empty command queue
    std::queue<command_t>().swap(commandQueue);

    //Delete all allocated players and clear the vector
    for (auto it = players.begin() ; it != players.end(); ++it)
    {
        delete (*it)->ip;
        delete (*it)->nick;
        delete (*it);
    }
    players.clear();

    //Clear maps
    projectilesInMessage.clear();
    pendingMessageAcks.clear();

    //Reset world and load map
    world.init("maps/level1.txt", &players);

    //Clear state variables
    message_number = 0;
    currentFrame = 0;
    alivePlayers[0] = 0;
    alivePlayers[1] = 0;
    noPlayers[0] = 0;
    noPlayers[1] = 0;
    gameEnded = false;



    pthread_mutex_unlock(&commandQueueMutex);
}

void Game::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Time timeSinceLastNetworkUpdate = sf::Time::Zero;
    while (!checkForGameEnd() || timeSinceGameEnded < limboTime)
    {
        sf::Time elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        timeSinceLastNetworkUpdate += elapsedTime;
        timeSinceGameEnded += gameEnded ? elapsedTime : sf::Time::Zero;
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

    broadcastResult();

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
        case c_team_command:
            printf("Received join command\n");
            Serialization::charsToShort(buffer, command.team, 2);
            break;
        case c_join_game_command:
            printf("here!\n");
            memset(command.nickname,0,7);
            strcpy(command.nickname, buffer + 2);
            printf("Received nick '%s' length: %i\n", command.nickname, strlen(command.nickname));
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
        else if (command.commandType == c_team_command)
        {
            processTeamCmd(command);
        }
        else if (command.commandType == c_join_game_command)
        {
            processJoinCmd(command);
        }
    }
    pthread_mutex_unlock(&commandQueueMutex);
}

void Game::processTeamCmd(const command_t &command)
{
    auto playerIndex = findPlayerIndexByIp(command.client_ip);

    if (playerIndex != -1)
    {
        auto player = players[playerIndex];
        player->hasNotAckedId = true;

        if (player->getValid() == 0)
        {
            player->setTeam(command.team);
            player->setValid(1);
            printf("Setting team for player, ID: %i, TEAM: %i\n, VALID: %i", playerIndex, player->getTeam(), player->getValid());
        }
    }
    else
    {
        printf("ESTO NO DEBERÍA PASAAAAAARRRRRRRRR !!!!!!!!!!!!!!\n");
    }
}

void Game::processInputCmd(const command_t &command)
{
    players[command.playerId]->controls = command.controls;
    players[command.playerId]->rotation = command.rotation;
    players[command.playerId]->hasNotAckedId = false;

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

size_t Game::constructPlayersMessage(char outbuffer[])
{
    size_t pos = 0;

    Serialization::intToChars(message_number++, outbuffer, pos); //idmsg 0 - 3
    pos += 4;
    Serialization::shortToChars(s_players_command, outbuffer, pos); //Command type 4 - 5
    pos += 2;

    for (auto &player : players)
    {
        pos += player->serialize(outbuffer, pos);
    }

    Serialization::shortToChars(-1, outbuffer, pos);
    pos += 2;

    return pos;
}

size_t Game::constructProjectileMessage(char projectiles[], int16_t *porjMsgNum)
{
    size_t projectile_pos = 0;
    const auto projectile_msgno = message_number++;
    Serialization::intToChars(projectile_msgno, projectiles, projectile_pos);
    projectile_pos += 4;
    Serialization::intToChars(s_projectiles_command, projectiles, projectile_pos);
    projectile_pos += 2;

    for (auto &player : players)
    {
        for (auto &projectile : player->projectiles)
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

    Serialization::shortToChars(-1, projectiles, projectile_pos);
    projectile_pos += 2;

    return projectile_pos;
}

size_t Game::constructPickupMessage(char outbuffer[])
{
    size_t pos = 0;

    Serialization::intToChars(message_number++, outbuffer, pos); //idmsg 0 - 3
    pos += 4;
    Serialization::shortToChars(s_pickups_command, outbuffer, pos); //Command type 4 - 5
    pos += 2;

    for (auto &entities : world.static_entities)
    {
        for (auto &entity : entities)
        {
            if (entity->type == Pickup_T)
            {
                auto pickup = ((Pickup*) entity);

                Serialization::shortToChars(pickup->pickupId, outbuffer, pos);
                pos += 2;
                outbuffer[pos] = (char)(pickup->enabled ? 1 : 0);
                pos++;
            }
        }
    }

    Serialization::shortToChars(-1, outbuffer, pos);
    pos += 2;

    return pos;
}

void Game::networkUpdate()
{
    char outbuffer[512];
    char pickupBuffer[512];
    char projectiles[1500];
    int16_t projectile_msgno = 0;

    size_t pos = constructPlayersMessage(outbuffer);
    size_t projectile_pos = constructProjectileMessage(projectiles, &projectile_msgno);
    size_t pickup_pos = constructPickupMessage(pickupBuffer);

    //Maybe move this to another thread?
    for (auto &player : players)
    {
        player->send(outbuffer, pos);

        if (projectile_pos > 10)
        {
            player->send(projectiles, projectile_pos);
            if (pendingMessageAcks.count(projectile_msgno) == 0)
            {
                pendingMessageAcks[projectile_msgno] = std::vector<int16_t >();
            }
            pendingMessageAcks[projectile_msgno].push_back(player->playerId);
        }

        if (pickup_pos > 8)
        {
            player->send(pickupBuffer, pickup_pos);
        }

        if (player->hasNotAckedId)
        {
            player->sendPlayerId(message_number++, s_player_id_command);
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

void Game::processJoinCmd(command_t &command)
{
    int playerIdx = findPlayerIndexByIp(command.client_ip);


    if (playerIdx != -1) //Player already exists
    {
        char out[2];
        Serialization::shortToChars(2, out, 0);
        strcpy(players[playerIdx]->nick,command.nickname);
        OutputSocket(command.client_ip, 50422).send(out, 2);
    }
    else if (players.size() < maxPlayers) //There is room for the player
    {
        int16_t new_player_id = (int16_t) players.size();
        char * c_ip = (char *)calloc(strlen(command.client_ip)+1, sizeof(char));
        strcpy(c_ip,command.client_ip);

        char * nick = (char *)malloc(7*sizeof(char));
        memset(nick,0,7);
        strcpy(nick,command.nickname);

        printf("Created player %i with ip %s and nick %s team pending!\n", new_player_id, c_ip, nick);

        Player* newPlayer = new Player(new_player_id, sf::Vector2f(50.0f, 50.0f), OutputSocket(c_ip, 50421), nick);
        newPlayer->movementBounds = sf::FloatRect(0.0f, 0.0f, world.bounds.width, world.bounds.height);
        players.push_back(newPlayer);
        sendGameInfo(c_ip);
    }
    else //Server full
    {
        char out[2];
        Serialization::shortToChars(3, out, 0);
        OutputSocket(command.client_ip, 50422).send(out, 2);
    }
}

//Returns index of player with the given ip, -1 if not found
int16_t Game::findPlayerIndexByIp(const char * ip)
{
    int16_t playerIndex = -1;
    for(auto &player : players)
    {
        if(strcmp(player->ip,ip) == 0)
            playerIndex = player->playerId;
    }
    return  playerIndex;
}



void Game::sendGameInfo(char * c_ip)
{
    char out[6];
    Serialization::shortToChars(1, out, 0);
    Serialization::shortToChars(noPlayers[0], out, 2);
    Serialization::shortToChars(noPlayers[1], out, 4);
    OutputSocket(c_ip, 50422).send(out, 6);
}


bool Game::checkForGameEnd()
{
    alivePlayers[0] = 0;
    alivePlayers[1] = 0;
    noPlayers[0] = 0;
    noPlayers[1] = 0;

    for (auto& player : players)
    {
        if (player->getValid() == 0)
            continue;

        noPlayers[player->getTeam()]++;
        if (player->health > 0)
            alivePlayers[player->getTeam()]++;
    }

    bool teamsValid = (noPlayers[0] > 0 && noPlayers[1] > 0);
    bool deadTeam   = (alivePlayers[0] == 0 || alivePlayers[1] == 0);


    if (teamsValid &&
        deadTeam)
    {
        gameEnded = true;
    }

    return gameEnded;
}

void Game::broadcastResult()
{
    char buffer[8];
    message_number++;
    int16_t winner = 2;

    if (alivePlayers[0] > 0) winner = 0;
    if (alivePlayers[1] > 0) winner = 1;

    pendingMessageAcks.clear();
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Time timeRemaining = sf::seconds(5);

    while (timeRemaining.asSeconds() > 0)
    {
        sf::Time elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        timeRemaining -= elapsedTime;
        while (timeSinceLastUpdate > TimePerNetworkUpdate)
        {
            for (auto& player : players)
            {
                Serialization::intToChars(message_number, buffer, 0);
                Serialization::shortToChars(s_gameover_command, buffer, 4);
                Serialization::shortToChars(winner, buffer, 6);
                player->send(buffer, 8);
                timeSinceLastUpdate -= TimePerNetworkUpdate;
            }
        }
    }
}











