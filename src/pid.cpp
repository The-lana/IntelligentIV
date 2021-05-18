#include "pid.h"

////////////////////////Variables///////////////////////
int Read = 0;
extern int driprateset;
extern IV_Type iv;
float flowrate = 0.0;     //flowrate initial
float elapsedTime, times = millis(), timePrev;        //Variables for time control
float flowrate_previous_error, flowrate_error;
int period = 50;  //Refresh rate period of the loop is 50ms
///////////////////////////////////////////////////////


///////////////////PID constants///////////////////////
float kp=8; //Mine was 8
float ki=0.2; //Mine was 0.2
float kd=3100; //Mine was 3100
float flowrate_setpoint = driprateset;           //Should be the distance from sensor to the middle of the bar in mm
                                        //rotor value
float PID_p, PID_i, PID_d, PID_total;
///////////////////////////////////////////////////////


void doPID() {
  if (millis() > times+period)
  {
    times = millis();    
    flowrate = iv.driprate;           // calc flowrate
    flowrate_error = driprateset - flowrate;    // determine error- error = Setpoint - inp;           
    PID_p = kp * flowrate_error;
   //float flow_diference = flowrate_error - flowrate_previous_error;     
    PID_d = kd*((flowrate_error - flowrate_previous_error)/period);   // compute derivative
      
    if(-3 < flowrate_error && flowrate_error < 3)     //PID_i
    {
      PID_i = PID_i + (ki * flowrate_error);
    }
    else
    {
      PID_i = 0;
    }
  
    
    PID_total = PID_p + PID_i + PID_d;  

/*
    PID_total = map(PID_total, -150, 150, 0, 150);  //pin,
  
    if(PID_total < 20){PID_total = 20;}
    if(PID_total > 160) {PID_total = 160; } 
 */ 
    if(PID_total>0)
    motorclose(PID_total);
    if(PID_total<0) 
    motoropen(PID_total*(-1));// set rotation time according to PID
    flowrate_previous_error = flowrate_error;
  }
}

