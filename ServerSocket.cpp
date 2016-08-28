//
// Created by Alejandro on 18/08/16.
//

#include <arpa/inet.h>
#include <cstring>
#include "ServerSocket.h"
#include <stdio.h>
#include <errno.h>

ServerSocket::ServerSocket(const char *ip, int port, SocketListener *listener) :
ip(ip),
port(port),
listener(listener)
{

}

void ServerSocket::run()
{
    int udpSocket, COMMAND_BUFFER_SIZE = 1024;
    ssize_t nBytes;
    char buffer[COMMAND_BUFFER_SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage;
    socklen_t addr_size;
    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof clientAddr;

    while(1)
    {
        int16_t commandType;
        nBytes = recvfrom(udpSocket, buffer, COMMAND_BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &addr_size);

        if (listener != nullptr)
        {
            listener->receiveMessage(buffer, nBytes, &clientAddr);
        }
    }
}

void *ServerSocket::runThread(void *serverSocket) {
    ((ServerSocket*) serverSocket)->run();
    return nullptr;
}





