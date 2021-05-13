#include "wifinetwork.h"


const char* ID = "DEVICE1";
const char* DRIPRATE_TOPIC =  "DEVICE1/DROPRATE";
const char* DROPFACTOR_TOPIC =  "DEVICE1/DRIPFACTOR";
const char* VOLUMEINFUSED_TOPIC =  "DEVICE1/VOLUMEINFUSED";
const char* MQTTBROKER =  "192.168.1.8";
const char* WILLMSG = "Going offline";
const char* WILLTOPIC = "DEVICE1/WILL";


WiFiClient espClient;
PubSubClient client(MQTTBROKER,1883,espClient);

QueueHandle_t mqttqueue = xQueueCreate(10,sizeof(IV_Type)) ;
//client.setServer(mqtt_server, 1883);

void keepwifialive(void * parameters){
    bool wififlag = false;
  for(;;){

    if(WiFi.status()==WL_CONNECTED){
      if(!wififlag){
      Serial.println("connected");
      wififlag=true;
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
    char buffer[10];
    if(client.connected()){
      client.loop();
      Serial.println("sending data");
      while(xQueueReceive(mqttqueue,(void*) &device1,0) == pdTRUE){
        client.publish(DROPFACTOR_TOPIC,itoa(device1.dropfactor,buffer,10));
        client.publish(DRIPRATE_TOPIC,itoa(device1.driprate,buffer,10));
        client.publish(VOLUMEINFUSED_TOPIC,itoa(device1.volumeinfused,buffer,10));
        vTaskDelay(500/portTICK_PERIOD_MS);
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
  flowStatus = !flowStatus;


}


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