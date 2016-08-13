#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <cctype>
#include <SFML/Graphics.hpp>
#include <queue>
#include "serialization.h"
#include "Player.h"
#include "command.h"
#include "Projectile.h"
#include <unistd.h>

const sf::Time TimePerFrame = sf::seconds(1.f/50.f);
const ushort COMMAND_BUFFER_SIZE = 1024;

const int16_t s_players_command = 0;
const int16_t s_projectiles_command = 1;
int16_t const s_player_id_command = 2;
const int16_t c_input_command = 0;
const int16_t c_join_game_command = 2;
int message_number = 0;

uint currentFrame = 0;
std::queue<command_t> commandQueue;
pthread_mutex_t commandQueueMutex;

std::vector<Player> players;
std::map<const char*, int16_t> player_ips;
//std::map<int32_t, Projectile[]> projectilesInMessage;

const char* serverIP = "192.168.1.9";


void init()
{
    nextProjectileId = 0;
    pthread_mutex_init(&commandQueueMutex, NULL);
    /*players.push_back(Player(0, "192.168.2.2", 50421, sf::Vector2f(20.0f,20.0f)));
    players.push_back(Player(1, "192.168.2.2", 50421, sf::Vector2f(10.0f,10.0f)));*/
    //players.push_back(Player(2, "127.0.0.1", 50421, sf::Vector2f(40.0f,0.0f)));
}


void *listenToClients(void * args)
{
    int udpSocket;
    ssize_t nBytes;
    char buffer[COMMAND_BUFFER_SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size, client_addr_size;
    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(50420);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverStorage;

    while(1)
    {

        int16_t commandType;
        nBytes = recvfrom(udpSocket, buffer, COMMAND_BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &addr_size);
        Serialization::charsToShort(buffer, commandType, 0);
        command_t command;
        command.commandType = commandType;

        if (commandType == c_input_command)
        {
            printf("Receiving controls\n");
            Serialization::charsToShort(buffer, command.playerId, 2);
            Serialization::charsToInt(buffer, command.msgNum, 4);
            Serialization::charsToInt(buffer, command.controls, 8);
            Serialization::charsToFloat(buffer, command.rotation, 12);
        }
        else if (commandType == c_join_game_command)
        {
            printf("received join command\n");
            command.client_ip = inet_ntoa(clientAddr.sin_addr);
        }

        pthread_mutex_lock(&commandQueueMutex);
        commandQueue.push(command);
        pthread_mutex_unlock(&commandQueueMutex);
    }

    return 0;
}

void update(sf::Time elapsedTime)
{

    for (auto &player : players)
    {
        player.update(elapsedTime);
    }

    char outbuffer[512];
    char projectiles[1500];

    size_t pos = 0, projectile_pos = 0;
    Serialization::intToChars(message_number++, outbuffer, pos); //idmsg 0 - 3
    pos += 4;
    Serialization::shortToChars(s_players_command, outbuffer, pos); //Command type 4 - 5
    pos += 2;

    Serialization::intToChars(message_number++, projectiles, projectile_pos);
    projectile_pos += 4;
    Serialization::intToChars(s_projectiles_command, projectiles, projectile_pos);
    projectile_pos += 2;

    for (auto &player : players)
    {
        pos += player.serialize(outbuffer, pos);

        for (auto &projectile : player.projectiles)
        {
            projectile_pos += projectile.serialize(projectiles, projectile_pos);
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
        }

        if (player.hasNotAckedId)
        {
            printf("has not acked \n");
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

void processEvents()
{
    pthread_mutex_lock(&commandQueueMutex);
    while (!commandQueue.empty())
    {
        command_t command = commandQueue.front();
        commandQueue.pop();

        if (command.commandType == c_input_command && command.playerId != -1 && players.size() >= command.playerId)
        {
            printf("Processing controls\n");
            players[command.playerId].controls = command.controls;
            players[command.playerId].rotation = command.rotation;
            players[command.playerId].hasNotAckedId = false;
        }
        else if (command.commandType == c_join_game_command)
        {
            printf("Processing join request\n");
            auto it = player_ips.find(command.client_ip);
            if (it == player_ips.end())
            {
                auto new_player_id = (int16_t)players.size();
                player_ips[command.client_ip] = new_player_id;
                Player newPlayer(new_player_id, command.client_ip, 50421, sf::Vector2f(20.0f,20.0f));
                players.push_back(newPlayer);
                printf("Inserted player %d\n", new_player_id);
            }
        }
    }
    pthread_mutex_unlock(&commandQueueMutex);

}

int main()
{
    init();
    pthread_t listeningThread;
    pthread_create(&listeningThread, NULL, listenToClients, NULL);

    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    while (true)
    {
        sf::Time elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        while (timeSinceLastUpdate > TimePerFrame)
        {
            currentFrame++;
            timeSinceLastUpdate -= TimePerFrame;
            processEvents();
            update(TimePerFrame);
        }
    }

    pthread_join(listeningThread, NULL);
}