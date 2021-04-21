#pragma once
#ifndef MENU_H
#define MENU_H
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include<Adafruit_GFX.h>



#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 128
#define TEXT_SIZE_USED 2

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

enum MENU{FUNCTION_SELECTION, FUNCTION_SET};
enum FUNCTION_SELECTION_MENU{DRIP_RATE,DROP_FACTOR,ALARM_LEVEL};

extern enum MENU CurrentMenu;
extern enum FUNCTION_SELECTION_MENU CurrentFuntion;

void checkIIC();
void initialize();
void drawMenu();
void drawWords(int , char* );
void drawInvertedWords(int, char*);



#endif