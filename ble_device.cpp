#include "ble_device.h"

#define LED_BUILTIN 2

//uses - https://github.com/espressif/arduino-esp32
//https://github.com/T-vK/ESP32-BLE-Keyboard/blob/master/BleKeyboard.cpp

// See the following for generating UUIDs - https://www.uuidgenerator.net/
const char* CUSTOM_SERVICE_UUID="9c9d4aa7-8050-4c8a-bc67-146a66d0443e";
const char*PRESET_CHARACTERISTIC_UUID="9c9d4aa7-8051-4c8a-bc67-146a66d0443e";
const char*VOLUME_CHARACTERISTIC_UUID="9c9d4aa7-8052-4c8a-bc67-146a66d0443e";



static BLEHIDDevice* hid;
static BLECharacteristic* pInput=NULL;
static BLECharacteristic* pOutput=NULL;
static BLEService* pCustomPresetService=NULL;
static BLECharacteristic* pCustomPresetCharacteristic=NULL;
static BLECharacteristic* pCustomVolumeCharacteristic=NULL;
static BLEServer * pServer=NULL;

uint8_t gamepadReport[] = {0, 0, 0, 0};

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
  0x01, 0x05,  // USAGE (Gamepad)
  0x09, 0x01,  // COLLECTION (Application)
  0x01,0x01, //   USAGE (Pointer)
  0xa1,0x00, //   COLLECTION (Physical)
  0x85, 0x01,  //   REPORT_ID (1)

  // Buttons (1 to 16)
  0x09, 0x09,  //   USAGE_PAGE (Button)
  0x19, 0x01,  //   USAGE_MINIMUM (Button 1)
  0x29, 0x08,  //   USAGE_MAXIMUM (Button 16)
  0x15, 0x00,  //   LOGICAL_MINIMUM (0)
  0x29, 0x08,  //   LOGICAL_MAXIMUM (1)
  0x75,0x01, //   REPORT_SIZE (1)
  0x95, 0x08,  //   REPORT_COUNT (16)
  0x81, 0x02,  //   INPUT (Data,Var,Abs)

 // ------------------------------------------------- Padding
  // REPORT_SIZE(1),      0x01, //     REPORT_SIZE (1)
  // REPORT_COUNT(1),     0x08, //     REPORT_COUNT (8)
  // INPUT(1),         0x03, //     INPUT (Constant, Variable, Absolute) ;8 bit padding
  // ------------------------------------------------- X/Y position, Z/rZ position
  
  0x05,0x01, //     USAGE_PAGE (Generic Desktop)
  0x30,0x30, //     USAGE (X)
  0x31,0x31, //     USAGE (Y)
  0x32,0x32, //     USAGE (Z)
  0x35,0x35, //     USAGE (rZ)
  0x33,0x33, //     USAGE (rX) Left Trigger
  0x34,0x34, //     USAGE (rY) Right Trigger
  0x15,0x0, //     LOGICAL_MINIMUM (0)
  0x26,0xff,0x00, //     LOGICAL_MAXIMUM (255)
  0x75,0x08, //     REPORT_SIZE (8)
  0x95,0x06, //     REPORT_COUNT (6)
  0x81,0x02, //     INPUT (Data, Variable, Absolute) ;6 bytes (X,Y,Z,rZ,lt,rt)
  
  0xc0,  // END_COLLECTION

  // output report
  0x06,0x00,0xff, // vendor specific code
  0x09, 0x21, // usage Output Report Data
  0x15,0x00,
  0x26,  0x7f,
  0x75,0x08,
  0x95,0x01,
  0x91,0x02, // data,var,abs

  0xc0 //     END_COLLECTION (application)

};

class MyCallbacks : public BLEServerCallbacks {
public:
	MyCallbacks(ble_device* pad)
	{
		mPad=pad;
	}
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
  ble_device* mPad;
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
  MyCharacteristics(ble_device* pad) 
  {
    mPad=pad;
  }

  void onRead(BLECharacteristic* pCharacteristic) {
    if(pCharacteristic==pCustomVolumeCharacteristic){
        Serial.println("Characteristic Read Request - volume");
    }
    if(pCharacteristic==pCustomPresetCharacteristic)
		{
      Serial.println("Read Request - preset");

    }else{
      Serial.println("Characteristic Read Request - unknown characteristic");
      // Get and print the value for the unknown characteristic
    }
  }

  void onWrite(BLECharacteristic* pCharacteristic) {
    Serial.println("Characteristic Write Request");
  }
  	ble_device* mPad;
};

ble_device::ble_device() {
  ble_device::setupGamepadBLE();
  //start();
  ble_device::run();
}

void ble_device::run() {
  
  while (true) {
    toggleAButton();
    delay(100);
  }
}


ble_device* ble_device::CreateGamepad(void) {
  ble_device* pDevice = new ble_device();
  return pDevice;
}

void ble_device::setupGamepadBLE() {

  BLEDevice::init("MotusVR");
  BLEAddress addr = BLEDevice::getAddress();
  Serial.println(addr.toString().c_str());
  //BLEDevice::setDeviceName(std::string("MotusVR:")+addr.toString().substr(9));	

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks(this));
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_NO_MITM);
  BLEDevice::setSecurityCallbacks(new MySecurity());

  hid = new BLEHIDDevice(pServer);
  Serial.println("HID created");

  pInput = hid->inputReport(1);
  pOutput = hid->outputReport(1);
  MyCharacteristics* changeHandler=new MyCharacteristics(this);
  pInput->setCallbacks(changeHandler);
	pOutput->setCallbacks(changeHandler);

  hid->manufacturer();
  hid->manufacturer(manufacturerName);
  
  pCustomPresetService = pServer->createService(CUSTOM_SERVICE_UUID);
  pCustomPresetCharacteristic=pCustomPresetService->createCharacteristic(PRESET_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_READ);
	pCustomVolumeCharacteristic=pCustomPresetService->createCharacteristic(VOLUME_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE_NR | BLECharacteristic::PROPERTY_READ);
  pCustomPresetService->start();

  hid->pnp(0x1, 0x2e5, 0xabcd, 0x0110);
  hid->hidInfo(0x00, 0x01);
  
  hid->reportMap((uint8_t*)reportMapGamepad, sizeof(reportMapGamepad));
  hid->startServices();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAppearance(HID_GAMEPAD);  // WALKING DEVICE 0x440;
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();

  uint8_t batteryLevel = 50; // 50% battery level
	hid->setBatteryLevel(batteryLevel);

  //https://www.esp32.com/viewtopic.php?t=17230 - look at  ble se urity for pc
  BLESecurity* pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
  pSecurity->setCapability(ESP_IO_CAP_NONE);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined!");
}

void ble_device::toggleAButton() {
  //function to test gamepad on android & PC - use 3v to onto D23 to toggle A press on
  if (sendValues) {
    Serial.println("Sending Data");
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
  /*
  batteryService = pServer->createService(BATTERY_SERVICE_UUID);
  batteryLevelCharacteristic = batteryService->createCharacteristic(BATTERY_LEVEL_UUID, BLECharacteristic::PROPERTY_READ);
  uint8_t initialBatteryLevel = 50;
  batteryLevelCharacteristic->setValue(&initialBatteryLevel, 1);
  batteryService->start();
  pAdvertising->addServiceUUID(batteryService->getUUID());
  */
}