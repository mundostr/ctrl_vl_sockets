#pragma once

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include "config.h"

struct params_format {
    int startDelay = 0;
    int stabOffset = 0;
    int takeoffTime = 250;
    int climbTime = 750;
    int transitionTime = 500;
    int flightTime = 10000;
    int towAngle = -10;
    int circularAngle = -20;
    int takeoffAngle = -5;
    int climbAngle = 0;
    int transitionAngle = 30;
    int flightAngle = -5;
    int dtAngle = -45;
    int stabServoInverted = 1;
};
const int params_format_total = 14;

params_format flight_params;
WebSocketsServer webSocket = WebSocketsServer(81);

void IRAM_ATTR servos_isr();

void print_params() {
    Serial.printf("Demora partida (ms): %i\n", flight_params.startDelay);
    Serial.printf("Invertir servo estabilizador (1 o -1): %i\n", flight_params.stabServoInverted);
    Serial.printf("Desplazar servo estabilizador (grados): %i\n", flight_params.stabOffset);
    Serial.printf("Tiempo despegue (ms): %i\n", flight_params.takeoffTime);
    Serial.printf("Tiempo trepada (ms): %i\n", flight_params.climbTime);
    Serial.printf("Tiempo transicion (ms): %i\n", flight_params.transitionTime);
    Serial.printf("Tiempo vuelo (ms): %i\n", flight_params.flightTime);
    Serial.printf("Angulo remolque (grados): %i\n", flight_params.towAngle);
    Serial.printf("Angulo circular (grados): %i\n", flight_params.circularAngle);
    Serial.printf("Angulo despegue (grados): %i\n", flight_params.takeoffAngle);
    Serial.printf("Angulo trepada (grados): %i\n", flight_params.climbAngle);
    Serial.printf("Angulo transicion (grados): %i\n", flight_params.transitionAngle);
    Serial.printf("Angulo vuelo (grados): %i\n", flight_params.flightAngle);
    Serial.printf("Angulo destermalizado (grados): %i\n", flight_params.dtAngle);
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

    webSocket.sendTXT(id, json);
}

void parse_flight_params(uint8_t num, const String &message) {
    int values[params_format_total];
    int index = 0;
    int startIndex = 0;
    int endIndex = message.indexOf('|');
    
    while (endIndex != -1 && index < params_format_total) {
        values[index++] = message.substring(startIndex, endIndex).toInt();
        startIndex = endIndex + 1;
        endIndex = message.indexOf('|', startIndex);
    }
    
    if (index < params_format_total) values[index] = message.substring(startIndex).toInt();

    // Respetar orden según cadena enviada desde el cliente websockets
    if (index == params_format_total - 1) {
        flight_params.startDelay = values[0];
        flight_params.takeoffTime = values[1];
        flight_params.climbTime = values[2];
        flight_params.transitionTime = values[3];
        flight_params.flightTime = values[4] * 1000; // porque llega en segs
        flight_params.stabOffset = values[5];
        flight_params.towAngle = values[6];
        flight_params.circularAngle = values[7];
        flight_params.takeoffAngle = values[8];
        flight_params.climbAngle = values[9];
        flight_params.transitionAngle = values[10];
        flight_params.flightAngle = values[11];
        flight_params.dtAngle = values[12];
        flight_params.stabServoInverted = values[13];

        need_to_save_params = true;
        webSocket.sendTXT(num, "{ \"update\":\"ok\" }");
    }
}

void handle_websockets_event(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
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
            #ifdef DEBUG
            Serial.printf("[%u] Msj recibido: %s\n", num, payload);
            #endif

            parse_flight_params(num, String((char*)payload));

            #ifdef DEBUG
            print_params();
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
    int blink_count = 0;
    uint32_t led_timer = 0;
    uint32_t blink_freq = 500;

    if (hook_front_sensor.isPressed()) {
        init_wifi();
        delay(250);
        
        while(1) {
            if (millis() - led_timer >= blink_freq) {
                led_timer = millis();
                digitalWrite(LED_PIN, !digitalRead(LED_PIN));

                if (blink_freq == 125) {
                    blink_count++;
                    if (blink_count == 20) {
                        blink_count = 0;
                        blink_freq = 500;
                    }
                }
            }
        
            if (need_to_save_params) {
                need_to_save_params = false;
                blink_freq = 125;
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
