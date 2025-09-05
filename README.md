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

<br>

## Carga inicial de parámetros en memoria permanente:
Modificar temporalmente en main.cpp la llamada a init_system con true:
```
init_system(true);
```

Compilar y subir el firmware, luego volver a false, compilar y subir nuevamente, para permitir que los futuros cambios desde la app queden almacenados.

<br>

## Ajuste de punto de acceso:
La placa funcionará como access point wifi (AP) para permitir la conexión de una app de configuración de parámetros. Para ello, modificar en config.h las credenciales:
```
const char* WIFI_AP_NAME = "NOMBREAP";
const char* WIFI_AP_PASS = "clave_minimo_8_digitos";
```

<br>

## Configuración de parámetros de vuelo:
Colocar la placa en modo CONFIGURACION y conectarse al punto de acceso que provee (configurado en el paso anterior como WIFI_AP_NAME). Se puede utilizar un teléfono, tablet o computadora sin problemas.

Abrir una ventana de navegador y visitar la siguiente dirección para descargar la app:
```
https://f1timer.up.railway.app/
```

La app conectará al websocket y recuperará los valores de parámetros actuales. Habilitará también un botón para instalar, si se lo pulsa, generará un ícono de acceso rápido en el escritorio del dispositivo desde el cual se accede, para mayor comodidad.

Los parámetros de tiempo (transición, vuelo, etc) se deben colocar en MILISEGUNDOS, y los de ángulos en GRADOS. El CERO del estabilizador, se considera observando el fuse lateralmente, con el estabilizador a la DERECHA, y alineado con la línea central principal del fuse. Desde esta posición, los ángulos ANTIHORARIOS se toman como NEGATIVOS y los HORARIOS como POSITIVOS.

Ejemplo: para indicar ángulo de trepada o circular, se utilizarán números NEGATIVOS (el estabilizador debe rotar desde el CERO en sentido ANTIHORARIO); para transición, un valor POSITIVO (rota en sentido HORARIO).

No es necesario guardar luego de cambiar cada parámetro, si se deben ajustar varios, se pueden modificar y luego pulsar GUARDAR para actualizar todos, el timer parpadea más rápido unos instantes para confirmar que almacena correctamente.

Al finalizar, simplemente se desconecta el dispositivo para salir de CONFIGURACION y se lo vuelve a conectar para iniciar en modo normal de VUELO.