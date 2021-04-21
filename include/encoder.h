#pragma once
#ifndef ENCODER_H
#define ENCODER_H

#define encoder0PinA  25
#define encoder0PinB  26

extern int encoderPos ;
extern volatile int valrotary ;
extern int oldvalrotary ;
extern int oldtime;
extern int newtime;
void doencoder(void);
void IRAM_ATTR encoderbtn(void);
void printandupdate();
#endif