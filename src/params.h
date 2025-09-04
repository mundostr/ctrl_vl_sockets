#pragma once

#include <EEPROM.h>
#include "config.h"

struct params_format {
    int startDelay;
    int stabServoInverted;
    int stabOffset;
    int takeoffTime;
    int climbTime;
    int transitionTime;
    int flightTime;
    int towAngle;
    int circularAngle;
    int takeoffAngle;
    int climbAngle;
    int transitionAngle;
    int flightAngle;
    int dtAngle;
};

params_format flight_params;

void saveParams(params_format params) {
    EEPROM.begin(sizeof(params_format));
    for (unsigned int i = 0; i < sizeof(params_format); i++) EEPROM.write(i, *((byte*)&params + i));
    EEPROM.commit();
}

params_format loadParams() {
    params_format params;
    EEPROM.begin(sizeof(params_format));
    for (unsigned int i = 0; i < sizeof(params_format); i++) *((byte*)&params + i) = EEPROM.read(i);
    
    return params;
}
