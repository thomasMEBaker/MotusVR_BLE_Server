#include "ble_device.h"

#define LED_BUILTIN 2

/*
To do -
Fix issue with connection on PC
Sort out gamepad movement and range of scale
Change to dynamically changing mac address name
*/

//https://github.com/nkolban/esp32-snippets/blob/master/Documentation/BLE%20C%2B%2B%20Guide.pdf
//https://github.com/nkolban/esp32-snippets/issues
//https://github.com/espressif/arduino-esp32/tree/master/libraries/BLE

static ble_device *gp;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Starting BLE");
  gp = ble_device::CreateGamepad();

  /*
  imu=motus_imu::CreateIMU(); //not needed currently
  */
}

void loop() {
  delay(100);
}
