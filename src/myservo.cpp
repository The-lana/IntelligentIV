#include "myservo.h"

void setupPWM(){
      // Setup timer and attach timer to a led pin
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
}
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax){
     // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty); 
}
void motorclose(){
    ledcAnalogWrite(LEDC_CHANNEL_0, 60);            //width of pulse set for one directrion
    vTaskDelay(3500/portTICK_PERIOD_MS);
    ledcAnalogWrite(LEDC_CHANNEL_0,0);
}
void motoropen(){
    ledcAnalogWrite(LEDC_CHANNEL_0,120);            //width of pulse determined for oppst direction
    vTaskDelay(3500/portTICK_PERIOD_MS);
    ledcAnalogWrite(LEDC_CHANNEL_0,0);
}
void motorstop(){
    ledcAnalogWrite(LEDC_CHANNEL_0,0);              //to stop the motor rotation from current position
}