#ifndef Variables
#define Variables
//Esta matriz guarda informacion de los sensores,
//los primeros dos pines deben ser trigger y echo respectivamente
// ultrasoundPinPairs [n°deSensor] === {pinTrigger, pinEcho}
int ultrasoundPinPairs[3][2] = {
    {7, 8},
    {2, 4},
    {12, 13},
};
int infraredPins[3] = {
    {},
    {},
    {},
};
//Los pines que controlan el motor, el primer pin controla la direccion hacia adelante,
//el segundo hacia atras, el tercero controla la velocidad y debe ser un PWM
int motorPins[2][3] = {
    {},
    {},
};

//Estado general del robot
//[0]: Estado del robot visto online
//[1]: Objetivo del robot, dado por el sensor de ultrasonido evitando otros robots, o moviendose
//[2]: Dirección como la da el sensor infrarojo
int MyState[3] = {
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
//-moviendose (en alguna dirección)
//-detenido (esperando que otro robot pase)
//-evasión (retrocediendo de otro robot)
//-buscando linea
enum RobotActions
{
  esperando,
  moverse,
  detenido,
  evasion,
  buscando,
};

enum RobotDirection
{
  adelante,
  izquierda,
  derecha,
  cruce,
  perdido,
};

enum MotorDirection
{
  avanzar,
  detenerse,
  retroceder,
};
#endif

// Variable auxiliar para el timer de la medicion de los sensores
long sensorTimer = 0;

#pragma region TestingVariables
// Delay entre medición y medición del sensor de ultra sonido
int sensorTimerDelay = 100;
//Constante de velocidad del sonido m/s
short sensorConstant = 58.2;
//Distancia minima en centimetros antes de detectar una unidad
short distanciaMinima = 100;
//Distancia minima en centimetros antes de considerar un peligro de colision
short distanciaColision = 50;
#pragma endregion

void setup()
{

  Serial.begin(9600);

  //Sensores de Ultrasonido: Por cada lista en la matriz se revisara los primeros dos valores y los activara como pines
  for (int _iArray = 0; _iArray < sizeof(ultrasoundPinPairs) / sizeof(ultrasoundPinPairs[_iArray]); _iArray++)
  {
    int _tempBool = 0;
    for (int _i = 0; _i < sizeof(ultrasoundPinPairs[_iArray]) / sizeof(ultrasoundPinPairs[_iArray][_i]); _i++)
    {

      switch (_tempBool)
      {
      case 0:
        pinMode(_i, OUTPUT);
        _tempBool += 1;
        break;

      case 1:
        pinMode(_i, INPUT);
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
  for (int _iArray = 0; _iArray < sizeof(motorPins) / sizeof(motorPins[_iArray]); _iArray++)
  {
    pinMode(motorPins[_iArray][0], OUTPUT);
    pinMode(motorPins[_iArray][1], OUTPUT);
    pinMode(motorPins[_iArray][2], OUTPUT);
  }

  MyState[estado] = libre;
  MyState[objetivo] = esperando;
  MyState[direccion] = perdido;

  delay(10);
}

void loop()
{

  switch (MyState[estado])
  {
  case libre:
    delay(500);
    //esperar direcciones del servidor,
    //no se que tan bien funcione el online+delay, falta testear
    break;

  case ocupado:
    if (sensorTimer - millis() < sensorTimer - sensorTimerDelay)
    {
      ArtificialIntelligence();
      sensorTimer = millis();
    }
    delay(10);
    break;

  default:
    break;
  }
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
      ChangeMotorOrientation();
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
  RobotActions _newState[3];
  //Revisar que los sensores no detecten a otro robot cerca
  //por cada sensor de ultrasonido en la matriz, revisar del primero al ultimogu
  for (int _iArray = 0; _iArray < sizeof(ultrasoundPinPairs) / sizeof(ultrasoundPinPairs[_iArray]); _iArray++)
  {
    short distanciaSensor = UltraSoundPulseCheck(ultrasoundPinPairs[_iArray]);

    if (distanciaSensor < distanciaColision)
    {
      _newState[_iArray] = evasion;
    }
    else if (distanciaSensor < distanciaMinima)
    {
      _newState[_iArray] = detenido;
    }
    else
    {
      _newState[_iArray] = moverse;
    }
  }
  for (int _iArray = 0; _iArray < sizeof(_newState) / sizeof(_newState[_iArray]); _iArray++)
  {
    if (_newState[_iArray] != MyState[objetivo])
    {
      MyState[objetivo] = _newState[_iArray];
      return true;
    }
    else
    {
      return false;
    }
  }
}

  //returns true if the value changed
  bool CheckInfraRedStep()
  {
    bool _results[3] = {};
    RobotDirection _newState;

    for (int _iArray = 0; _iArray < sizeof(infraredPins) / sizeof(infraredPins[_iArray]); _iArray++)
    {
      _results[_iArray] = digitalRead(infraredPins[_iArray]);
    }

    if (!_results[0])
    {
      //nada a la izquierda
      if (!_results[direccion])
      {
        //Y nada a la derecha
        if (_results[1])
        {
          MyState[direccion] = adelante;
        }
        else
        {
          MyState[direccion] = perdido;
        }
      }

      else
      {
        MyState[direccion] = derecha;
      }
    }
    else
    {
      //algo a la izquierda
      if (!_results[2])
      {
        if (_results[1] | !_results[1])
        {
          MyState[direccion] = izquierda;
        }
      }

      else if (_results[1])
      {
        MyState[direccion] = cruce;
      }
    }
    if (_newState != MyState[direccion])
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  short UltraSoundPulseCheck(int _pinSet[2])
  {
    digitalWrite(_pinSet[0], HIGH);
    delayMicroseconds(100);

    digitalWrite(_pinSet[0], LOW);

    short duracionPulso = pulseIn(_pinSet[1], HIGH);
    return (duracionPulso / sensorConstant);

    //Serial.println(distanciaSensor);
  }

  void ChangeMotorOrientation()
  {
    switch (MyState[direccion])
    {
    case adelante:
      MotorBase(0, avanzar, 1);
      MotorBase(1, avanzar, 1);
      break;
    case izquierda:
      MotorBase(0, detenerse, 0);
      MotorBase(1, avanzar, 1);
      break;
    case derecha:
      MotorBase(0, avanzar, 1);
      MotorBase(1, detenerse, 0);
      break;
    case cruce:
      //aca deberia decidir la IA que camino tomar
      break;
    default:
      break;
    }
  }

  void ChangeMotorDirection()
  {
    switch (MyState[objetivo])
    {
    case moverse:
      ChangeMotorOrientation();
      break;
    case detenido:
      MotorBase(0, detenerse, 0);
      MotorBase(1, detenerse, 0);
      break;
    case evasion:
      MotorBase(0, retroceder, 0.5);
      MotorBase(1, retroceder, 0.5);
      break;
    default:
      break;
    }
  }

  //REVISAR CON MOTOR
  //Usado para cambiar el estado de un motor, el numero es el de la lista de motores, la velocidad es de 0 a 100%
  void MotorBase(int _motorNumber, MotorDirection _changeMotorState, short _motorSpeed)
  {
    int _realMotorSpeed = 255 * _motorSpeed/100;
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