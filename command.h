//
// Created by Alejandro on 31/07/16.
//

#ifndef TEST_SERVER_COMMAND_H
#define TEST_SERVER_COMMAND_H

typedef struct commamd
{
    int16_t playerId;
    int16_t commandType;
    int msgNum;
    int controls;
} command_t;




#endif //TEST_SERVER_COMMAND_H