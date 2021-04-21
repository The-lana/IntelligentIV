#include "Arduino.h"
#include "encoder.h"
#include "menu.h"

int encoderPos = 0;
volatile int valrotary = 0;
int oldvalrotary = 0;
int newtime;
int oldtime =0;
int oldtimebtn=0;
int newtimebtn;
int setValue = 0;
int counter = 0;
extern Adafruit_SSD1306 display;
int currentposition = 0;

char *MenuElement[3]{
  "Drop factor",
  "Drip rate",
  "Alarm level"
};


void IRAM_ATTR encoderbtn(void){
newtimebtn = millis();
if( newtimebtn-oldtimebtn> 500){
Serial.println("button pressed");
if (CurrentMenu==FUNCTION_SELECTION){
  CurrentMenu = FUNCTION_SET;
  return ;
}
if(CurrentMenu==FUNCTION_SET){
  setValue = 1;
  CurrentMenu = FUNCTION_SELECTION;
}
}
oldtimebtn= newtimebtn;
}

void IRAM_ATTR doencoder(void){
  
  newtime = millis();

  if(newtime-oldtime > 50){
    Serial.println("interrupted. and counted");
  int outA = digitalRead(encoder0PinA);
  int outB = digitalRead(encoder0PinB);
  if (outA == outB)
  {
	encoderPos++;
  }
  else
  {
	encoderPos--;
  }
  valrotary = encoderPos;

  oldtime = newtime;
  }
  
}

void printandupdate(){
  if(valrotary!=oldvalrotary&&CurrentMenu==FUNCTION_SELECTION){

    if(valrotary > oldvalrotary){
      display.clearDisplay();
      currentposition++;
      if(currentposition > 2)
          currentposition = 0;
    drawWords(0,MenuElement[currentposition]);   
  }
  else if(valrotary < oldvalrotary){
    display.clearDisplay();
    currentposition--;
    if(currentposition < 0)
      currentposition = 2;
    drawWords(0,MenuElement[currentposition]);
  }
  oldvalrotary = valrotary;
  }
   if(valrotary!=oldvalrotary&&CurrentMenu==FUNCTION_SET){
    char buffer[5];
    if(valrotary > oldvalrotary)
        counter++;
    else if(valrotary < oldvalrotary)
        counter--;
    
    display.clearDisplay();
    snprintf(buffer,5,"%d",counter);
    drawWords(1,buffer);
    oldvalrotary = valrotary;
   }


}
void moveMenu(){


}