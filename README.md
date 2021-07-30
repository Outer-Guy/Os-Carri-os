# Cosas Importantes

Para trabajar en el módulo Wi-Fi:
La placa que tenemos actualmente es una "NodeMCU 1.0 ESP8266 (12E)"
Para programarla se necesita preparar el entorno de desarrollo (Ya sea la Arduino IDE o VSCode)
Video con guía acá: https://www.aprendiendoarduino.com/2018/01/27/video-preparacion-ide-arduino-para-esp8266/

Cualquier pregunta a Discord

"checkAllSensors"
Por ahora los sensores funcionan de manera primitiva, se detienen al detectar a otro robot a su derecha y retroceden de ser necesario.
Seria bueno poder integrar un modo de precaución y alguna manera de integrar los sensores infrarojos en la ecuación
Cuando el robot detecta a otro robot, este queda encerrado en un bucle en el que no puede hacer nada hasta salir de este estado
hay que actualizar los motores en otro paso

TinkerCAD odia bastante funcionar
#ifndef a
#def a
#endif
No funciona, por lo que utilizar los enums dentro de la declaracion de una funcion no se puede, va a haber que tener esto en cuenta a la hora de probar el codigo
