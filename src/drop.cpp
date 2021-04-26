#include "drop.h"


int interuptDropCount=0;  //count number of drops inside the interrupt
float interuptDropTime=1; // keep track of time
long long int interuptOldDropTime=0; // keeping old time to check difference
int protectedDropCount=0; // drop count protected from interrupts
float protectedDropTime=1; // drop time protected form interrupts
int oldDropcount = 0;// to check for changes in dropcount
uint8_t dropfactor = 20; //  drop factor of the iv set
int driprate = 0; // rate of flow
int volumeinfused = 0;
int volumetobeinfused = 1000;
uint8_t btnCount = 0;// count number of times button is pressed.
int8_t menucount = 0;
int8_t oldmenuCount = 1;
QueueHandle_t serialqueue = xQueueCreate(5,sizeof(char)*50) ;
TaskHandle_t handle_doCalculation = NULL;
TaskHandle_t handle_displayMenu = NULL;
uint8_t aState=0;
uint8_t aLastState=0;
int encoderCounter=0;
int oldencodervalue = 0;
static TimerHandle_t handle_timer = xTimerCreate(
  "menu hold timer",
  30000/portTICK_PERIOD_MS,
  pdFALSE,
  (void*)0,
  timercallback);






char menu[3][15] = {
    "Drop rate",
    "Drip factor",
    "mL infusion"
};


void timercallback(TimerHandle_t xtimer){
    btnCount = 0 ; 
    vTaskSuspend(handle_displayMenu);
    vTaskResume(handle_doCalculation);
}

    

/**
 * @brief interrupt to handle drop counting.
 * 
 */
void IRAM_ATTR dropInterrupt(void){
    int newtime = millis();
    if(newtime-interuptOldDropTime > 200){
        interuptDropTime = (newtime - interuptOldDropTime)/100;
        interuptDropCount++;
        interuptOldDropTime = newtime;
    }

}


/**
 * @brief task to do calculations to find drop rate and 
 * ml infused.
 * 
 * @param parameters 
 */

void doCalculation(void * parameters){
for(;;){
    // stopping interrupts to copy over data without changes.
    noInterrupts();
    protectedDropCount = interuptDropCount;
    protectedDropTime = interuptDropTime;
    //renable interrupts
    interrupts();
    char buffer[100];
    //check if there is any change in number of drops.
    if(protectedDropCount != oldDropcount){
        volumeinfused = protectedDropCount/dropfactor;
        driprate = 3600/(dropfactor*protectedDropTime);
        snprintf(buffer,50,"volume infused : %d  drip rate : %d \n",volumeinfused,driprate);
        if(xQueueSend(serialqueue,&buffer,0) == pdFALSE){
            Serial.println("queue full");
        }
        if(xQueueSend(mqttqueue,&buffer,0) == pdFALSE){
            Serial.println("queue full");
        }
        oldDropcount = protectedDropCount;
    }

    vTaskDelay(200/portTICK_PERIOD_MS);
 }

}

//task to handle btn event

void btnTask(void * parameters){
    for(;;){

        if(!digitalRead(SW)){
            Serial.println("in btn task");
            btnCount++;
            //Serial.println("count incremented");
            xTimerStart(handle_timer,portMAX_DELAY);
            //Serial.println("timer started");
            if (handle_doCalculation!=NULL && handle_displayMenu!=NULL){
            vTaskSuspend(handle_doCalculation);
           // Serial.println("suspend do calculation");
            vTaskResume(handle_displayMenu);
            //Serial.println("resume displaymenu");
            }else{
            //Serial.println("handle null in cpp");
            }
        }
        //Serial.println("going to sleep");
        vTaskDelay(200/portTICK_PERIOD_MS);
    }



}


void displaySerial(void * parameters){
    for(;;){
        char buffer[100];
        if(xQueueReceive(serialqueue,(void*) &buffer,0)==pdTRUE){
        Serial.println(buffer);
        }
        vTaskDelay(50/portTICK_PERIOD_MS);
    }


}


/**
 * @brief 
 * 
 * funtion to read encoder state and returns 1 if CW , -1 for CCW and 0 for no change.
 * 
 * @return int 
 */

void doencoder(){
   aState = digitalRead(CLK); // Reads the "current" state of the outputA
   // If the previous and the current state of the outputA are different, that means a Pulse has occured
   if (aState != aLastState){     
     // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
     if (digitalRead(DT) == aState) { 
       encoderCounter++;
       //Serial.println("encoder incremented");
     } else {
       encoderCounter--;
       //Serial.println("encoder decremneted");
     }
   } 
   aLastState = aState; // Updates the previous state of the outputA with the current state
   
 }

void displayMenu(void * parameters){
    for(;;){
        if(btnCount){
        doencoder();
        if(encoderCounter- oldencodervalue > 1){
            menucount++;
            if(menucount > 2) {menucount = 0;}
            oldencodervalue = encoderCounter;
        }
        if(encoderCounter - oldencodervalue < -1){
            menucount--;
            if(menucount < 0){menucount = 2;}
            oldencodervalue = encoderCounter;
        }
        
        if(menucount!= oldmenuCount){
        if(xQueueSend(serialqueue,&menu[menucount],0)==pdFALSE){
            Serial.println("queue full");
        }
        oldmenuCount = menucount;
        }

        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }


}