#ifndef MOTUS_IMU_H
#define MOTUS_IMU_H

#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_BNO080_Arduino_Library.h"

class motus_imu {
public:
  motus_imu();

  BNO080 myIMU;

  float quatI;
  float quatJ;
  float quatK;
  float quatReal;
  float quatRadianAccuracy;
  float x;
  float y;
  float z;

  //Kalman Filter
  float x_est_last = 0;
  float P_last = 0;
  //the noise in the system
  float qKal = 0.004;
  float rkal = 0.617;
  float K;
  float P;
  float P_temp;
  float x_temp_est;
  float x_est;
  float z_measured;    //the 'noisy' value we measured
  float z_real = 0.5;  //the ideal value we wish to measure
  float sum_error_kalman = 0;
  float sum_error_measure = 0;

  static motus_imu* CreateIMU();

  void printRotationVector();
  void printAccelerometer();
  void plotAccelerometer();
  double KalmanFilter(double inputVal);
};
#endif