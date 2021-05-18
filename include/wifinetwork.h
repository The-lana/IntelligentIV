#pragma once
#ifndef WIFINETWORK_H
#define WIFINETWORK_H


#include "Arduino.h"
#include "WiFi.h"
#include "PubSubClient.h"
#include "IV.h"
#include "heartbeat.h"


#define WIFI_SSID "NANDANAM"
#define WIFI_PSD "nandanam1889"
#define WIFI_TIMEOUT 20000 



extern bool flowStatus;


void keepwifialive(void * parameters);
void mqttTask(void * parameters);
void callback( char* topic, byte* payload, unsigned int length);
void keepMQTTConnected(void * parameters);

#endif
