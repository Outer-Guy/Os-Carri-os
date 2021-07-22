
//Esta matriz guarda informacion de los sensores,
//los primeros dos pines deben ser trigger y echo respectivamente
// sensorPinPair [n°deSensor] === {pinTrigger, pinEcho}
int sensorPinPair[3][2] = {
    {7, 8},
    {2, 4},
    {12, 13},
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
  
  //Por cada lista en la matriz se revisara los primeros dos valores y los activara como pines
  for (int _iArray = 0; _iArray < sizeof(sensorPinPair) / sizeof(sensorPinPair[_iArray]); _iArray++)
  {
    int _tempBool = 0;
    for (int _i = 0; _i < sizeof(sensorPinPair[_iArray]) / sizeof(sensorPinPair[_iArray][_i]); _i++)
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
}

void loop()
{
    checkAllSensors();
}

void checkAllSensors()
{
  //Revisar que los sensores no detecten a otro robot cerca
  //por cada sensor en la matriz, revisar del primero al ultimo
  for (int _iArray = 0; _iArray < sizeof(sensorPinPair) / sizeof(sensorPinPair[_iArray]); _iArray++)
  {
    SensorCheckUltraSound(sensorPinPair[_iArray]);

    while (distanciaSensor < distanciaMinima)
    {
      if (distanciaSensor < distanciaColision)
      {
        //Motor_retroceder
      }
      else
      {
        //Motor_detenerse
      }

      SensorCheckUltraSound(sensorPinPair[0]);
    }
  }

  //Moverse
  
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

