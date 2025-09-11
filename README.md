# F1A TIMER Versión 3 para Wemos D1 Mini (ESP8266)

<br>

## Compilación:
Clonar e inicializar proyecto en VSCode / PlatformIO. El sistema debe instalar automáticamente las librerías requeridas y compilar sin warnings.

<br>

## Limpieza de placa:
Para evitar problemas, si la placa ya fue usada en otras pruebas o proyectos, limpiar completamente memoria con esptool, utilizando el siguiente comando:
```
esptool --chip esp8266 erase_flash
```
El sistema buscará automáticamente un dispositivo ESP8266 y limpiará la flash. Si no se cuenta con esptool, instalar [Python](https://www.python.org/) en el equipo y luego agregar la herramienta con:
```
pip install esptool
```
<br>

## Modos de uso:
 - VUELO: conectando placa con gancho libre (en posición de circular). El led testigo permanece encendido.
 - CONFIGURACION: conectando la placa con gancho adelante (en posición de remolque). El led testigo parpadea.
 
No se puede cambiar de modo por soft, es necesario desconectar el timer.

<br>

## Carga inicial de parámetros en memoria permanente:
Modificar temporalmente en main.cpp la llamada a init_system con true:
```
init_system(true);
```

Compilar y subir el firmware. El sistema cargará al timer los valores predeterminados que estén configurados en el struct params_format dentro de params.h. Luego volver la llamada a false, compilar y subir nuevamente, para permitir que tome los cambios de configuración que lleguen desde la app, sino en cada inicio volverá a escribir los predeterminados.

<br>

## Ajuste de punto de acceso:
La placa funcionará como access point wifi (AP) para permitir la conexión de una app de configuración de parámetros. Para ello, modificar en config.h las credenciales:
```
const char* WIFI_AP_NAME = "NOMBREAP";
const char* WIFI_AP_PASS = "clave_minimo_8_digitos";
```

<br>

## App de configuración
La app para Android está disponible en [https://github.com/mundostr/f1a_config_python](https://github.com/mundostr/f1a_config_python) o [https://github.com/mundostr/f1a_config_cordova](https://github.com/mundostr/f1a_config_cordova). Descargar el APK de instalación desde Releases.

## Configuración de parámetros de vuelo:
Colocar la placa en modo CONFIGURACION y conectar el teléfono al punto de acceso que provee (configurado en el paso anterior como WIFI_AP_NAME).

Abrir la app y pulsar el botón Conectar. El sistema enlazará al websocket y recuperará los valores de parámetros actuales desde el timer.

Los parámetros de tiempo (transición, vuelo, etc) se deben colocar en MILISEGUNDOS, EXCEPTO el tiempo de vuelo que se indica en SEGUNDOS para más comodidad; los parámetros de ángulos por su parte, se completan en GRADOS.

El CERO del estabilizador, se considera observando el fuse lateralmente, con el estabilizador a la DERECHA, y alineado con la línea central principal del fuse. Desde esta posición, los ángulos ANTIHORARIOS se toman como NEGATIVOS y los HORARIOS como POSITIVOS.

Ejemplo: para indicar ángulo de trepada o circular, se utilizarán números NEGATIVOS (el estabilizador debe rotar desde el CERO en sentido ANTIHORARIO); para transición, un valor POSITIVO (rota en sentido HORARIO).

No es necesario guardar luego de cambiar cada parámetro, si se deben ajustar varios, se pueden modificar y luego pulsar GUARDAR para actualizar todos, el timer parpadea más rápido unos instantes para confirmar que almacena correctamente.

Al finalizar, simplemente se desconecta el dispositivo para salir de CONFIGURACION y se lo vuelve a conectar para iniciar en modo normal de VUELO.
