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

const sf::Time TimePerFrame = sf::seconds(1.f/60.f);
const ushort COMMAND_BUFFER_SIZE = 1024;
uint currentFrame = 0;
std::queue<command_t> commandQueue;
pthread_mutex_t commandQueueMutex;

std::vector<Player> players;


void init()
{
    pthread_mutex_init(&commandQueueMutex, NULL);
    players.push_back(Player(0, "10.1.11.57", 50421, sf::Vector2f(0.0f,0.0f)));
    players.push_back(Player(1, "192.168.2.6", 50421, sf::Vector2f(0.0f,0.0f)));
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
    serverAddr.sin_addr.s_addr = inet_addr("10.1.11.57");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverStorage;
    int disparito = 0;
    while(1)
    {
        nBytes = recvfrom(udpSocket, buffer, COMMAND_BUFFER_SIZE, 0, (struct sockaddr *)&serverStorage, &addr_size);

        command_t command;
        charsToInt(buffer, command.playerId, 0);
       // printf("playerId: %i ", command.playerId);

        int playerId;
        charsToInt(buffer, playerId, 0);
        //printf("playerId: %i ", playerId);

        int msgNum;
        charsToInt(buffer, msgNum, 4);
        //printf("msgNum: %i ", msgNum);

        int keys;
        charsToInt(buffer, keys, 8);
        //printf("keys: %04x\n", keys);

        float speed = 2;
        if (keys & 0x1)
            players[playerId].position.y += speed;
        if (keys & 0x2)
            players[playerId].position.y -= speed;
        if (keys & 0x4)
                players[playerId].position.x -= speed;
        if (keys & 0x8)
            players[playerId].position.x += speed;
        if (keys & 0x10)
        {
            if((currentFrame - players[playerId].lastBulletFrame ) > 15)
            {
                Projectile p(players[playerId].position,100);
                players[playerId].lastBulletFrame = currentFrame;
            }
        }
        char outbuffer[512];

        intToChars(4, outbuffer, 0); //idmsg

        int pos = 4;
        for (auto &player : players) // access by reference to avoid copying
        {
            intToChars(player.playerId, outbuffer, pos);
            pos += 4;
            floatToChars(player.position.x, outbuffer, pos);
            pos += 4;
            floatToChars(player.position.y, outbuffer, pos);
            pos += 4;
        }

        charsToInt(buffer, command.msgNum, 4);
        //printf("msgNum: %i ", command.msgNum);

        charsToInt(buffer, command.controls, 8);
        //printf("keys: %04x\n", command.controls);

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

    intToChars(4, outbuffer, 0); //idmsg

    int pos = 4;
    for (auto &player : players)
    {
        intToChars(player.playerId, outbuffer, pos);
        pos += 4;
        floatToChars(player.position.x, outbuffer, pos);
        pos += 4;
        floatToChars(player.position.y, outbuffer, pos);
        pos += 4;
    }

    for (auto &player : players)
    {
        player.send(outbuffer, pos);
    }
}

void processEvents()
{

    while (!commandQueue.empty())
    {
        pthread_mutex_lock(&commandQueueMutex);
        command_t command = commandQueue.front();
        commandQueue.pop();
        pthread_mutex_unlock(&commandQueueMutex);

        players[command.playerId].controls = command.controls;
    }

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