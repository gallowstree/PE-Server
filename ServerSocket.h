//
// Created by Alejandro on 18/08/16.
//

#ifndef TEST_SERVER_NETLISTENER_H
#define TEST_SERVER_NETLISTENER_H


#include <netinet/in.h>

class SocketListener
{
public:
    virtual void receiveMessage(char buffer[], size_t nBytes, char * client_ip) = 0;
};


class ServerSocket
{
public:
    ServerSocket(const char* ip, int port, SocketListener* listener);
    void run();

    static void * runThread(void * serverSocket);

private:
    SocketListener* listener;
    const char *ip;
    int port;
};



#endif //TEST_SERVER_NETLISTENER_H
