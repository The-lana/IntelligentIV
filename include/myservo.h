#pragma once
#ifndef MYSERVO_H
#define MYSERVO_H

#include "Arduino.h"

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     50

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN            19


void setupPWM();
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255);
void motorclose();
void motoropen();

#endif