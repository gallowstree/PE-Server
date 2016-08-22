//
// Created by Alejandro on 21/08/16.
//

#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <errno.h>
#include "OutputSocket.h"

OutputSocket::OutputSocket(const char *ip, int port) :
ip(ip),
port(port)
{
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    addr_size = sizeof serverAddr;
}

void OutputSocket::send(const char *outbuffer, size_t size, int32_t reliableId) const
{
    sendto(udpSocket,outbuffer,size,0,(struct sockaddr *)&serverAddr, addr_size);
    //printf("Sending %s!\n",strerror(errno));
    /*
    if (reliableId != -1)
    {
        //Copiar el buffer a uno confiable
        char* copyBuffer = (char *)malloc(size);
        memcpy(copyBuffer, outbuffer, size);

        //Might need unique_ptrs
        reliable_message_t message;
        message.buffer = copyBuffer;
        message.messageId = reliableId;
        //Insertar mensaje confiable
        //reliable_queue.insert(std::pair<int32_t, reliable_message_t*>(reliableId, &message));
        reliable_queue[reliableId] = message;
    }

    for (auto const& message: reliable_queue)
    {
        if (message.first != reliableId)
        {
            sendto(udpSocket, message.second.buffer, message.second.size, 0,(struct sockaddr *)&serverAddr, addr_size);
        }
    }*/
}



