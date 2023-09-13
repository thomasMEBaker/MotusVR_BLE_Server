#include "ble_device.h"

#define LED_BUILTIN 2

// See the following for generating UUIDs - https://www.uuidgenerator.net/
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BATTERY_SERVICE_UUID "0000180F-0000-1000-8000-00805F9B34FB"
#define BATTERY_LEVEL_UUID "0000180F-0000-1000-8000-00805F9B34FB"

static BLEServer* pServer = NULL;
static BLEAdvertising* pAdvertising;
static BLEHIDDevice* hid;
static BLEService* batteryService;
static BLECharacteristic* pCharacteristic = NULL;
static BLECharacteristic* batteryLevelCharacteristic;

static BLECharacteristic* pInput = NULL;
static BLECharacteristic* pOutput = NULL;

std::string manufacturerName = "MotusVR";
bool sendValues = false;
uint32_t passKey = 0;

static uint8_t buttonState = 0;
static uint8_t xAxis = 0;
static uint8_t yAxis = 0;

// Define constants for button indices
const int BUTTON_A_INDEX = 1;  // Index of the A button

const uint8_t reportMapGamepad[] = {
  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x05,  // USAGE (Gamepad)
  0xA1, 0x01,  // COLLECTION (Application)
  0x85, 0x01,  //   REPORT_ID (1)

  // Buttons (1 to 16)
  0x05, 0x09,  //   USAGE_PAGE (Button)
  0x19, 0x01,  //   USAGE_MINIMUM (Button 1)
  0x29, 0x10,  //   USAGE_MAXIMUM (Button 16)
  0x15, 0x00,  //   LOGICAL_MINIMUM (0)
  0x25, 0x01,  //   LOGICAL_MAXIMUM (1)
  0x95, 0x10,  //   REPORT_COUNT (16)
  0x75, 0x01,  //   REPORT_SIZE (1)
  0x81, 0x02,  //   INPUT (Data,Var,Abs)

  // Padding to align to the next byte
  0x95, 0x06,  //   REPORT_COUNT (6)
  0x75, 0x02,  //   REPORT_SIZE (2)
  0x81, 0x03,  //   INPUT (Cnst,Var,Abs)

  // Analog Axes (X and Y)
  0x05, 0x01,  //   USAGE_PAGE (Generic Desktop)
  0x09, 0x30,  //   USAGE (X)
  0x09, 0x31,  //   USAGE (Y)
  0x15, 0x81,  //   LOGICAL_MINIMUM (-127)
  0x25, 0x7F,  //   LOGICAL_MAXIMUM (127)
  0x75, 0x08,  //   REPORT_SIZE (8)
  0x95, 0x02,  //   REPORT_COUNT (2)
  0x81, 0x02,  //   INPUT (Data,Var,Abs)

  0xC0  // END_COLLECTION
};

class MyCallbacks : public BLEServerCallbacks {
public:
  void onConnect(BLEServer* pServer) {
    Serial.println("Connected device!");
    sendValues = true;
    digitalWrite(LED_BUILTIN, HIGH);
  }

  void onDisconnect(BLEServer* pServer) {
    Serial.println("Disconnected device!");
    sendValues = false;
    digitalWrite(LED_BUILTIN, LOW);
  };
};

class MySecurity : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() {
    vTaskDelay(25000);
    Serial.print("The passkey request");
    Serial.println(passKey);

    return passKey;
  }
  void onPassKeyNotify(uint32_t pass_key) {
    Serial.print("The passkey Notify number");
    Serial.println(pass_key);
    passKey = pass_key;
  }
  bool onSecurityRequest() {
    return true;
  }
  void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl) {
    if (auth_cmpl.success) {
      Serial.println("Authentication success");
    } else {
      Serial.println("Authentication Failure");
    }
  }

  virtual bool onConfirmPIN(uint32_t pin) {
    return true;
  }
};

class MyCharacteristics : public BLECharacteristicCallbacks {
public:
  MyCharacteristics() {
  }

  void onRead(BLECharacteristic* pCharacteristic) {
    Serial.println("Characteristic Read Request");
  }

  void onWrite(BLECharacteristic* pCharacteristic) {
    Serial.println("Characteristic Write Request");
  }
};

ble_device::ble_device() {
  ble_device::setupGamepadBLE();
  //start();
  ble_device::run();
}

void ble_device::run() {
  while (true) {
    ble_device::toggleAButton();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}


ble_device* ble_device::CreateGamepad(void) {
  //esp_log_level_set("*", ESP_LOG_DEBUG);
  ble_device* pDevice = new ble_device();
  return pDevice;
}

void ble_device::setupGamepadBLE() {

  BLEDevice::init("MotusVR_BLE_3c:71:bf:fd:49:a6");
  BLEAddress addr = BLEDevice::getAddress();
  Serial.println(addr.toString().c_str());

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_NO_MITM);
  BLEDevice::setSecurityCallbacks(new MySecurity());

  hid = new BLEHIDDevice(pServer);
  Serial.println("HID created");

  pInput = hid->inputReport(1);
  pOutput = hid->outputReport(1);

  hid->manufacturer();
  hid->manufacturer(manufacturerName);

  hid->pnp(0x1, 0x2e5, 0xabcd, 0x0110);
  hid->hidInfo(0x00, 0x01);

  hid->reportMap((uint8_t*)reportMapGamepad, sizeof(reportMapGamepad));
  hid->startServices();

  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAppearance(HID_GAMEPAD);  // WALKING DEVICE 0x440;
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);

  pAdvertising->start();

  //https://www.esp32.com/viewtopic.php?t=17230 - look at  ble se urity for pc
  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined!");
}

void ble_device::setupExampleCode() {
  BLEDevice::init("Long name works now");
  BLEServer* pServer = BLEDevice::createServer();

  pServer->setCallbacks(new MyCallbacks());
  BLEDevice::setSecurityCallbacks(new MySecurity());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);

  pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED);

  pCharacteristic->setCallbacks(new MyCharacteristics());
  pCharacteristic->setValue("Hello World says Neil");

  pCharacteristic->addDescriptor(new BLE2902());

  //BLESecurity *pSecurity = new BLESecurity();
  //pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
  //pSecurity->setCapability(ESP_IO_CAP_NONE);
  //pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  pService->start();
  //BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);

  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void ble_device::toggleAButton() {
  //function to test gamepad on android & PC - use 3v to onto D23 to toggle A press on
  if (sendValues) {
    Serial.println("Send Values = true!");
    if (digitalRead(23) == HIGH) {
      Serial.println("PIN HIGH - SENDING!");
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

void ble_device::analogStockMovement() {
  // Simulate analog stick movement
  xAxis = (xAxis + 1) % 256;
  yAxis = (yAxis + 1) % 256;
  uint8_t gamepadReport[] = { 0, 0, xAxis, yAxis };
  pInput->setValue(gamepadReport, sizeof(gamepadReport));
  pInput->notify();
}

void ble_device::setupBatteryService() {
  batteryService = pServer->createService(BATTERY_SERVICE_UUID);
  batteryLevelCharacteristic = batteryService->createCharacteristic(BATTERY_LEVEL_UUID, BLECharacteristic::PROPERTY_READ);
  uint8_t initialBatteryLevel = 50;
  batteryLevelCharacteristic->setValue(&initialBatteryLevel, 1);
  batteryService->start();
  pAdvertising->addServiceUUID(batteryService->getUUID());
}