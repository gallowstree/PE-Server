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

const sf::Time TimePerFrame = sf::seconds(1.f/60.f);
const ushort COMMAND_BUFFER_SIZE = 1024;

std::queue<command_t> commandQueue;
pthread_mutex_t commandQueueMutex;

std::vector<Player> players;


void init()
{
    pthread_mutex_init(&commandQueueMutex, NULL);
    players.push_back(Player(0, sf::Vector2f(0.0f,0.0f)));
    players.push_back(Player(1, sf::Vector2f(0.0f,0.0f)));
}


void *listenToClients(void * args)
{
    int udpSocket;
    ssize_t nBytes;
    char buffer[COMMAND_BUFFER_SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size, client_addr_size;
    int i;

    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.1.78");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverStorage;
    int received[4];
    while(1)
    {
        /* Try to receive any incoming UDP datagram. Address and port of
          requesting client will be stored on serverStorage variable */
        nBytes = recvfrom(udpSocket, buffer, COMMAND_BUFFER_SIZE, 0, (struct sockaddr *)&serverStorage, &addr_size);

        int playerId;
        charsToInt(buffer, playerId, 0);
        printf("playerId: %i ", playerId);

        int msgNum;
        charsToInt(buffer, msgNum, 4);
        printf("msgNum: %i ", msgNum);

        int keys;
        charsToInt(buffer, keys, 8);
        printf("keys: %04x\n", keys);

        float speed = 2;
        if (keys & 0x1)
            players[playerId].position.y += speed;
        if (keys & 0x2)
            players[playerId].position.y -= speed;
        if (keys & 0x4)
                players[playerId].position.x -= speed;
        if (keys & 0x8)
            players[playerId].position.x += speed;

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



        pthread_mutex_lock(&commandQueueMutex);
        //commandQueue.push(command);
        pthread_mutex_unlock(&commandQueueMutex);



        /*Send uppercase message back to client, using serverStorage as the address*/
        sendto(udpSocket,outbuffer,pos,0,(struct sockaddr *)&serverStorage,addr_size);
    }

    return 0;
}

void update(sf::Time elapsedTime)
{

}

void processEvents()
{

    while (!commandQueue.empty())
    {
        /*pthread_mutex_lock(&commandQueueMutex);
        char* command = commandQueue.front();
        commandQueue.pop();
        pthread_mutex_unlock(&commandQueueMutex);
        printf("Command: %s\n", command);*/
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
            timeSinceLastUpdate -= TimePerFrame;
            processEvents();
            update(TimePerFrame);

        }

    }

    pthread_join(listeningThread, NULL);

}