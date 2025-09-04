#pragma once

#include <Arduino.h>
#include <Button2.h>

#define DEBUG

#define LED_PIN 2
#define BATTERY_PIN A0
#define STAB_SERVO_PIN 5
#define PUSH_BUTTON_PIN 13
#define HOOK_TOP_SENSOR_PIN 12
#define HOOK_FRONT_SENSOR_PIN 14

#define STAB_SERVO_MIN_PULSE 500
#define STAB_SERVO_MID_PULSE 1500
#define STAB_SERVO_MAX_PULSE 2500

#define SERIAL_BAUDS 115200
#define TIMER1_PRESCALER TIM_DIV256
#define US_TO_TICKS 3.2f // 3.2 for TIM_DIV256

boolean hook_armed = false;
boolean hook_closed = false;
boolean execute_once = true;
boolean movement_active = false;
volatile uint32_t stab_servo_pulse = STAB_SERVO_MIN_PULSE;
uint32_t main_timer = 0;
uint32_t current_movement_delay = 0;

enum flight_modes { IDLE, TOW, CIRCULAR, TAKEOFF, CLIMB, TRANSITION, FLIGHT, DT };
flight_modes flight_mode = IDLE;
flight_modes next_mode = IDLE;

Button2 hook_top_sensor;
Button2 hook_front_sensor;
