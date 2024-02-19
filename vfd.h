#pragma once
#ifndef AIRPLAY_VFD_H
#define AIRPLAY_VFD_H

#include "SPI.h"

#include <map>
#include <utility>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdint>

// Define a type for the two bytes
typedef std::pair<unsigned int,unsigned int> TwoBytes;
extern std::map<char, TwoBytes> asciiMap;

uint8_t sendLSB(uint8_t orig);
void vfdSend(SPI spi, uint8_t cmd);
void scrollString(SPI spi, std::string s);

#define VFD_MAXLENGTH 10
extern int lastMoveIdx;

#endif
