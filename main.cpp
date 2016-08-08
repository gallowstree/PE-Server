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
const int16_t c_input_command = 0;
int message_number = 0;

uint currentFrame = 0;
std::queue<command_t> commandQueue;
pthread_mutex_t commandQueueMutex;

std::vector<Player> players;

const char* serverIP = "192.168.1.90";


void init()
{
    pthread_mutex_init(&commandQueueMutex, NULL);
    players.push_back(Player(0, "192.168.1.12", 50421, sf::Vector2f(20.0f,20.0f)));
    players.push_back(Player(1, serverIP, 50421, sf::Vector2f(10.0f,10.0f)));
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
        nBytes = recvfrom(udpSocket, buffer, COMMAND_BUFFER_SIZE, 0, (struct sockaddr *)&serverStorage, &addr_size);
        command_t command;
        Serialization::charsToShort(buffer, command.commandType, 0);
        Serialization::charsToShort(buffer, command.playerId, 2);
        Serialization::charsToInt(buffer, command.msgNum, 4);
        Serialization::charsToInt(buffer, command.controls, 8);

        /*printf("commandType: %i ", command.commandType);
        printf("playerId: %i ", command.playerId);
        printf("msgNum: %i ", command.msgNum);
        printf("keys: %04x\n", command.controls);*/

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
            player.send(projectiles, projectile_pos);
    }
}

void processEvents()
{

    pthread_mutex_lock(&commandQueueMutex);
    while (!commandQueue.empty())
    {
        command_t command = commandQueue.front();
        commandQueue.pop();

        players[command.playerId].controls = command.controls;
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