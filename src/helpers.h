// bool bateriaOK() {
// 	// La Wemos D1 Mini ya tiene un divisor incorporado con resistencias de 100k y 220k
// 	// adicionar solo resistencia 100k a la entrada A0 p/ incrementar el rango a 4.2V.
// 	// P/ otras placas adicionar directamente divisor completo, ej 22k + 47k
// 	// y calibrar valor de KBAT según muestrario de lecturas.
// 	int lectura = 0;
// 	int muestras = 50;

// 	for (int i = 0; i < muestras; i++) { lectura += analogRead(BATERIA); delay(5); }
// 	lectura /= muestras;
// 	float voltaje = lectura * KBAT;
	
// 	if (DEBUG) Serial.printf("Voltaje: %.1f", voltaje);

// 	return (voltaje <= BATMIN) ? false : true;
// }

void ajustarServo(uint8_t servo, int pulso) {
	switch (servo) {
		case 1: {
			pulso > 0 ? servoEstab.writeMicroseconds(MED_PWM + parametros.offsetEstab + pulso) : servoEstab.writeMicroseconds(MED_PWM - parametros.offsetEstab + pulso);
			break;
		}

		default: {
			break;
		}
	}
}

void cbCtrlLed();
void cbCtrlSocket();
void cbCtrlTrepada();
void cbCtrlTransicion();
void cbCtrlVuelo();
void cbCtrlNotificacion();
void cbCtrlAutoDT();
void cbCtrlRDT();

Task ctrlLed(100, TASK_FOREVER, &cbCtrlLed);
Task ctrlSocket(3000, TASK_FOREVER, &cbCtrlSocket);
Task ctrlTrepada(1000, TASK_FOREVER, &cbCtrlTrepada);
Task ctrlTransicion(1000, TASK_FOREVER, &cbCtrlTransicion);
Task ctrlVuelo(1000, TASK_FOREVER, &cbCtrlVuelo);
Task ctrlNotificacion(1000, TASK_FOREVER, &cbCtrlNotificacion);
Task ctrlAutoDT(30000L, TASK_FOREVER, &cbCtrlAutoDT);
Task ctrlRDT(250, TASK_FOREVER, &cbCtrlRDT);

void cbCtrlLed() {
	digitalWrite(LED, !digitalRead(LED));
}

void cbCtrlSocket() {
	String cadenaJson = "{ \"topico\": \"ping\" }";
	webSocket.broadcastTXT(cadenaJson);
}

void cbCtrlTrepada() {
	if (!ctrlTrepada.isFirstIteration()) {
		modo = TRANSICION;
		procesarUnaVez = true;
		ctrlTrepada.disable();
	}
}

void cbCtrlTransicion() {
	if (!ctrlTransicion.isFirstIteration()) {
		modo = VUELO;
		procesarUnaVez = true;
		ctrlTransicion.disable();
	}
}

void cbCtrlVuelo() {
	if (!ctrlVuelo.isFirstIteration()) {
		timerDetencion = millis();
		modo = DESTERMALIZADO;
		procesarUnaVez = true;
		ctrlVuelo.disable();
	}
}

void cbCtrlAutoDT() {
	if (!ctrlAutoDT.isFirstIteration()) {
		modo = DESTERMALIZADO;
		procesarUnaVez = true;
		ctrlAutoDT.disable();
	}
}

void cbCtrlNotificacion() {
	if (!ctrlNotificacion.isFirstIteration()) {
		frec_parpadeo_activo = 3000;
		ctrlLed.setInterval(frec_parpadeo_activo);
		ctrlNotificacion.disable();
	}
}

void cbCtrlRDT() {
	int lectura = analogRead(RDT);
	// int lectura = digitalRead(RDT);
	Serial.println(lectura);
	
	// if (lectura <= 800) {
	// 	procesarUnaVez = true;
	// 	modo = DESTERMALIZADO;
	// 	if (DEBUG) Serial.println(lectura);
	// }
}

void inicializarTareas() {
	tareas.init();
	tareas.addTask(ctrlLed);
	tareas.addTask(ctrlSocket);
	tareas.addTask(ctrlTrepada);
	tareas.addTask(ctrlTransicion);
	tareas.addTask(ctrlVuelo);
	tareas.addTask(ctrlNotificacion);
	tareas.addTask(ctrlAutoDT);
	tareas.addTask(ctrlRDT);
}

void inicializarPines() {
	pinMode(LED, OUTPUT);
	digitalWrite(LED, HIGH); // recordar led onboard negado ESP8266 12F

	// Pulsador de inicio
	pinMode(PULSADOR, INPUT_PULLUP);
	debouncerPulsador.attach(PULSADOR);
	debouncerPulsador.interval(FREC_PULSADOR);

	// Sensor delantero gancho (trepada / circular)
	pinMode(GANCHO_FCD, INPUT_PULLUP);
	debouncerFCD.attach(GANCHO_FCD);
	debouncerFCD.interval(FREC_FCS);

	// Sensor superior gancho (abierto / cerrado)
	pinMode(GANCHO_FCS, INPUT_PULLUP);
	debouncerFCS.attach(GANCHO_FCS);
	debouncerFCS.interval(FREC_FCS);
	ganchoAbierto = digitalRead(GANCHO_FCS);

	// RDT
	pinMode(RDT, INPUT_PULLUP);

	// Servo control estabilizador
	servoEstab.attach(SERVO_ESTAB, MIN_PWM, MAX_PWM);
}

void inicializarFS() {
	while (!LittleFS.begin()) delay(500);
}

bool recuperarConfig() {
	File archivoConfig = LittleFS.open("/config.txt", "r");
	if (!archivoConfig) return false;

	String linea = "";
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.offsetEstab = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.retardoInicio = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.tiempoTrepada = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.tiempoTransicion = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.tiempoVuelo = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.angRemolque = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.angCircular = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.angDespegue = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.angTransicion = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.angVuelo = linea.toInt();
	
	linea = archivoConfig.readStringUntil('\n');
	parametros.angDT = linea.toInt();

	linea = archivoConfig.readStringUntil('\n');
	parametros.servoEstabInvertido = linea.toInt();

	archivoConfig.close();

	return true;
}

bool actualizarConfig() {
	static bool retorno = false;

	File archivoConfig = LittleFS.open("/config.txt", "w");
	if (!archivoConfig) return false;

	String cadena = String(parametros.offsetEstab) + "\n" + String(parametros.retardoInicio) + "\n" + String(parametros.tiempoTrepada) + "\n" + String(parametros.tiempoTransicion) + "\n" + String(parametros.tiempoVuelo) + "\n" + String(parametros.angRemolque) + "\n" + String(parametros.angCircular) + "\n" + String(parametros.angDespegue) + "\n" + String(parametros.angTransicion) + "\n" + String(parametros.angVuelo) + "\n" + String(parametros.angDT) + "\n" + String(parametros.servoEstabInvertido) + "\n";
	int escrito = archivoConfig.print(cadena);
	escrito > 0 ? retorno = true : retorno = false;
	archivoConfig.close();

	if (retorno) {
		frec_parpadeo_activo = 100;
		ctrlLed.setInterval(frec_parpadeo_activo);
		ctrlNotificacion.enableIfNot();
	}

	return retorno;
}

void activarSecuencia() {
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
	
	frec_parpadeo_activo = 250;
	ctrlLed.setInterval(frec_parpadeo_activo);
	procesarUnaVez = true;
	secuenciaIniciada = true;
	digitalRead(GANCHO_FCD) ? ajustarServo(ID_SERVO_ESTAB, parametros.angCircular) : ajustarServo(ID_SERVO_ESTAB, parametros.angRemolque);
	
	// webSocket.sendTXT(idClienteSocket, "{ \"topico\": \"estado\", \"modo\": \"LISTO\" }");
}

void eventosWS(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
	switch (type) {
		case WStype_DISCONNECTED: {
			clienteAPConectado = false;
			break;
		}

		case WStype_CONNECTED: {
			clienteAPConectado = true;
			idClienteSocket = num;
			// IPAddress ipCliente = webSocket.remoteIP(num);

			String buffer = "{ \"topico\": \"config\", \"modo\": " + String(modo) + ", \"datos\": [ " + parametros.offsetEstab + ", " + parametros.retardoInicio + ", " + parametros.tiempoTrepada + ", " + parametros.tiempoTransicion + ", " + parametros.tiempoVuelo + ", " + parametros.angRemolque + ", " + parametros.angCircular + ", " + parametros.angDespegue + ", " + parametros.angTransicion + ", " + parametros.angVuelo + ", " + parametros.angDT + ", " + parametros.servoEstabInvertido + " ] }";
			webSocket.sendTXT(num, buffer);
			ctrlSocket.enableIfNot();
			break;
		}

		case WStype_TEXT: {
			String cadenaRecibida = "";
			for (int i = 0; i < (int)length; i++) cadenaRecibida += (char)payload[i];
			
			StringSplitter *cadenaProcesada = new StringSplitter(cadenaRecibida, '|', 2);
			const String topico = cadenaProcesada->getItemAtIndex(0);
			const String datos = cadenaProcesada->getItemAtIndex(1);

			if (topico == "config") {
				bool reposicionarEstab = false;
				// Recordar actualizar en StringSplitter.h el límite MAX!!!
				StringSplitter *cadenaDatos = new StringSplitter(datos, ',', 12);

				const int offsetEstabilizador = cadenaDatos->getItemAtIndex(0).toInt();
				const int retardoInicio = cadenaDatos->getItemAtIndex(1).toInt();
				const int tiempoTrepada = cadenaDatos->getItemAtIndex(2).toInt();
				const int tiempoTransicion = cadenaDatos->getItemAtIndex(3).toInt();
				const int tiempoVuelo = cadenaDatos->getItemAtIndex(4).toInt();
				const int anguloRemolque = cadenaDatos->getItemAtIndex(5).toInt();
				const int anguloCircular = cadenaDatos->getItemAtIndex(6).toInt();
				const int anguloDespegue = cadenaDatos->getItemAtIndex(7).toInt();
				const int anguloTransicion = cadenaDatos->getItemAtIndex(8).toInt();
				const int anguloVuelo = cadenaDatos->getItemAtIndex(9).toInt();
				const int anguloDT = cadenaDatos->getItemAtIndex(10).toInt();
				const int servoEstabInvertido = cadenaDatos->getItemAtIndex(11).toInt();

				if (parametros.offsetEstab != offsetEstabilizador) { parametros.offsetEstab = offsetEstabilizador; reposicionarEstab = true; }
				if (parametros.retardoInicio != retardoInicio) parametros.retardoInicio = retardoInicio;
				if (parametros.tiempoTrepada != tiempoTrepada) parametros.tiempoTrepada = tiempoTrepada;
				if (parametros.tiempoTransicion != tiempoTransicion) parametros.tiempoTransicion = tiempoTransicion;
				if (parametros.tiempoVuelo != tiempoVuelo) parametros.tiempoVuelo = tiempoVuelo;
				if (parametros.angRemolque != anguloRemolque) parametros.angRemolque = anguloRemolque;
				if (parametros.angCircular != anguloCircular) parametros.angCircular = anguloCircular;
				if (parametros.angDespegue != anguloDespegue) parametros.angDespegue = anguloDespegue;
				if (parametros.angTransicion != anguloTransicion) parametros.angTransicion = anguloTransicion;
				if (parametros.angVuelo != anguloVuelo) parametros.angVuelo = anguloVuelo;
				if (parametros.angDT != anguloDT) parametros.angDT = anguloDT;
				if (parametros.servoEstabInvertido != servoEstabInvertido) parametros.servoEstabInvertido = servoEstabInvertido;

				if (actualizarConfig()) {
					if (reposicionarEstab) {
						reposicionarEstab = false;
						ajustarServo(ID_SERVO_ESTAB, 0);
					}
					webSocket.sendTXT(0, "{ \"topico\": \"confirmacion\", \"resultado\": \"OK\", \"mensaje\": \"Configuración actualizada\" }");
				} else {
					webSocket.sendTXT(0, "{ \"topico\": \"confirmacion\", \"resultado\": \"ERROR\", \"mensaje\": \"ERROR al actualizar\" }");
				}
			}
			break;
		}

		case WStype_BIN: {
			// USE_SERIAL.printf("[%u] binario long: %u\n", num, length);
			// hexdump(payload, length);
			// webSocket.sendBIN(num, payload, length);
			break;
		}
		
		case WStype_PING: {
			break;
		}

		case WStype_PONG: {
			break;
		}

		case WStype_ERROR:
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN: {
			break;
		}
	}
}

void activarSocket() {
	webSocket.begin();
	webSocket.onEvent(eventosWS); // Gestor de eventos socket
	// webSocket.setAuthorization("pepe", "pompin");
	// webSocket.enableHeartbeat(5000, 5000, 0); // Heartbeat
}

void controlarPulsador() {	
	if (debouncerPulsador.fell()) {
		if (!btnPresionado) {
			timerInicio = millis();
			btnPresionado = true;
		}
	}

	if (debouncerPulsador.rose()) {
		if (btnPresionado) btnPresionado = false;
	}

	if ((millis() - timerInicio >= ((unsigned long)parametros.retardoInicio) * 1000) && btnPresionado && !secuenciaIniciada) { // pq el parámetro se guarda en segs
		if (modo == LISTO && !ganchoAbierto) {
			activarSecuencia();
		} else {
			btnPresionado = false;
			if (modo != DETENIDO) modo = LISTO;
		}
		timerInicio = millis();
	}
}

void controlarFinalesCarrera() {
	if (debouncerFCD.fell()) {
		if (secuenciaIniciada) {
			modo = REMOLCANDO;
			ctrlAutoDT.disable();
		}
		procesarUnaVez = true;
	}

	if (debouncerFCD.rose()) {
		if (ganchoAbierto) {
			if (secuenciaIniciada) modo = TREPADA;
		} else {
			if (secuenciaIniciada) {
				modo = CIRCULANDO;
				ctrlAutoDT.enableIfNot();
			}
		}
		procesarUnaVez = true;
	}
	
	if (debouncerFCS.fell()) {
		ganchoAbierto = false;
		procesarUnaVez = true;
	}

	if (debouncerFCS.rose()) {
		ganchoAbierto = true;
		procesarUnaVez = true;
		if (secuenciaIniciada) modo = DESPEGUE;
	}
}
