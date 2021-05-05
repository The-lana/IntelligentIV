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
QueueHandle_t displayqueue = xQueueCreate(5,sizeof(char)*50) ;
TaskHandle_t handle_doCalculation = NULL;
TaskHandle_t handle_displayMenu = NULL;
uint8_t aState=0;
uint8_t aLastState=0;
int encoderCounter=0;
int oldencodervalue = 0;
int driprateset=0,dripfactorset=0,mlinfused=0;
int temp=0;
bool flowStatus = true;


Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);
Servo servo1;


/**
 * @brief timer to automatically close .
 * 
 */
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
        interuptDropTime = (newtime - interuptOldDropTime)/10;
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
    char displaybuffer[50];
    //check if there is any change in number of drops.
    if(protectedDropCount != oldDropcount){
        volumeinfused = protectedDropCount/dropfactor;
        driprate = 360000/(dropfactor*protectedDropTime);
        snprintf(buffer,50,"volume infused : %d  drip rate : %d \n",volumeinfused,driprate);
        if(xQueueSend(serialqueue,&buffer,0) == pdFALSE){
            Serial.println("Serial queue full");
        }
        if(xQueueSend(mqttqueue,&buffer,0) == pdFALSE){
            //Serial.println("mqtt queue full");
        }
        snprintf(displaybuffer,50,"%d ml/hr",driprate);
        if(xQueueSend(displayqueue,&displaybuffer,0) == pdFALSE){
            Serial.println("display queue full");
        }
        oldDropcount = protectedDropCount;
        
    }
    flowstop();
    vTaskDelay(200/portTICK_PERIOD_MS);
 }

}

//task to handle btn event

void btnTask(void * parameters){
    for(;;){

        if(!digitalRead(SW)){
            Serial.println("in btn task");
            btnCount++;
            Serial.println(btnCount);
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

/**
 * @brief task to print queue elements to serial terminal.
 * 
 * @param parameters 
 */
void displaySerial(void * parameters){
    for(;;){
        char buffer[100];
        if(xQueueReceive(serialqueue,(void*) &buffer,0)==pdTRUE){
        Serial.println(buffer);
        }
        vTaskDelay(50/portTICK_PERIOD_MS);
    }


}


void displayOLED(void * parameters){
    for(;;){
        char buffer[50];
        if(xQueueReceive(displayqueue,(void*)&buffer,0)==pdTRUE){
            Serial.print("sending to display: ");
            Serial.print(buffer);
            display.clearDisplay();
            display.setCursor(0,0);
            display.println(buffer);
            display.display();

        }

    vTaskDelay(20/portTICK_PERIOD_MS);
    }
}

/**
 * @brief 
 * 
 * funtion to read encoder and increment a counter.
 * 
 *  
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

//ente pokirtharam
//entem
int setvalue(int temp)
{
    if(encoderCounter>temp){
         temp++;
         if(temp>999)
           temp=0;
     }
     else{
         temp--;
        if(temp<0)
            temp=0;
     }
     return temp;
}

void displayMenu(void * parameters){
    for(;;){
        char buffer[12];
        if(btnCount==1){
        doencoder(); // menu change
        if(encoderCounter- oldencodervalue > 1){
            menucount++;
            if(menucount > 2) {menucount = 0;}
            oldencodervalue = encoderCounter;

           
        }
        if(encoderCounter - oldencodervalue < -1){
            menucount--;
            if(menucount < 0){menucount = 2;}
            oldencodervalue = encoderCounter;

            //
        }
        
        if(menucount!= oldmenuCount){
       /* if(xQueueSend(serialqueue,&menu[menucount],0)==pdFALSE){
            Serial.println("queue full");
        }*/
        if(xQueueSend(displayqueue,&menu[menucount],0) == pdFALSE){
            Serial.println("display queue full");
        }
        oldmenuCount = menucount;
        }

        }

      
         if(btnCount==2)
             switch(menucount)
            {
                case 0 : //driprate
                {
                    //encoderCounter=driprateset;
                    
                    doencoder();
                    if(encoderCounter<0)
                        encoderCounter=0;
                    if(encoderCounter!=oldencodervalue){
                        driprateset=setvalue(driprateset);
                        
                    snprintf(buffer,15,"%d ml/hr",driprateset);
                    if(xQueueSend(displayqueue,&buffer,0)==pdFALSE){
                    Serial.println("queue full");
                 }
                    oldencodervalue = encoderCounter;
             }
                    
                    
                    //old drip can be send to setencounter if necessery
                }break;
                case 1 : //dripfactor
                { 
                   //oldencodervalue=oldencodervalue;
                    doencoder();
                    if(encoderCounter<0)
                        encoderCounter=0;
                    if(oldencodervalue != encoderCounter){
                        dripfactorset=setvalue(dripfactorset);
                    snprintf(buffer,15,"%d ml/drop",dripfactorset);
                    if(xQueueSend(displayqueue,&buffer,0)==pdFALSE){
                         Serial.println("queue full");
                 }
                    oldencodervalue = encoderCounter;
                    }
                }break;
                case 2 :  //mlinfusion
                {
                   // oldencodervalue=volumetobeinfused;
                    doencoder();
                    if(encoderCounter<0)
                        encoderCounter=0;
                    if(oldencodervalue != encoderCounter){
                        mlinfused=setvalue(mlinfused);
                    snprintf(buffer,15,"%d ml",mlinfused*100);
                    if(xQueueSend(displayqueue,&buffer,0)==pdFALSE){
                         Serial.println("queue full");
                    }
                     oldencodervalue = encoderCounter;
                    }
                }break;
            }
         if(btnCount==3){
             switch(menucount)
            {
                case 0 : //driprate
                {
                
                    driprateset = encoderCounter;
                    display.println("drop rate set");
                    //old drip can be send to setencounter if necessery
                }break;
                case 1 : //dripfactor
                { 
                    display.println("dripfactor set");
                    dropfactor = encoderCounter;
                }break;
                case 2 :  //mlinfusion
                {
                    display.println("volume to be infused is set");
                    volumetobeinfused =encoderCounter;
                }break;
            } 
            btnCount = 0;
         }
            vTaskDelay(35/portTICK_PERIOD_MS);
    }


}

/**
 * @brief initialize the display.
 * 
 * @return true if successful.
 * @return false if unable to initialize display.
 */
bool initilizeDisplay(){
    
    if(!display.begin(SSD1306_SWITCHCAPVCC,0x3C)){
        Serial.println("false");
        return false;
         }
    
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("display working");
    Serial.println("display setup done");
    display.display();
    //delay(500);
    return true;

}


void servoinit(){

    servo1.attach(SERVOPIN);

}

void flowstop(){
    if(flowStatus) {
        servo1.write(0);
    }
    else {
        servo1.write(60);
    }
}

