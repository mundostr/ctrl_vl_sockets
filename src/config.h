#pragma once

#include <Arduino.h>
#include <Button2.h>

#define DEBUG

#define LED_PIN 2
#define STAB_SERVO_PIN 5
#define HOOK_TOP_SENSOR_PIN 12
#define HOOK_FRONT_SENSOR_PIN 14

#define STAB_SERVO_MIN_PULSE 500
#define STAB_SERVO_MID_PULSE 1500
#define STAB_SERVO_MAX_PULSE 2500

#define SERIAL_BAUDS 115200
#define TIMER1_PRESCALER TIM_DIV256
#define US_TO_TICKS 3.2f // 3.2 for TIM_DIV256
const char* WIFI_AP_NAME = "ELVIOTIMER3";
const char* WIFI_AP_PASS = "12345678";

uint32_t main_timer = 0;
boolean hook_armed = false;
boolean hook_closed = false;
boolean execute_once = true;
boolean movement_active = false;
boolean need_to_save_params = false;
uint32_t current_movement_delay = 0;
volatile uint32_t stab_servo_pulse = STAB_SERVO_MIN_PULSE;

enum flight_modes { IDLE, TOW, CIRCULAR, TAKEOFF, CLIMB, TRANSITION, FLIGHT, DT };
flight_modes flight_mode = IDLE;
flight_modes next_mode = IDLE;

Button2 hook_top_sensor;
Button2 hook_front_sensor;
