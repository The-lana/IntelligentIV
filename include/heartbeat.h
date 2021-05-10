#pragma once
#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include"Arduino.h"
#include "MAX30100_PulseOximeter.h"
#include <Wire.h>

bool initPulseoximeter();
void heartbeat_task(void * parameters);


#endif