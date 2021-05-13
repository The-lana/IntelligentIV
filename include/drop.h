#pragma once
#ifndef DROP_H
#define DROP_H

#include "Arduino.h"
#include  "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "Servo.h"
#include "IV.h"
#include "myservo.h"
#define IRPIN 13
#define CLK 25
#define DT 26 
#define SW 27
#define SERVOPIN 19

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET -1



extern TaskHandle_t handle_doCalculation;
extern TaskHandle_t handle_displayMenu;
extern QueueHandle_t mqttqueue;
extern bool flowStatus;


void IRAM_ATTR dropInterrupt(void);
void doCalculation(void * parameters);
void btnTask(void * parameters);
void displayMenu(void * parameters);
void timercallback(TimerHandle_t xtimer);
void displaySerial(void * parameters);
bool initilizeDisplay();
void displayOLED(void * parameters);
void servoinit();
void flowstop();
//void createQueue();





#endif