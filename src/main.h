#pragma once

#include "config.h"

void handle_delayed_movement() {
    if (movement_active) {
        if (millis() - main_timer >= current_movement_delay) {
            movement_active = false;
            flight_mode = next_mode;
        }
    }
}

uint32_t ms_to_ticks(uint32_t ms) {
    return (ms * 1000) / (uint32_t)US_TO_TICKS;
}

void change_mode_delayed(flight_modes mode, uint32_t ms_delay) {
    next_mode = mode;
    flight_mode = IDLE;
    main_timer = millis();
    movement_active = true;
    current_movement_delay = ms_delay;
}

void hook_top_handler(Button2 &btn) {
    hook_closed = hook_top_sensor.isPressed();
    if (hook_closed) hook_armed = true;

    #ifdef DEBUG
    Serial.printf("Hook closed: %i\n", hook_closed);
    #endif
}

void hook_front_handler(Button2 &btn) {
    bool hook_towing = hook_front_sensor.isPressed();

    flight_mode = hook_towing ? TOW : CIRCULAR;
    execute_once = true;

    if (hook_armed && !hook_towing && !hook_closed) { // Model launched
        flight_mode = TAKEOFF;
    }
}

void init_system(int save_default_params = false) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, 0);

    EEPROM.begin(sizeof(params_format));
    if (save_default_params) save_params();
    flight_params = load_params();
    
    #ifdef DEBUG
    print_params();
    #endif
}

void init_sensors_and_buttons() {
    hook_top_sensor.begin(HOOK_TOP_SENSOR_PIN, INPUT_PULLUP, true);
    hook_front_sensor.begin(HOOK_FRONT_SENSOR_PIN, INPUT_PULLUP, true);

    hook_top_sensor.setChangedHandler(hook_top_handler);
    hook_front_sensor.setChangedHandler(hook_front_handler);

    hook_closed = hook_top_sensor.isPressed();

    #ifdef DEBUG
    Serial.printf("Hook closed: %i\n", hook_closed);
    Serial.printf("Hook towing: %i\n", hook_front_sensor.isPressed());
    #endif
}

void position_stab_servo(int position) {
    stab_servo_pulse = flight_params.stabServoInverted * position;
}

void IRAM_ATTR servos_isr() {
    static bool state = false;
    
    if (state) {
        digitalWrite(STAB_SERVO_PIN, LOW);
        timer1_write((uint32_t)(US_TO_TICKS * 1000)); // counts down during 10ms with current prescaler
    } else {
        digitalWrite(STAB_SERVO_PIN, HIGH);
        timer1_write((uint32_t)(stab_servo_pulse / US_TO_TICKS));
    }
    
    state = !state;
}

void init_servos() {
    pinMode(STAB_SERVO_PIN, OUTPUT);
    
    timer1_attachInterrupt(servos_isr);
    timer1_enable(TIMER1_PRESCALER, TIM_EDGE, TIM_LOOP);
    timer1_write((uint32_t)(US_TO_TICKS * 1000)); // counts down during 10ms with current prescaler

    stab_servo_pulse = STAB_SERVO_MID_PULSE;
}
