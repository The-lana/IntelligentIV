#include <Arduino.h>
#include "menu.h"
#include "encoder.h"


int currentSelection = 0;
int incomingByte = 0;


void setup(){
  Serial.begin(9200);
  checkIIC();
  initialize();
  pinMode(encoder0PinB,INPUT);
  pinMode(encoder0PinA,INPUT);
  pinMode(27,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoder0PinA),doencoder,CHANGE);
  attachInterrupt(digitalPinToInterrupt(27),encoderbtn,FALLING);
  drawMenu();
}


void loop(){
  //test commment 
  printandupdate();
  
  
}