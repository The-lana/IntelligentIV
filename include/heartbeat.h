#pragma once
#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include"Arduino.h"
#include "MAX30100_PulseOximeter.h"
#include <Wire.h>

typedef struct PULSEOXI{

    uint16_t heartRateAvg;
    uint16_t O2satAvg;
}PulseOxi_type;


extern QueueHandle_t pulseoxiqueue;

bool initPulseoximeter();
void heartbeat_task(void * parameters);


#endif