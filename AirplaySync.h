#pragma once
#ifndef AIRPLAY_M_H
#define AIRPLAY_M_H

#include "vfd.h"
#include "jvc.h"
#include "compulink.h"

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <utility>

#include <cstring>
#include <iostream>

// standby led pin front panel
#define LED_PIN 22 //GPIO pin 6
#define LED_STANDBY LOW
#define LED_READY HIGH

// controls power on power source
#define POWER_SOURCE_PIN 2 //GPIO 27

// control front panel power control
#define V3_3_CTRL 26 // GPIO pin 12

#define LED_BUILTIN 0 //GPIO 17
// pin to read button array
#define ADC_PIN_ARRAY 25 // GPIO pin 26

// track status of Power
bool poweredOn = false;

const int scrollDelayMs = 1500;
unsigned long lastMoveTime = 0;

std::string getPressed(float voltage);

void handleADC();

#endif
