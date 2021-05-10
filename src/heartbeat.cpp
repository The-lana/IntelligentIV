#include "heartbeat.h"
    
#define REPORTING_PERIOD_MS     1000
uint32_t tsLastReport = 0;


PulseOximeter pox;

bool initPulseoximeter(){

    if (!pox.begin()) {
        Serial.println("INIT PULSEOXIMETER FAILED");
        return false;
    } 
    Serial.println("PULSE OXIMETER INITIALIZED");
    return true;
}
void heartbeat_task(void * parameters){

    for(;;){
        pox.update();
        if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
            Serial.print("Heart rate:");
            Serial.print(pox.getHeartRate());
            Serial.print("bpm / SpO2:");
            Serial.print(pox.getSpO2());
            Serial.println("%");

            tsLastReport = millis();
        }
    }

}
