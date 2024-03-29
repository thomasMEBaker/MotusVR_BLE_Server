#ifndef BLE_DEVICE_H
#define BLE_DEVICE_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEHIDDevice.h>
#include <string>
#include <Arduino.h>

class ble_device {
public:
  ble_device();

  static ble_device* CreateGamepad();
  void run();

  void setupExampleCode();
  void setupGamepadBLE();
  void toggleAButton();
  void analogStockMovement();
  void setupBatteryService();
};

#endif