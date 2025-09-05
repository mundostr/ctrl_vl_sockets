#pragma once

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
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
WebSocketsServer webSocket = WebSocketsServer(81);

void IRAM_ATTR servos_isr();

void print_params() {
    Serial.printf("%-20s: %i\n", "Start Delay (ms)", flight_params.startDelay);
    Serial.printf("%-20s: %i\n", "Stab Servo Inverted", flight_params.stabServoInverted);
    Serial.printf("%-20s: %i\n", "Stab Offset", flight_params.stabOffset);
    Serial.printf("%-20s: %i\n", "Takeoff Time (ms)", flight_params.takeoffTime);
    Serial.printf("%-20s: %i\n", "Climb Time (ms)", flight_params.climbTime);
    Serial.printf("%-20s: %i\n", "Transition Time (ms)", flight_params.transitionTime);
    Serial.printf("%-20s: %i\n", "Flight Time (ms)", flight_params.flightTime);
    Serial.printf("%-20s: %i\n", "Tow Angle", flight_params.towAngle);
    Serial.printf("%-20s: %i\n", "Circular Angle", flight_params.circularAngle);
    Serial.printf("%-20s: %i\n", "Takeoff Angle", flight_params.takeoffAngle);
    Serial.printf("%-20s: %i\n", "Climb Angle", flight_params.climbAngle);
    Serial.printf("%-20s: %i\n", "Transition Angle", flight_params.transitionAngle);
    Serial.printf("%-20s: %i\n", "Flight Angle", flight_params.flightAngle);
    Serial.printf("%-20s: %i\n", "DT Angle", flight_params.dtAngle);
}

void save_params() {
    static bool eeprom_busy = false;
    if (eeprom_busy) return;
    eeprom_busy = true;
    
    EEPROM.begin(sizeof(params_format));
    const byte* bytePtr = (const byte*)&flight_params;
    for (unsigned int i = 0; i < sizeof(params_format); i++) EEPROM.write(i, bytePtr[i]);
    
    timer1_detachInterrupt();
    EEPROM.commit();
    timer1_attachInterrupt(servos_isr);
    
    #ifdef DEBUG
    Serial.println("Parametros almacenados");
    #endif
    
    eeprom_busy = false;
}

params_format load_params() {
    params_format params;

    EEPROM.begin(sizeof(params_format));
    for (unsigned int i = 0; i < sizeof(params_format); i++) *((byte*)&params + i) = EEPROM.read(i);

    #ifdef DEBUG
    Serial.println("Parametros recuperados");
    #endif
    
    return params;
}

void send_flight_params(int id) {
    String json = "{";
        json += "\"startDelay\":" + String(flight_params.startDelay) + ",";
        json += "\"stabServoInverted\":" + String(flight_params.stabServoInverted) + ",";
        json += "\"stabOffset\":" + String(flight_params.stabOffset) + ",";
        json += "\"takeoffTime\":" + String(flight_params.takeoffTime) + ",";
        json += "\"climbTime\":" + String(flight_params.climbTime) + ",";
        json += "\"transitionTime\":" + String(flight_params.transitionTime) + ",";
        json += "\"flightTime\":" + String(flight_params.flightTime) + ",";
        json += "\"towAngle\":" + String(flight_params.towAngle) + ",";
        json += "\"circularAngle\":" + String(flight_params.circularAngle) + ",";
        json += "\"takeoffAngle\":" + String(flight_params.takeoffAngle) + ",";
        json += "\"climbAngle\":" + String(flight_params.climbAngle) + ",";
        json += "\"transitionAngle\":" + String(flight_params.transitionAngle) + ",";
        json += "\"flightAngle\":" + String(flight_params.flightAngle) + ",";
        json += "\"dtAngle\":" + String(flight_params.dtAngle);
        json += "}";

    id == -1 ? webSocket.broadcastTXT(json) : webSocket.sendTXT(id, json);
}

void parse_flight_params(char* payload) {
    String data = String(payload);
    int index = data.indexOf('=');
    
    if (index != -1) {
        String key = data.substring(0, index);
        String value = data.substring(index + 1);
        
        if (key == "startDelay") flight_params.startDelay = value.toInt();
        if (key == "stabServoInverted") flight_params.stabServoInverted = value.toInt();
        if (key == "stabOffset") flight_params.stabOffset = value.toInt();
        if (key == "takeoffTime") flight_params.takeoffTime = value.toInt();
        if (key == "climbTime") flight_params.climbTime = value.toInt();
        if (key == "transitionTime") flight_params.transitionTime = value.toInt();
        if (key == "flightTime") flight_params.flightTime = value.toInt();
        if (key == "towAngle") flight_params.towAngle = value.toInt();
        if (key == "circularAngle") flight_params.circularAngle = value.toInt();
        if (key == "takeoffAngle") flight_params.takeoffAngle = value.toInt();
        if (key == "climbAngle") flight_params.climbAngle = value.toInt();
        if (key == "transitionAngle") flight_params.transitionAngle = value.toInt();
        if (key == "flightAngle") flight_params.flightAngle = value.toInt();
        if (key == "dtAngle") flight_params.dtAngle = value.toInt();
    }

    print_params();
}

void handle_websockets_event(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_BIN:
        case WStype_PING:
        case WStype_PONG:
        case WStype_ERROR:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT_TEXT_START:
            break;
            
        case WStype_DISCONNECTED:
            #ifdef DEBUG
            Serial.printf("[%u] desconectado\n", num);
            #endif
        
        break;
        
        case WStype_CONNECTED: {
            send_flight_params(num);
            
            #ifdef DEBUG
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Conectado desde %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
            #endif
        }
        
        break;
        
        case WStype_TEXT: {
            parse_flight_params((char*)payload);
            send_flight_params(-1);
            need_to_save_params = true;

            #ifdef DEBUG
            Serial.printf("[%u] Msj recibido: %s\n", num, payload);
            #endif
        }
        
        break;
    }
}

void init_wifi() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_AP_NAME, WIFI_AP_PASS);

    webSocket.begin();
    webSocket.onEvent(handle_websockets_event);

    #ifdef DEBUG
        Serial.print("AP activo en ");
        Serial.println(WiFi.softAPIP());
    #endif
}

void handle_config() {
    static uint32_t led_timer = 0;

    if (hook_front_sensor.isPressed()) {
        init_wifi();
        delay(250);
        
        while(1) {
            if (millis() - led_timer >= 500) {
                led_timer = millis();
                digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            }
        
            if (need_to_save_params) {
                need_to_save_params = false;
                save_params();
            }
        
            webSocket.loop();
            ESP.wdtFeed();
        }

        #ifdef DEBUG
        Serial.println("Modo CONFIG activo");
        #endif
    }
}
