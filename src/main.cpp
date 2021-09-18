#include <Arduino.h>
#include <Bounce2.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <TaskScheduler.h> // https://github.com/arkhipenko/TaskScheduler
#include <WebSocketsServer.h>

#include "Servo.h"
#include "StringSplitter.h"
#include "config.h"
#include "wf.h"
#include "helpers.h"

void setup() {
	if (DEBUG) Serial.begin(115200);
	if (DEBUG) Serial.setDebugOutput(true);
	if (DEBUG) Serial.println();

	delay(500);
	ESP.wdtDisable(); // Deshabilitación watchdog por software

	delay(500);
	// if (bateriaOK()) {
		inicializarPines(); // Seteo de pines
		inicializarTareas(); // Activación de tareas
		inicializarFS(); // Inicialización sistema de archivos (p/lectura datos config)
		if (recuperarConfig()) { // Si se recupera el archivo de config
			// Este parche está puesto acá porque el RDT reinicia el micro, hay algún bardo en la lectura con el A0.
			// Hasta resolver ese punto, al reiniciar se manda a modo DT para que actúe.
			ajustarServo(ID_SERVO_ESTAB, parametros.servoEstabInvertido * parametros.angDT);
			delay(1000);

			activarAP(); // Activación AP wifi p/configuración
			// activarST(); // Activación como cliente wifi p/depuración
			activarSocket(); // Activación socket acceso remoto
			frec_parpadeo_activo = 3000;
			modo = LISTO;
		} else { // sino el sistema queda detenido
			frec_parpadeo_activo = 100;
			modo = DETENIDO;
		}
	// } else {
	// 	frec_parpadeo_activo = 500;
	// 	modo = DETENIDO;
	// }

	ctrlLed.setInterval(frec_parpadeo_activo); // se ajusta la frecuencia de parpadeo del led testigo
	ctrlLed.enableIfNot();
		
	timerInicio = millis();
}

void loop() {
	switch (modo) {
		case LISTO: {
			// Centra el servo si no lo está, y actualiza la frec de parpadeo
			if (paServoEstab != MED_PWM) {
				paServoEstab = MED_PWM;
				frec_parpadeo_activo = 3000;
				ajustarServo(ID_SERVO_ESTAB, 0);
				if (DEBUG) Serial.println("Listo");
			}
			break;
		}

		case REMOLCANDO: {
			if (procesarUnaVez) {
				procesarUnaVez = false;
				ajustarServo(ID_SERVO_ESTAB, parametros.servoEstabInvertido * parametros.angRemolque);
				if (DEBUG) Serial.println("Remolcando");
			}
			break;
		}

		case CIRCULANDO: {
			if (procesarUnaVez) {
				procesarUnaVez = false;
				ajustarServo(ID_SERVO_ESTAB, parametros.servoEstabInvertido * parametros.angCircular);
				if (DEBUG) Serial.println("Circulando");
			}
			break;
		}

		case DESPEGUE: {
			if (procesarUnaVez) {
				procesarUnaVez = false;
				ajustarServo(ID_SERVO_ESTAB, parametros.servoEstabInvertido * parametros.angDespegue);
			}
			break;
		}

		case TREPADA: {
			if (procesarUnaVez) {
				procesarUnaVez = false;
				ajustarServo(ID_SERVO_ESTAB, parametros.servoEstabInvertido * parametros.angVuelo);
				ctrlTrepada.setInterval(parametros.tiempoTrepada);
				ctrlTrepada.enableIfNot();
			}
			break;
		}

		case TRANSICION: {
			if (procesarUnaVez) {
				procesarUnaVez = false;
				ajustarServo(ID_SERVO_ESTAB, -(parametros.servoEstabInvertido * parametros.angTransicion));
				ctrlTransicion.setInterval(parametros.tiempoTransicion);
				ctrlTransicion.enable();
			}
			break;
		}

		case VUELO: {
			if (procesarUnaVez) {
				procesarUnaVez = false;
				ajustarServo(ID_SERVO_ESTAB, parametros.servoEstabInvertido * parametros.angVuelo);
				ctrlVuelo.setInterval(parametros.tiempoVuelo * 1000); // pq el parámetro se guarda en segs
				ctrlVuelo.enable();
				ctrlRDT.enable();
				if (DEBUG) Serial.println("En vuelo");
			}
			break;
		}

		case DESTERMALIZADO: {
			if (procesarUnaVez) {
				procesarUnaVez = false;
				timerDetencion = millis();
				ajustarServo(ID_SERVO_ESTAB, parametros.servoEstabInvertido * parametros.angDT);
				if (DEBUG) Serial.println("Destermalizado");
				modo = DETENIDO;
			}
			break;
		}

		case DETENIDO: {
			// Simplemente espera 3 segs y suspende el micro
			if (millis() - timerDetencion >= 3000) {
				if (DEBUG) Serial.println("Detenido");
				ESP.deepSleep(0);
			}
			break;
		}

		default: {
		}
	}

	tareas.execute(); // Control general de tareas
	
	if (modo != VUELO) {
		if (WiFi.getMode() != WIFI_OFF) webSocket.loop(); // Control del socket
	
		debouncerPulsador.update();
		debouncerFCD.update();
		debouncerFCS.update();

		controlarPulsador();
		controlarFinalesCarrera();
	}
	
	ESP.wdtFeed(); // Alimentación watchdog por hardware
}
