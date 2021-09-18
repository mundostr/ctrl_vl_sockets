#define LED 2 // Led testigo
#define SERVO_ESTAB 5 // antes 14, Servo estabilizador
#define PULSADOR 13 // antes 12, Botón de inicio
#define GANCHO_FCD 14 // antes 3, Final carrera delantero gancho
#define GANCHO_FCS 12 // antes 13, Final carrera superior gancho
// por ahora ponemos el RDT en el analógico, hay que ajustarlo cuando se pueda
#define RDT A0 // D2, señal de entrada del RDT
#define ID_SERVO_ESTAB 1 // Identificador del servo estabilizador
// #define BATERIA A0 // Control nivel batería

#define FREC_PULSADOR 25 // frecuencia refresco lectura pulsador
#define FREC_FCS 25 // frecuencia refresco lectura finales carrera
#define MIN_PWM 500 // Pulso mínimo servos (ms)
#define MED_PWM 1500 // Pulso medio servos (ms)
#define MAX_PWM 2500 // Pulso máximo servos (ms)

const float KBAT = 0.0041; // Constante p/ ctrl nivel batería 1S
const float BATMIN = 3.4; // Voltaje mínimo tolerable en batería
const char* SSIDWF = "ctrlvl02"; // SSID
const char* CLAVEWF = "ctrlvl02"; // Clave AP wifi
const char* SSIDWF_ST = "id_wf"; // Datos wifi local (solo debug)
const char* CLAVEWF_ST = "clave_wf"; // Datos wifi local (solo debug)
const bool DEBUG = false;

bool procesarUnaVez = true; // Helper p/ ejecutar procesos aislados una sola vez
bool mostrarRDT = true;
bool btnPresionado = false; // Flag indicador estado pulsador
bool ganchoAbierto = false; // Flag gancho abierto o cerrado
bool clienteAPConectado = false; // Flag indicador de interface config conectada
bool secuenciaIniciada = false; // Flag p/ indicar inicio secuencia vuelo

uint8_t idClienteSocket; // ID del cliente websocket
unsigned int paServoEstab = MED_PWM - 1; // Pulso por defecto servo estab
unsigned int frec_parpadeo_activo = 0; // Helper indicador frecuencia parpadeo actual
unsigned long timerInicio, timerDetencion; // Timers generales

struct formatoParametros { int retardoInicio; int tiempoTrepada; int tiempoTransicion; int tiempoVuelo; int offsetEstab; int angRemolque; int angCircular; int angDespegue; int angTransicion; int angVuelo; int angDT; int servoEstabInvertido; } parametros; // parámetros de vuelo

enum modos { LISTO, REMOLCANDO, CIRCULANDO, DESPEGUE, TREPADA, TRANSICION, VUELO, DESTERMALIZADO, DETENIDO } modo; // modos de vuelo

Scheduler tareas; // Gestor de tareas
Bounce debouncerPulsador = Bounce(); // Obj ctrl pulsador
Bounce debouncerFCD = Bounce(); // Obj ctrl final carrera delantero gancho
Bounce debouncerFCS = Bounce(); // Obj ctrl final carrera superior gancho
WebSocketsServer webSocket = WebSocketsServer(1337); // Obj ctrl socket
Servo servoEstab; // Obj ctrl variador
