#include "drop.h"


int interuptDropCount=0;  //count number of drops inside the interrupt
float interuptDropTime=1; // keep track of time
long long int interuptOldDropTime=0; // keeping old time to check difference
int protectedDropCount=0; // drop count protected from interrupts
float protectedDropTime=1; // drop time protected form interrupts
int oldDropcount = 0;// to check for changes in dropcount
IV_Type iv;         //struct that contains dropfactor,driprate,volinf and vol to be inf
uint8_t btnCount = 0;// count number of times button is pressed.
int8_t menucount = 0;
int8_t oldmenuCount = 1;
QueueHandle_t serialqueue = xQueueCreate(5,sizeof(char)*50) ;   //create serial que with length 5 of 50 chars
QueueHandle_t displayqueue = xQueueCreate(5,sizeof(char)*50) ;
TaskHandle_t handle_doCalculation = NULL;
TaskHandle_t handle_displayMenu = NULL;
uint8_t aState=0;
uint8_t aLastState=0;
int encoderCounter=0;       //to find how  much the encoder has turned    
int oldencodervalue = 0;    //storing old value for comparison
int driprateset=0,dripfactorset=0,mlinfused=0;
int temp=0;
bool flowStatus = true;
//static bool oldflowstatus = true;
Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);
//Servo servo1;


/**
 * @brief timer to automatically close .
 * 
 */
static TimerHandle_t handle_timer = xTimerCreate(
  "menu hold timer",
  60000/portTICK_PERIOD_MS,
  pdFALSE,
  (void*)0,
  timercallback);


//set menu content as 2d array

char menu[3][15] = {
    "Drop rate",
    "Drip factor",
    "mL infusion"
};


void timercallback(TimerHandle_t xtimer){       
    //continue with do calculation task after the menu is diplayed for a paricular period
    btnCount = 0 ; 
    vTaskSuspend(handle_displayMenu);
    vTaskResume(handle_doCalculation);
}

    

/**
 * @brief interrupt to handle drop counting.
 * 
 */
void IRAM_ATTR dropInterrupt(void){
    int newtime = millis();                         //time when drop falls
    if(newtime-interuptOldDropTime > 200){          //to reduce one drop to be counted more than once,the drop is counted only if the time difference is greater than 200ms
        interuptDropTime = (newtime - interuptOldDropTime)/10;      //time between drops is stored..for calculating ml per hr
        interuptDropCount++;                        //count drop
        interuptOldDropTime = newtime;              //set olddroptime for next cycle
    }

}


/**
 * @brief task to do calculations to find drop rate and 
 * ml infused.
 * 
 * @param parameters 
 */

void doCalculation(void * parameters){
iv.driprate = 0;
iv.dropfactor = 20;
iv.volumeinfused = 0;
iv.volumetobeinfused = 500;
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
        iv.volumeinfused = protectedDropCount/iv.dropfactor;
        iv.driprate = 360000/(iv.dropfactor*protectedDropTime);     //calculate vol inf from dropcount using dropfactor
        snprintf(buffer,50,"volume infused : %d  drip rate : %d \n",iv.volumeinfused,iv.driprate);      
        //data is sent to buffer in format
        if(xQueueSend(serialqueue,&buffer,0) == pdFALSE){
            //xQueuesend reutrns FALSE if buffer was not placed on serialque //0 is number of ticks to wait for space
            Serial.println("Serial queue full");
        }
        if(xQueueSend(mqttqueue,&iv,0) == pdFALSE){
            //xQueuesend reutrns FALSE if iv was not placed on mqttque //0 is number of ticks to wait for space
            Serial.println("mqtt queue full");
        }
        snprintf(displaybuffer,50,"%d ml/hr",iv.driprate);
        //data is sent to display buffer in format
        if(xQueueSend(displayqueue,&displaybuffer,0) == pdFALSE){
            //xQueuesend reutrns FALSE if displaybuffer was not placed on displayque //0 is number of ticks to wait for space
            Serial.println("display queue full");
        }
        oldDropcount = protectedDropCount;      //write previous value
        
    }
    
    if (ENABLE_PID) doPID();
    flowstop();          //why is this here??
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
           // Serial.println("suspend do calculation");//calcs are stopped and menu is given priority while interrupt works
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
        //recieves buffer value that contains driprate and vol inf
        if(xQueueReceive(serialqueue,(void*) &buffer,0)==pdTRUE){
        Serial.println(buffer);
        }
        vTaskDelay(50/portTICK_PERIOD_MS);
    }


}


void displayOLED(void * parameters){
    for(;;){
        char buffer[50];
        //receives values from display que containing driprate or 
        if(xQueueReceive(displayqueue,(void*)&buffer,0)==pdTRUE){
            Serial.print("sending to display: ");
            Serial.print(buffer);
            display.clearDisplay();
            display.setCursor(0,0);
            display.println(buffer);        //displaying buffer value
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
int setvalue(int temp) //to set value of driprate/droprate/vol using a temp variable for repetition
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
        if(btnCount==1){                    //on first button count menu is displayed
        doencoder(); // menu change
        if(encoderCounter- oldencodervalue > 1){        //to check for rotation in one direction
            menucount++;                               
            if(menucount > 2) {menucount = 0;}          //limit count from 0-2
            oldencodervalue = encoderCounter;

           
        }
        if(encoderCounter - oldencodervalue < -1){      //check for rotation in opp dir
            menucount--;
            if(menucount < 0){menucount = 2;}           //limiting count in 0-2
            oldencodervalue = encoderCounter;       //set previous value for next cycle

            //
        }
         
        if(menucount!= oldmenuCount){
       /* if(xQueueSend(serialqueue,&menu[menucount],0)==pdFALSE){
            Serial.println("queue full");
        }*/
        if(xQueueSend(displayqueue,&menu[menucount],0) == pdFALSE){     //send the menu strings through displayque to be displayed
            Serial.println("display queue full");
        }
        oldmenuCount = menucount;
        }

        }

      
         if(btnCount==2)        //on second buttoncount values can be set depending on the value of menucount during press of button
             switch(menucount)
            {
                case 0 : //driprate
                {
                    //encoderCounter=driprateset;
                    
                    doencoder();
                    if(encoderCounter<0)        //no neg values required
                        encoderCounter=0;
                    if(encoderCounter!=oldencodervalue){                    //check if encoder has been rotated
                        driprateset=setvalue(driprateset);
                        
                    snprintf(buffer,15,"%d ml/hr",driprateset);            //send value to buffer
                    if(xQueueSend(displayqueue,&buffer,0)==pdFALSE){       //check for space in display que to place new value 
                    Serial.println("queue full");
                 }
                    oldencodervalue = encoderCounter;                      //update oldencoder value for next loop
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
         if(btnCount==3){       //on buttoncount 3 the value is confirmed and an output is shown on display
             switch(menucount)      //based on earlier menucount value
            {
                case 0 : //driprate
                {
                
                    driprateset = encoderCounter;
                    snprintf(buffer,40,"drip rate set as %d ml/hr",driprateset);        //sending value to displayque
                    if(xQueueSend(displayqueue,&buffer,0)==pdFALSE){
                         Serial.println("queue full");
                 }
                    Serial.println("drop rate set");
                    //old drip can be send to setencounter if necessery
                }break;
                case 1 : //dropfactor
                { 
                    Serial.println("dropfactor set");
                    iv.dropfactor = encoderCounter;
                    snprintf(buffer,40,"dropfactor set as %d ml/hr",iv.dropfactor);         //sending to displayque
                    if(xQueueSend(displayqueue,&buffer,0)==pdFALSE){
                         Serial.println("queue full");
                 }
                }break;
                case 2 :  //mlinfusion
                {
                    Serial.println("volume to be infused is set");
                    iv.volumetobeinfused =encoderCounter;
                    snprintf(buffer,40,"volumetobeinfused set as %d ml/hr",iv.volumetobeinfused*100);
                    if(xQueueSend(displayqueue,&buffer,0)==pdFALSE){
                         Serial.println("queue full");
                 }
                }break;
            } 
            btnCount = 0;           //reset buttoncount as 0
            vTaskDelay(portTICK_PERIOD_MS);
            vTaskResume(handle_doCalculation);

            vTaskSuspend(NULL);
         }
            vTaskDelay(20/portTICK_PERIOD_MS);
    }


}

/**
 * @brief initialize the display.
 * 
 * @return true if successful.
 * @return false if unable to initialize display.
 */
bool initilizeDisplay(){
    
    if(!display.begin(SSD1306_SWITCHCAPVCC,0x3C)){          //check if display is working
        Serial.println("false");
        return false;
         }
    
    display.clearDisplay();                 //clear previouus content
    display.setTextColor(WHITE);
    display.setTextSize(1);                 //set text size
    display.setCursor(0,0);                 //set cursor to beginning 
    display.println("display working");     //display on screen to confirm working
    Serial.println("display setup done");   //display on serial monitor to confirm working
    display.display();
    //delay(500);
    return true;

}

<<<<<<< HEAD
/*
void flowstop(){
=======

/*void flowstop(){
>>>>>>> pid

    if(flowStatus!=oldflowstatus) {
        if(flowStatus){
        motoropen();
             }
    else    {
        motorclose();
    }
    oldflowstatus = flowStatus;
}
<<<<<<< HEAD
}
*/
=======
<<<<<<< HEAD
} */
=======
}

>>>>>>> asim/PID
>>>>>>> pid
