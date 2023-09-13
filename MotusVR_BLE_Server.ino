#include "ble_device.h"

#define LED_BUILTIN 2

/*
To do -
Fix issue with connection on PC
Sort out gamepad movement and range of scale
Change to dynamically changing mac address name
Connection and disconnection - move into classes and BLEServerCallbacks
Implement BNO080
*/

//https://github.com/nkolban/esp32-snippets/blob/master/Documentation/BLE%20C%2B%2B%20Guide.pdf
//https://github.com/nkolban/esp32-snippets/issues
//https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE

static ble_device *gp;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Starting BLE");
  gp=ble_device::CreateGamepad();
}

void loop() {
  delay(100);
}
/*
void setupBatteryService() {
  batteryService = pServer->createService(BATTERY_SERVICE_UUID);
  batteryLevelCharacteristic = batteryService->createCharacteristic(BATTERY_LEVEL_UUID, BLECharacteristic::PROPERTY_READ);
  uint8_t initialBatteryLevel = 50;
  batteryLevelCharacteristic->setValue(&initialBatteryLevel, 1);
  batteryService->start();
  pAdvertising->addServiceUUID(batteryService->getUUID());
}

//test functions

void toggleAButton() {
  //function to test gamepad on android & PC - use 3v to onto D23 to toggle A press on
  if (sendValues) {
    if (digitalRead(23) == HIGH) {
      uint8_t gamepadReport[] = { BUTTON_A_INDEX, 0, 0, 0 };
      pInput->setValue(gamepadReport, sizeof(gamepadReport));
      pInput->notify();
    } else {
      uint8_t gamepadReport[] = { 0, 0, 0, 0 };
      pInput->setValue(gamepadReport, sizeof(gamepadReport));
      pInput->notify();
    }
  }
}

void analogStockMovement() {
  // Simulate analog stick movement
  xAxis = (xAxis + 1) % 256;
  yAxis = (yAxis + 1) % 256;
  uint8_t gamepadReport[] = { 0, 0, xAxis, yAxis };
  pInput->setValue(gamepadReport, sizeof(gamepadReport));
  pInput->notify();
}
*/

