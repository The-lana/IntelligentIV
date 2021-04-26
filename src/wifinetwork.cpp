#include "wifinetwork.h"


const char* ID = "DEVICE1";
const char* IVTOPIC =  "DEVICE1/IV";
const char* MQTTBROKER =  "192.168.3.64";
const char* WILLMSG = "Going offline";
const char* WILLTOPIC = "DEVICE1/WILL";


WiFiClient espClient;
PubSubClient client(MQTTBROKER,1883,espClient);

QueueHandle_t mqttqueue = xQueueCreate(10,sizeof(char)*50) ;
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
    char buffer[50];
    if(client.connected()){
      while(xQueueReceive(mqttqueue,(void*) &buffer,0) == pdTRUE){
        client.publish(IVTOPIC,buffer);
      }
    }else{
      Serial.println("client not connected");
    }

    vTaskDelay(1000/portTICK_PERIOD_MS);
  }


}

void callback( char* topic, byte* payload, unsigned int length){

  Serial.print("message arrived on : ");
  Serial.println(topic);

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