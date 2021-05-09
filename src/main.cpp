
#include "Arduino.h"
#include "wifinetwork.h"
#include "drop.h"

void setup(){

Serial.begin(9600);
// connect to wifi
pinMode(CLK,INPUT);
pinMode(DT,INPUT);
pinMode(SW,INPUT_PULLUP);


servoinit();




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


xTaskCreatePinnedToCore(
  mqttTask,
  "send items to mqtt",
  1024*5,
  NULL,
  2,
  NULL,
  CONFIG_ARDUINO_RUNNING_CORE
);


attachInterrupt(digitalPinToInterrupt(IRPIN),dropInterrupt,HIGH);

}

void loop(){

vTaskDelete(NULL);
// testing git push
}