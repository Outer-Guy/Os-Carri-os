//se debe crear una variable indefinida y llamar esta funcion para que el compilador de arduino permita tener el codigo aca arriba
#ifndef Variables
#define Variables
//Esta matriz guarda informacion de los sensores,
//los primeros dos pines deben ser trigger y echo respectivamente
// ultrasoundPinPairs [n°deSensor] === {pinTrigger, pinEcho}
//los sensores deberian setearse horariamente empezando desde la izquierda dando la vuelta completamente
uint8_t ultrasoundPinPairs[1][2] = {
    {12, 11},

};
//el orden de los pines deberia ser de izquiera a derecha
uint8_t infraredPins[3] = {2, 3, 4};
//Los pines que controlan el motor, el primer pin controla la direccion hacia adelante,
//el segundo hacia atras, el tercero controla la velocidad y debe ser un PWM
uint8_t motorPins[2][3] = {
    {9, 8, 10}, {6, 7, 5}};

//Estado general del robot
//[0]: Estado del robot visto online
//[1]: Objetivo del robot, dado por el sensor de ultrasonido evitando otros robots, o moviendose
//[2]: Dirección como la da el sensor infrarojo
uint8_t MyState[3] = {
    {},
    {},
    {},
};

#pragma endregion

#pragma region EnumStates

//las variables que maneja la IA en myState:
//-estado
//-objetivo  (robot actions)
//-direccion
enum RobotState
{
  estado,
  objetivo,
  direccion,
};

//el robot puede estar:
//-libre
//-ocupado llendo a algun lado
// o buscando un cargador
enum RobotStatus
{
  libre,
  ocupado,
  buscando_Cargador,
};

//los diferentes objetivos del robot:
//-esperando
//-Serial (en alguna dirección)
//-detenido (esperando que otro robot pase)
//-evasión (retrocediendo de otro robot)
//-buscando linea
//cada valor indica la urgencia del mismo, y es usado en el CheckUltraSoundStep
enum RobotActions
{
  esperando = 0,
  moverse = 1,
  detenido = 2,
  evasion = 3,
  buscando = 4,
};

enum RobotDirection
{
  adelante,
  izquierda,
  derecha,
  izquierdaExtra,
  derechaExtra,
  cruce,
  perdido,
};

enum MotorDirection
{
  avanzar,
  detenerse,
  retroceder,
};

uint16_t directivas[] = {
    0,
    0,
    0};
uint8_t directivaActual = 0;

uint8_t lastDirection = 0;

long delayCruzes = 0;

#endif

// Variable auxiliar para el timer de la medicion de los sensores
long sensorTimer = 0;

#pragma region TestingVariables
// Delay entre medición y medición del sensor de ultra sonido
uint8_t sensorTimerDelay = 100;
//Constante de velocidad del sonido m/s
uint8_t sensorConstant = 58.2;
//Distancia minima en centimetros antes de detectar una unidad
uint8_t distanciaMinima = 20;
//Distancia minima en centimetros antes de considerar un peligro de colision
uint8_t distanciaColision = 10;

//el motor izquierdo es mas rapido
uint8_t startingSpeed1 = 36;
uint8_t startingSpeed2 = 42;

//uint8_t normalSpeed1 = 30;
//uint8_t normalSpeed2 = 29;
#pragma endregion

const char *WFSCB = "!"; // Wi-Fi Serial Command Beginer
const char *WFSCT = "|"; // Wi-Fi Serial Command Terminator
bool active = false;

void setup()
{

  Serial.begin(9600);

  //Sensores de Ultrasonido: Por cada lista en la matriz se revisara los primeros dos valores y los activara como pines
  for (uint8_t _iArray = 0; _iArray < sizeof(ultrasoundPinPairs) / sizeof(ultrasoundPinPairs[_iArray]); _iArray++)
  {
    uint8_t _tempBool = 0;
    for (uint8_t _i = 0; _i < sizeof(ultrasoundPinPairs[_iArray]) / sizeof(ultrasoundPinPairs[_iArray][_i]); _i++)
    {

      switch (_tempBool)
      {
      case 0:
        pinMode(ultrasoundPinPairs[_iArray][_i], OUTPUT);
        _tempBool += 1;
        break;

      case 1:
        pinMode(ultrasoundPinPairs[_iArray][_i], INPUT);
        _tempBool += 1;
        break;

      default:
        break;
      }
    }
  }

  //Sensores infrarojos, cada uno tiene un solo pin
  for (int _iArray = 0; _iArray < sizeof(infraredPins) / sizeof(infraredPins[_iArray]); _iArray++)
  {
    pinMode(infraredPins[_iArray], INPUT);
  }

  //Controlador de motores toma 3 valores por array
  for (uint8_t _iArray = 0; _iArray < sizeof(motorPins) / sizeof(motorPins[_iArray]); _iArray++)
  {
    pinMode(motorPins[_iArray][0], OUTPUT);
    pinMode(motorPins[_iArray][1], OUTPUT);
    pinMode(motorPins[_iArray][2], OUTPUT);
  }

  MyState[estado] = libre;
  MyState[objetivo] = esperando;
  MyState[direccion] = perdido;
  Serial.println("");
  Serial.print("--------------------");
  Serial.print("INICIANDO ATLAS:");
  Serial.println("--------------------");
  delay(4000);
}

//REVISAR CON DIRECCIONES DEL SERVIDOR
//la IA decide cuando actualizar los sensores y que hacer en todo momento
void ArtificialIntelligence()
{
  switch (MyState[objetivo])
  {
  case moverse:

    //si se detecta un cambio, cambiar el estado del motor
    if (CheckUltraSoundStep() == true)
    {
      ChangeMotorDirection();
    }

    else if (CheckInfraRedStep() == true)
    {
      ChangeMotorOrientation(MyState[direccion]);
    }
    break;

  case detenido:
    if (CheckUltraSoundStep() == true)
    {
      ChangeMotorDirection();
    }
    break;
  case evasion:
    if (CheckUltraSoundStep() == true)
    {
      ChangeMotorDirection();
    }
    if (CheckInfraRedStep() == true)
    {
      //esto es lo que debe hacer el robot cuando esta retrocediendo y se sale de la linea
    }
    break;

  case buscando:
    //buscar el camino de vuelta
    break;

  default:
    if (sensorTimer - millis() < sensorTimer - sensorTimerDelay)
    {
      CheckUltraSoundStep();
      CheckInfraRedStep();
    }
    break;
  }
}

//returns true if the value changed
bool CheckUltraSoundStep()
{
  RobotActions _newState = esperando;
  uint8_t _distanciaFinal = 255;

  //Revisar que los sensores no detecten a otro robot cerca
  //por cada sensor de ultrasonido en la matriz, revisar del primero al ultimogu
  for (uint8_t _iArray = 0; _iArray < sizeof(ultrasoundPinPairs) / sizeof(ultrasoundPinPairs[_iArray]); _iArray++)
  {
    uint8_t distanciaSensor = UltraSoundPulseCheck(_iArray);
    Serial.print(distanciaSensor);
    Serial.print(", ");
    if (distanciaSensor < _distanciaFinal)
    {
      Serial.print("(");
      Serial.print(distanciaSensor);
      Serial.print("<");
      Serial.print(_distanciaFinal);
      Serial.print(")");
      _distanciaFinal = distanciaSensor;
      Serial.println(_distanciaFinal);
    }
  }
  // Serial.print(".");
  // Serial.print(_distanciaFinal);

  if (_distanciaFinal < distanciaColision)
  {
    _newState = detenido;
  }
  else if (_distanciaFinal < distanciaMinima)
  {
    _newState = detenido;
  }
  else
  {
    _newState = moverse;
  }

  if (_newState == MyState[objetivo])
  {
    return false;
  }
  else
  {
    MyState[objetivo] = _newState;
    return true;
  }
}

//returns true if the value changed
bool CheckInfraRedStep()
{
  //  Serial.print("pines: ");
  uint8_t _results[3] = {};
  RobotDirection _newState;

  for (uint8_t _iArray = 0; _iArray < sizeof(infraredPins) / sizeof(infraredPins[_iArray]); _iArray++)
  {
    _results[_iArray] = digitalRead(infraredPins[_iArray]);
    // Serial.print(_results[_iArray]);
    //  Serial.print(",");
  }
  //  Serial.println("");

  // LA GENERACION DE IZQUIERDA EXTRA Y DERECHA EXTRA FUERON QUITADAS PARA SU PROXIMA IMPLEMENTACIÓN

  //nada a la izquierda
  if (!_results[0])
  {

    //Y nada a la derecha
    if (!_results[2])
    {
      //Y algo en el medio
      if (_results[1])
      {
        _newState = adelante;
      }
      //todos estan en 0
      else
      {
        _newState = perdido;
      }
    }
    //Algo a la derecha
    else
    {
      //algo en el medio
      if (_results[1])
      {
        _newState = derecha;
      }
      else
      {
        _newState = derecha;
      }
      lastDirection = derechaExtra;
    }
  }
  //algo a la izquierda
  else
  {
    //nada a la derecha
    if (!_results[2])
    { //nada en el medio
      if (!_results[1])
      {
        _newState = izquierda;
      }
      else
      {
        _newState = izquierda;
      }
      lastDirection = izquierdaExtra;
    }
    //algo a la derecha
    else
    {
      //algo a ambos lados
      _newState = cruce;
    }
  }

  if (_newState == perdido)
  {
    if (MyState[direccion] == adelante)
    {
      MyState[direccion] = perdido;
    }
    else
    {
      MyState[direccion] = lastDirection;
    }
    return true;
  }
  else if (_newState != MyState[direccion])
  {

    MyState[direccion] = _newState;
    return true;
  }
  else
  {
    return false;
  }
}

uint8_t UltraSoundPulseCheck(uint8_t _pinPair)
{

  digitalWrite(ultrasoundPinPairs[_pinPair][0], HIGH);
  delayMicroseconds(10);

  digitalWrite(ultrasoundPinPairs[_pinPair][0], LOW);

  uint16_t duracionPulso = pulseIn(ultrasoundPinPairs[_pinPair][1], HIGH);

  duracionPulso = duracionPulso / sensorConstant;
  if (duracionPulso < 255)
  {
    return (duracionPulso);
  }
  else
  {
    return (255);
  }

  //Serial.println(distanciaSensor);
}

void ChangeMotorOrientation(uint8_t _direccion)
{
  // Serial.println("");
  // Serial.println(".");

  switch (_direccion)
  {
  case adelante:
    MotorBase(0, avanzar, startingSpeed2);
    MotorBase(1, avanzar, startingSpeed1);

    break;
  case izquierda:
    MotorBase(0, avanzar, startingSpeed2);
    MotorBase(1, avanzar, startingSpeed1 * 1.5);

    break;
  case izquierdaExtra:
    MotorBase(0, retroceder, startingSpeed2 + 5);
    MotorBase(1, avanzar, startingSpeed1);
    break;
  case derecha:
    MotorBase(0, avanzar, startingSpeed2 * 1.5);
    MotorBase(1, avanzar, startingSpeed1);

    break;
  case derechaExtra:
    MotorBase(0, avanzar, startingSpeed2);
    MotorBase(1, retroceder, startingSpeed1 + 5);

    break;
  case cruce:
    if (millis() - delayCruzes > 2000)
    {

      uint8_t _new_state;
      Serial.println("");
      Serial.println(millis());
      Serial.println(", ");
      Serial.println(delayCruzes);
      Serial.println("Cruce: ");
      Serial.print(directivaActual);
      Serial.print(", ");
      Serial.print(directivas[directivaActual]);
      Serial.println(".");

      if (directivas[directivaActual] > 100)
      {
        MotorBase(0, detenerse, 0);
        MotorBase(1, detenerse, 0);
        delay(directivas[directivaActual]);

        if (directivaActual >= sizeof(directivas) / sizeof(uint16_t))
        {
          active = false;
          return;
        }
        else
        {
          directivaActual += 1;
        }
      }

      _new_state = directivas[directivaActual];
      if (directivaActual >= sizeof(directivas) / sizeof(uint16_t))
      {
        MotorBase(0, detenerse, 0);
        MotorBase(1, detenerse, 0);
        active = false;
        return;
      }
      else
      {
        directivaActual += 1;
      }

      ChangeMotorOrientation(_new_state);
      delayCruzes = millis();
    }
    else
    {
      ChangeMotorOrientation(lastDirection);
    }

    break;

  default:
    MotorBase(0, retroceder, startingSpeed2);
    MotorBase(1, retroceder, startingSpeed1);
    break;
  }
}

void ChangeMotorDirection()
{
  switch (MyState[objetivo])
  {
  case moverse:
    ChangeMotorOrientation(MyState[direccion]);
    break;
  case detenido:
    MotorBase(0, detenerse, 0);
    MotorBase(1, detenerse, 0);
    break;
  case evasion:
    MotorBase(0, retroceder, 50);
    MotorBase(1, retroceder, 50);
    break;
  default:
    break;
  }
}

//revisado con TinkerCAD
//Usado para cambiar el estado de un motor, el numero es el de la lista de motores, la velocidad es de 0 a 100%
void MotorBase(uint8_t _motorNumber, MotorDirection _changeMotorState, uint8_t _motorSpeed)
{
  uint8_t _realMotorSpeed = 255 * _motorSpeed / 100;
  //resetea la direccion del motor
  digitalWrite(motorPins[_motorNumber][0], LOW);
  digitalWrite(motorPins[_motorNumber][1], LOW);

  //cuando deberia encenderse 0 y cuando 1 depende de cual gire en que dirección.
  switch (_changeMotorState)
  {
  case avanzar:
    digitalWrite(motorPins[_motorNumber][1], HIGH);
    analogWrite(motorPins[_motorNumber][2], _realMotorSpeed);
    break;

  case detenerse:
    analogWrite(motorPins[_motorNumber][2], 0);
    break;

  case retroceder:
    digitalWrite(motorPins[_motorNumber][0], HIGH);
    analogWrite(motorPins[_motorNumber][2], _realMotorSpeed);
    break;

  default:
    break;
  }
}

void TestMovimientoBasico()
{
  MotorBase(0, avanzar, startingSpeed1);
  MotorBase(1, avanzar, startingSpeed2);
  delay(1000);

  MotorBase(0, detenerse, startingSpeed1);
  MotorBase(1, detenerse, startingSpeed2);
  delay(500);

  MotorBase(0, retroceder, startingSpeed1);
  MotorBase(1, retroceder, startingSpeed2);
  delay(1000);

  MotorBase(0, detenerse, startingSpeed1);
  MotorBase(1, detenerse, startingSpeed2);
  delay(500);

  MotorBase(0, retroceder, startingSpeed1);
  MotorBase(1, avanzar, startingSpeed2);
  delay(2000);

  MotorBase(0, detenerse, startingSpeed1);
  MotorBase(1, detenerse, startingSpeed2);
  delay(500);

  MotorBase(0, avanzar, startingSpeed1);
  MotorBase(1, retroceder, startingSpeed2);
  delay(2000);

  MotorBase(0, detenerse, startingSpeed1);
  MotorBase(1, detenerse, startingSpeed2);
  delay(100);
}

void handleWiFi()
{
  if (Serial.available())
  {
    String serialString = Serial.readStringUntil(WFSCT[0]);

    if (serialString.startsWith(WFSCB))
    {
      if (serialString.charAt(1) == 'l')
      {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      }
      if (serialString.charAt(1) == 'a')
      {
        active = !active;
        if (!active)
        {
          MotorBase(0, detenerse, 0);
          MotorBase(1, detenerse, 0);
        }
        delayCruzes = millis();
        directivaActual = 0;
      }
      if (serialString.charAt(1) == 'e')
      {
        if (serialString.charAt(2) == '1')
        {
          active = true;

          delayCruzes = millis();
          directivaActual = 2;
        }
        if (serialString.charAt(2) == '2')
        {
          active = true;

          delayCruzes = millis();
          directivaActual = 1;
        }
        if (serialString.charAt(2) == '3')
        {
          active = true;

          delayCruzes = millis();
          directivaActual = 0;
        }
      }
    }
  }
}

void loop()
{
  handleWiFi();
  if (active)
  {
    bool b = CheckInfraRedStep();

    // if (UltraSoundPulseCheck(0) < 10)
    // {
    //   MotorBase(0, detenerse, 0);
    //   MotorBase(1, detenerse, 0);
    // }
    if (b)
    {
      ChangeMotorOrientation(MyState[direccion]);
    }
    else
    {
      delay(50);
    }
  }
  else
  {
    delay(50);
  }
}
