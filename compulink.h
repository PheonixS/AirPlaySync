#pragma once
#ifndef AIRPLAY_COMPULINK_H
#define AIRPLAY_COMPULINK_H

#include <iostream>
#include <wiringPi.h>

#define D1 25 // GPIO pin 26
#define POWER_ON 0xC0
#define USE_CD 0xA3

void wordEnd();
void writeHigh();
void writeLow();
void sendCommand(unsigned char hexCode);

#endif
