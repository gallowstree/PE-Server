//
// Created by Alejandro on 31/07/16.
//

#ifndef TEST_SERVER_COMMAND_H
#define TEST_SERVER_COMMAND_H

typedef struct commamd
{
    int16_t playerId;
    int16_t commandType;
    int16_t team;
    int msgNum;
    int controls;
    float rotation;
    const char * client_ip;
    char * nickname;
    int numberOfAcks;
    int32_t *messageAcks;
} command_t;




#endif //TEST_SERVER_COMMAND_H
