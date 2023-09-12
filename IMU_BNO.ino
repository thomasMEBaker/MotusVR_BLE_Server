
#include <Wire.h>

#include "SparkFun_BNO080_Arduino_Library.h" 
BNO080 myIMU;

float quatI;
float quatJ; 
float quatK;
float quatReal;
float quatRadianAccuracy;
float x;
float y;
float z;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("BNO080 Setup");

  Wire.begin();

  if (myIMU.begin() == false)
  {
    Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1);
  }

  Wire.setClock(400000); //Increase I2C data rate to 400kHz

  myIMU.calibrateAll(); //Turn on cal for Accel, Gyro, and Mag
  myIMU.enableGameRotationVector(100); //Send data update every 100ms
  myIMU.enableAccelerometer(100); //Send data update every 50ms
}

void loop()
{
  //Look for reports from the IMU
    if (myIMU.dataAvailable() == true)
    {
      quatI = myIMU.getQuatI();
      quatJ = myIMU.getQuatJ();
      quatK = myIMU.getQuatK();
      quatReal = myIMU.getQuatReal();
      quatRadianAccuracy = myIMU.getQuatRadianAccuracy();

      x = myIMU.getAccelX();
      y = myIMU.getAccelY();
      z = myIMU.getAccelZ();

      plotAccelerometer();
  }
}

void printRotationVector(){
    Serial.print(quatI, 2);
    Serial.print(F(","));
    Serial.print(quatJ, 2);
    Serial.print(F(","));
    Serial.print(quatK, 2);
    Serial.print(F(","));
    Serial.print(quatReal, 2);
    Serial.print(F(","));
    Serial.print(quatRadianAccuracy, 2);
    Serial.print(F(","));
}

void printAccelerometer(){
    Serial.print(F(","));
    Serial.print(y, 2);
    Serial.print(F(","));
    Serial.print(z, 2);
    Serial.print(F(","));
}

void plotAccelerometer(){
  int upperLim = 20;
  int lowLim = -20;
  Serial.print(lowLim);
  Serial.print(F(","));
  Serial.print(upperLim);
  Serial.print(F(","));
  Serial.println(x, 2);
}