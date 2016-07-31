//
// Created by Alejandro on 31/07/16.
//

#ifndef TEST_SERVER_SERIALIZATION_H
#define TEST_SERVER_SERIALIZATION_H

#include <cstring>

typedef union f2c {
    float f;
    char  c[4];
} float2Chars;

typedef union i2c {
    int32_t i;
    char c[4];
} int2Chars;

void floatToChars(const float& f, char c[], int position)
{
    float2Chars converter;
    converter.f = f;
    memcpy(c + position, converter.c, sizeof(f) );
}

void charsToFloat(const char c[], float& f, int position)
{
    float2Chars converter;
    memcpy(converter.c, c + position, sizeof(position));
    f = converter.f;
}

void intToChars(const int& i, char c[], int position)
{
    int2Chars converter;
    converter.i = i;
    memcpy(c + position, converter.c, sizeof(i) );
}

void charsToInt(const char c[], int& i, int position)
{
    int2Chars converter;
    memcpy(converter.c, c + position, sizeof(position));
    i = converter.i;
}


#endif //TEST_SERVER_SERIALIZATION_H
