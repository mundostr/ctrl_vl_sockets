#pragma once

#include <EEPROM.h>
#include "config.h"

struct params_format {
    int startDelay = 0;
    int stabServoInverted = 1;
    int stabOffset = 0;
    int takeoffTime = 250;
    int climbTime = 750;
    int transitionTime = 500;
    int flightTime = 180000;
    int towAngle = 100;
    int circularAngle = 200;
    int takeoffAngle = 50;
    int climbAngle = 0;
    int transitionAngle = 600;
    int flightAngle = 50;
    int dtAngle = 1000;
};

params_format flight_params;

void save_params(params_format params) {
    EEPROM.begin(sizeof(params_format));
    for (unsigned int i = 0; i < sizeof(params_format); i++) EEPROM.write(i, *((byte*)&params + i));
    EEPROM.commit();

    #ifdef DEBUG
    Serial.println("Params saved");
    #endif
}

params_format load_params() {
    params_format params;

    EEPROM.begin(sizeof(params_format));
    for (unsigned int i = 0; i < sizeof(params_format); i++) *((byte*)&params + i) = EEPROM.read(i);

    #ifdef DEBUG
    Serial.println("Params recovered");
    #endif
    
    return params;
}
