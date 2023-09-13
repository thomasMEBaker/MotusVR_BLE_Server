#include "motus_imu.h"

motus_imu::motus_imu()
{
  Serial.begin(115200);
  Serial.println("BNO080 Setup");
  Wire.begin();

   if (myIMU.begin() == false)
  {
    Serial.println("BNO080 not detected at default I2C address. Check your jumpers and the hookup guide. Freezing...");
    while (1);
  }

  Wire.setClock(400000); //Increase I2C data rate to 400kHz

  //myIMU.calibrateAll(); //Turn on cal for Accel, Gyro, and Mag - currently not implemented
  myIMU.enableGameRotationVector(100); //Send data update every 100ms
  myIMU.enableAccelerometer(100); //Send data update every 50ms
}

 motus_imu* motus_imu::CreateIMU(void)
{
	//esp_log_level_set("*", ESP_LOG_DEBUG);
	motus_imu* IMU = new motus_imu();
	return IMU;
}

void motus_imu::printRotationVector(){
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

void motus_imu::printAccelerometer(){
    Serial.print(F(","));
    Serial.print(y, 2);
    Serial.print(F(","));
    Serial.print(z, 2);
    Serial.print(F(","));
}

void motus_imu::plotAccelerometer(){
  int upperLim = 20;
  int lowLim = -20;
  double kalmanFiltered = KalmanFilter(x);

  Serial.print(lowLim);
  Serial.print(F(","));
  Serial.print(upperLim);
  Serial.print(F(","));
  Serial.print(x, 2);
  Serial.print(F(","));
  Serial.println(kalmanFiltered);
}

double motus_imu::KalmanFilter(double inputVal){
		z_real = inputVal;
		//do a prediction
		x_temp_est = x_est_last;
		P_temp = P_last + qKal;
		//calculate the Kalman gain
		K = (P_temp * (1.0 / (P_temp + rkal)));
		//measure
    int randomNumber = random(101);
    float randomFloat = random(1000) / 1000.0;
    z_measured = z_real + randomFloat * 0.09;
		//correct
		x_est = x_temp_est + K * (z_measured - x_temp_est);
		P = (1 - K) * P_temp;
		sum_error_measure += fabs(z_real - z_measured);
		//update our last's
		P_last = P;
		x_est_last = x_est;

		return x_est;
}