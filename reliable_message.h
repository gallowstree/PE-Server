//
// Created by Alejandro on 8/08/16.
//

#ifndef TEST_SERVER_RELIABLE_MESSAGE_H
#define TEST_SERVER_RELIABLE_MESSAGE_H

#include <stdint.h>
#include <cstdlib>

typedef struct reliable_message
{
    int32_t messageId;
    const char* buffer;
    size_t size;
} reliable_message_t;

#endif //TEST_SERVER_RELIABLE_MESSAGE_H
