#pragma once
#ifndef IV_H
#define IV_H

#include "Arduino.h"

typedef struct IV{
uint8_t dropfactor; //  drop factor of the iv set
int driprate; // rate of flow
int volumeinfused;//volume of IV infused
int volumetobeinfused;//
}IV_Type;


#endif