#include "Arduino.h"             //existing lib included
#include "wifinetwork.h"         //wifinetwork.h was defined and added
#include "drop.h"               //drop.h was defined and added
#include "heartbeat.h"
#include "myservo.h"


void setup(){

Serial.begin(115200);     
// connect to wifi
pinMode(CLK,INPUT);         //clk pin deifned as pin 25 in drop.h set as input
pinMode(DT,INPUT);          //dt pin defined as pin 26 in drop.h is set as input
pinMode(SW,INPUT_PULLUP);   //sw pin defined as pin 27 in drop.h is set as pullup


//servoinit();              //servo initialized
setupPWM();



xTaskCreatePinnedToCore(
  keepwifialive,
  "keep_wifi_alive",
  1025*5,
  NULL,
  2,
  NULL,
  CONFIG_ARDUINO_RUNNING_CORE
);


xTaskCreatePinnedToCore(
  displaySerial,
  "print to serial from queue",
  1024*3,
  NULL,
  1,
  NULL,
  CONFIG_ARDUINO_RUNNING_CORE
);

Serial.println("setting up displya");
if(initilizeDisplay()){

  xTaskCreatePinnedToCore(
    displayOLED,
    "display in oled",
    1024*5,
    NULL,
    1,
    NULL,
    CONFIG_ARDUINO_RUNNING_CORE
  );
}else{
  Serial.println("Oled not available");
}

xTaskCreatePinnedToCore(
  doCalculation,
  "calculating values",
  1024*5,
  NULL,
  3,
  &handle_doCalculation,
  CONFIG_ARDUINO_RUNNING_CORE
);


xTaskCreatePinnedToCore(
  displayMenu,
  "menu display",
  1024*3,
  NULL,
  1,
  &handle_displayMenu,
  CONFIG_ARDUINO_RUNNING_CORE
);

vTaskSuspend(handle_displayMenu);

if (handle_doCalculation!=NULL && handle_displayMenu!=NULL)
{
  xTaskCreatePinnedToCore(
  btnTask,
  "handle btn",
  1024*4,
  NULL,
  2,
  NULL,
  CONFIG_ARDUINO_RUNNING_CORE
);

}
else{
  Serial.println("handle is null");
}

xTaskCreatePinnedToCore(
  keepMQTTConnected,
  "keep mqtt connection active",
  1024*2,
  NULL,
  2,
  NULL,
  CONFIG_ARDUINO_RUNNING_CORE
); 

if(initPulseoximeter()){
  xTaskCreatePinnedToCore(
    heartbeat_task,
    "pulse oximeter task",
    1024*5,
    NULL,
    2,
    NULL,
    CONFIG_ARDUINO_RUNNING_CORE
  );
}


xTaskCreatePinnedToCore(
  mqttTask,
  "send items to mqtt",
  1024*5,
  NULL,
  2,
  NULL,
  CONFIG_ARDUINO_RUNNING_CORE
);


attachInterrupt(digitalPinToInterrupt(IRPIN),dropInterrupt,FALLING);
}

void loop(){

vTaskDelete(NULL);
// testing git push
}