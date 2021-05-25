#include "wifinetwork.h"
#include "myservo.h"


const char* ID = "DEVICE1";
const char* DRIPRATE_TOPIC =  "DEVICE1/DROPRATE";
const char* DROPFACTOR_TOPIC =  "DEVICE1/DRIPFACTOR";
const char* VOLUMEINFUSED_TOPIC =  "DEVICE1/VOLUMEINFUSED";
const char* PULSE_TOPIC = "DEVICE1/PULSE";
const char* O2SAT_TOPIC = "DEVICE1/O2SAT";
///const char* IVTOPIC =  "DEVICE1/IV";
//const char* MQTTBROKER =  "192.168.3.152";
const char* MQTTBROKER =  "192.168.1.9";
const char* WILLMSG = "Going offline";
const char* WILLTOPIC = "DEVICE1/WILL";
const char* FLOWSTATUSTOPIC = "DEVICE1/FLOWSTATUS";

bool previousflowstate = false;

WiFiClient espClient;
PubSubClient client(MQTTBROKER,1883,espClient);

QueueHandle_t mqttqueue = xQueueCreate(10,sizeof(IV_Type)) ;
QueueHandle_t pulseoxiqueue = xQueueCreate(5,sizeof(PulseOxi_type));
//client.setServer(mqtt_server, 1883);

void keepwifialive(void * parameters){
    bool wififlag = false;            //set wifi flag as false
  for(;;){

    if(WiFi.status()==WL_CONNECTED){    //check if wifi connected
      if(!wififlag){                    
      Serial.println("connected");      //if wifi has connected now,print connected
      wififlag=true;                    //if wifi is connected,wififlag is turned true
      }
      vTaskDelay(10000/portTICK_PERIOD_MS);
      continue;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID,WIFI_PSD);
    unsigned long starttimer = millis();
    while(WiFi.status()!=WL_CONNECTED && millis()-starttimer<WIFI_TIMEOUT){}
    if(WiFi.status()!=WL_CONNECTED){
      Serial.println("failed to connect");
      vTaskDelay(20000/portTICK_PERIOD_MS);
      wififlag = false;
    }
    Serial.println("connected"); 

    

  }
}


void mqttTask(void * parameters){
  for(;;){
    IV_Type device1;
    PulseOxi_type oximeter;
    char buffer[10];
    if(client.connected()){
      client.loop();

      while(xQueueReceive(mqttqueue,(void*) &device1,0) == pdTRUE){
        Serial.println("sending data");
        client.publish(DROPFACTOR_TOPIC,itoa(device1.dropfactor,buffer,10));
        client.publish(DRIPRATE_TOPIC,itoa(device1.driprate,buffer,10));
        client.publish(VOLUMEINFUSED_TOPIC,itoa(device1.volumeinfused,buffer,10));
        //not sure if this will work as intended or cause some sort of race condition
        if(previousflowstate!=flowStatus){
          client.publish(FLOWSTATUSTOPIC,flowStatus?"TRUE":"FALSE");
          previousflowstate = flowStatus;
        }
      }
      while (xQueueReceive(pulseoxiqueue,(void*)&oximeter,0)==pdTRUE){
        client.publish(PULSE_TOPIC,itoa(oximeter.heartRateAvg,buffer,10));
        client.publish(O2SAT_TOPIC,itoa(oximeter.O2satAvg,buffer,10));
      }
      
    }else{
      //Serial.println("client not connected");
    }

    vTaskDelay(1000/portTICK_PERIOD_MS);
  }


}

void callback( char* topic, byte* payload, unsigned int length){

  Serial.print("message arrived on : ");
  Serial.println(topic);
  if((char)payload[0] == 'T'){
    //flowStatus = true;
    motorclose(2000);
  }
  if((char)payload[0] == 'F'){
   // flowStatus = false;
  motoropen(2000);
}  }




void keepMQTTConnected(void * parameters){
  for (;;){
    if(!client.connected()){
      if(client.connect(ID)){
        Serial.println("success");
        client.subscribe("DEVICE1/MSG");
        client.setCallback(callback);
      }
      else
      Serial.println("fail");
    }
    vTaskDelay(5000/portTICK_PERIOD_MS);

  }
}