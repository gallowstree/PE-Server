//
// Created by Alejandro on 21/08/16.
//

#ifndef TEST_SERVER_OUTPUTSOCKET_H
#define TEST_SERVER_OUTPUTSOCKET_H


#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>

class OutputSocket
{
public:
    OutputSocket(const char* ip, int port);
    void send(const char *outbuffer, size_t size, int32_t reliableId = -1) const;
    const char* ip;
private:
    int port;
    int udpSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size;
};


#endif //TEST_SERVER_OUTPUTSOCKET_H
