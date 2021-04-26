#pragma once
#ifndef DROP_H
#define DROP_H

#include "Arduino.h"

#define IRPIN 13
#define CLK 25
#define DT 26 
#define SW 27


extern TaskHandle_t handle_doCalculation;
extern TaskHandle_t handle_displayMenu;
extern QueueHandle_t mqttqueue;


void IRAM_ATTR dropInterrupt(void);
void doCalculation(void * parameters);
void btnTask(void * parameters);
void displayMenu(void * parameters);
void timercallback(TimerHandle_t xtimer);
void displaySerial(void * parameters);
//void createQueue();



#endif