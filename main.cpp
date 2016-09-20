#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <SFML/Graphics.hpp>
#include "Game.h"

int main()
{

    Game g;

    ServerSocket s("127.0.0.1", 50420, &g);
    //ServerSocket s("192.168.1.117", 50420, &g);



    pthread_t listening_thread;
    pthread_create(&listening_thread, nullptr, &ServerSocket::runThread, &s);

    while (true)
    {
        g.run();
        g.reset();
    }


    pthread_join(listening_thread, nullptr);
    return 0;
}