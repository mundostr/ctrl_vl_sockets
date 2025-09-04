#include "config.h"
#include "params.h"
#include "main.h"

void setup() {
    #ifdef DEBUG
    Serial.begin(SERIAL_BAUDS);
    Serial.println("SYSTEM STARTED");
    #endif

    // Replace for the params recovery
    flight_params.stabServoInverted = 1;
    
    init_sensors_and_buttons();
    init_servos();
    delay(1000);
}

void loop() {
    switch (flight_mode) {
        case IDLE: {
            #ifdef DEBUG
            if (execute_once) {
                execute_once = false;
                Serial.println("Idle");
            }
            #endif

            delay(1);
            
            break;
        }

        case TOW: {
            position_stab_servo(STAB_SERVO_MID_PULSE + 200);
            flight_mode = IDLE;
                    
            #ifdef DEBUG
            Serial.println("Towing");
            #endif

			break;
		}

        case CIRCULAR: {
            position_stab_servo(STAB_SERVO_MID_PULSE + 400);
            flight_mode = IDLE;
                    
            #ifdef DEBUG
            Serial.println("Circulating");
            #endif

			break;
		}

        case TAKEOFF: {
            position_stab_servo(STAB_SERVO_MID_PULSE);
            change_mode_delayed(CLIMB, 500);
                    
            #ifdef DEBUG
            Serial.println("Taking off");
            #endif

			break;
		}

        case CLIMB: {
            position_stab_servo(STAB_SERVO_MID_PULSE - 200);
            change_mode_delayed(TRANSITION, 1000);
                    
            #ifdef DEBUG
            Serial.println("Climbing");
            #endif

			break;
        }

        case TRANSITION: {
            position_stab_servo(STAB_SERVO_MID_PULSE - 600);
            change_mode_delayed(FLIGHT, 750);
                    
            #ifdef DEBUG
            Serial.println("Transitioning");
            #endif

			break;
		}

        case FLIGHT: {
            position_stab_servo(STAB_SERVO_MID_PULSE + 100);
            change_mode_delayed(DT, 20 * 1000);
                    
            #ifdef DEBUG
            Serial.println("Flying");
            #endif

			break;
		}

        case DT: {
            position_stab_servo(STAB_SERVO_MID_PULSE + 1000);
            
            #ifdef DEBUG
            Serial.println("DT, flight finished");
            #endif
            
            ESP.deepSleep(0);

			break;
		}

        default: {
		}
    }

    if (movement_active) {
        if (millis() - main_timer >= current_movement_delay) {
            movement_active = false;
            flight_mode = next_mode;
        }
    }
    
    hook_top_sensor.loop();
    hook_front_sensor.loop();
}
