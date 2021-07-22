
//Esta matriz guarda informacion de los sensores,
//los primeros dos pines deben ser trigger y echo respectivamente
// ultrasoundPinPairs [n°deSensor] === {pinTrigger, pinEcho}
int ultrasoundPinPairs[3][2] = {
    {7, 8},
    {2, 4},
    {12, 13},
};

//Los pines que controlan el motor, el primer pin controla la direccion hacia adelante,
//el segundo hacia atras, el tercero controla la velocidad y debe ser un PWM
int motorPins[2][3] ={
        {},
        {},
};


enum RobotState
{
  Libre,
  Ocupado,
  Buscando_Cargador,
  Evitando_Colision,
};

RobotState MyState = Libre;

enum MotorState
{
  Avanzar,
  Detenerse,
  Retroceder,
};

short distanciaSensor;

// Variable auxiliar para el timer del sensor de ultra sonido
long sensorTimer = 0;
// Delay entre medición y medición del sensor de ultra sonido
int sensorTimerDelay = 100;

#pragma region TestingVariables
//Constante de transformación de medida del sensor de ultrasonido
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

  //Controlador de motores toma 3 valores por array
  for (int _iArray = 0; _iArray < sizeof(motorPins) / sizeof(motorPins[_iArray]); _iArray++)
  {
    pinMode(motorPins[_iArray][0], OUTPUT);
    pinMode(motorPins[_iArray][1], OUTPUT);
    pinMode(motorPins[_iArray][2], OUTPUT);
  }
}

void loop()
{
  switch (MyState)
  {
  case Libre:
    /* code */
    break;
  case Ocupado:
    /* code */
    break;
  case Buscando_Cargador:
    /* code */
    break;
  case Evitando_Colision:
    /* code */
    break;
  
  default:
    MyState = Libre;
    break;
  }
  
  // Chequear si el tiempo ya pasó o si el runtime se reseteó para volver a sensar las distancias
  if (sensorTimer > millis() - 100)
  {
    checkAllSensors();
  }
}


void checkAllSensors()
{
  //Revisar que los sensores no detecten a otro robot cerca
  //por cada sensor de ultrasonido en la matriz, revisar del primero al ultimogu
  for (int _iArray = 0; _iArray < sizeof(ultrasoundPinPairs) / sizeof(ultrasoundPinPairs[_iArray]); _iArray++)
  {
    SensorCheckUltraSound(ultrasoundPinPairs[_iArray]);

    while (distanciaSensor < distanciaMinima)
    {
      if (distanciaSensor < distanciaColision)
      {
        
        MotorBase(0,Retroceder,0.5);
        MotorBase(1, Retroceder, 0.5);
      }
      else
      {
        
        MotorBase(0, Detenerse, 0);
        MotorBase(1, Detenerse, 0);
      }

      SensorCheckUltraSound(ultrasoundPinPairs[0]);
    }
  }

  MotorBase(0, Avanzar, 1);
  MotorBase(1, Avanzar, 1);
  sensorTimer = millis() + sensorTimerDelay;
}

void SensorCheckUltraSound(int _pinSet[2])
{
  digitalWrite(_pinSet[0], HIGH);
  delay(1);

  digitalWrite(_pinSet[0], LOW);

  short duracionPulso = pulseIn(_pinSet[1], HIGH);
  distanciaSensor = duracionPulso / sensorConstant;

  //Serial.println(distanciaSensor);
}

void SensorCheckInfraRed()
{
    //code
}

//REVISAR CON MOTOR
//Usado para cambiar el estado de un motor, el numero es el de la lista de motores, la velocidad es de 0 a 1
void MotorBase(int _motorNumber,MotorState _changeMotorState,short _motorSpeed)
{
  int _realMotorSpeed = _motorSpeed * 255;
  //resetea la direccion del motor
  digitalWrite(motorPins[_motorNumber][0], LOW);
  digitalWrite(motorPins[_motorNumber][1], LOW);

  //cuando deberia encenderse 0 y cuando 1 depende de cual gire en que dirección.
  switch (_changeMotorState)
  {
  case Avanzar:
    digitalWrite(motorPins[_motorNumber][1], HIGH);
    analogWrite(motorPins[_motorNumber][2], _realMotorSpeed);
    break;

  case Detenerse:
    analogWrite(motorPins[_motorNumber][2], 0);
    break;

  case Retroceder:
    digitalWrite(motorPins[_motorNumber][0], HIGH);
    analogWrite(motorPins[_motorNumber][2], _realMotorSpeed);
    break;

  default:
    break;
  }
}