#pragma once
#ifndef AIRPLAY_VFD_H
#define AIRPLAY_VFD_H

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <map>
#include <utility>
#include <iostream>
#include <cstring>

// Define a type for the two bytes
typedef std::pair<unsigned int,unsigned int> TwoBytes;
extern std::map<char, TwoBytes> asciiMap;

unsigned int sendLSB(unsigned int orig);
void printBinary(int value, int numBits);
void vfdSend(uint8_t cmd);
void writeChar(char c);
void writeString(const char *s);
void scrollString(const char *s);
void scrollLargeString(const char *s, size_t origLen, int maxIdx);
void scrollShortString(const char *s);

// display pins Chip Enable
#define VFD_STB 27 // GPIO pin 16

#define VFD_MAXLENGTH 10
extern int lastMoveIdx;

#define CHANNEL 0 // SPI channel (0 or 1)
#define SPEED   500000 // SPI speed in Hz

#endif

