#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEHIDDevice.h>
#include <BLEServer.h>
#include <string>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

/*
To do -
Fix issue with connection on PC
Sort out gamepad movement and range of scale
Battery service
Connection and disconnection - move into classes and BLEServerCallbacks

  BLEService *pBatteryService = hid->batteryService();
  BLECharacteristic *pBatteryLevelCharacteristic = pBatteryService->createCharacteristic(BATTERY_LEVEL_UUID,BLECharacteristic::PROPERTY_READ);
  uint8_t initialBatteryLevel = 50; // Example: 50%
  pBatteryLevelCharacteristic->setValue(&initialBatteryLevel, 1);
  pBatteryService->start();
*/

#define LED_BUILTIN 2
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BATTERY_SERVICE_UUID   "0000180f-0000-1000-8000-00805f9b34fb"
#define BATTERY_LEVEL_UUID     "00002a19-0000-1000-8000-00805f9b34fb"

static BLEHIDDevice* hid;
static BLECharacteristic* pInput=NULL;
static BLECharacteristic* pOutput=NULL;

std::string manufacturerName = "MotusVR"; 
bool sendValues = false;

static uint8_t buttonState = 0;
static uint8_t xAxis = 0;
static uint8_t yAxis = 0;

// Define constants for button indices
const int BUTTON_A_INDEX = 1; // Index of the A button

const uint8_t reportMapGamepad[] = {
  0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
  0x09, 0x05,       // USAGE (Gamepad)
  0xA1, 0x01,       // COLLECTION (Application)
  0x85, 0x01,       //   REPORT_ID (1)

  // Buttons (1 to 16)
  0x05, 0x09,       //   USAGE_PAGE (Button)
  0x19, 0x01,       //   USAGE_MINIMUM (Button 1)
  0x29, 0x10,       //   USAGE_MAXIMUM (Button 16)
  0x15, 0x00,       //   LOGICAL_MINIMUM (0)
  0x25, 0x01,       //   LOGICAL_MAXIMUM (1)
  0x95, 0x10,       //   REPORT_COUNT (16)
  0x75, 0x01,       //   REPORT_SIZE (1)
  0x81, 0x02,       //   INPUT (Data,Var,Abs)

  // Padding to align to the next byte
  0x95, 0x06,       //   REPORT_COUNT (6)
  0x75, 0x02,       //   REPORT_SIZE (2)
  0x81, 0x03,       //   INPUT (Cnst,Var,Abs)

  // Analog Axes (X and Y)
  0x05, 0x01,       //   USAGE_PAGE (Generic Desktop)
  0x09, 0x30,       //   USAGE (X)
  0x09, 0x31,       //   USAGE (Y)
  0x15, 0x81,       //   LOGICAL_MINIMUM (-127)
  0x25, 0x7F,       //   LOGICAL_MAXIMUM (127)
  0x75, 0x08,       //   REPORT_SIZE (8)
  0x95, 0x02,       //   REPORT_COUNT (2)
  0x81, 0x02,       //   INPUT (Data,Var,Abs)

  0xC0              // END_COLLECTION
};

class MyCallbacks : public BLEServerCallbacks {
	public:
	MyCallbacks()
	{

  }
	
  void onConnect(BLEServer* pServer){
    Serial.println("Connected device!");
    sendValues = true;
    digitalWrite(LED_BUILTIN, HIGH); 
  }

  void onDisconnect(BLEServer* pServer){
    Serial.println("Disconnected device!");
    sendValues = false;
    digitalWrite(LED_BUILTIN, LOW);  
  }
  };

void setupGamepadBLE(){
  
  BLEDevice::init("MotusVR_BLE_3c:71:bf:fd:49:a6"); //needs sorting with the address below
  BLEAddress addr=BLEDevice::getAddress();
  Serial.println(addr.toString().c_str());

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_NO_MITM);

  hid = new BLEHIDDevice(pServer);
  Serial.println("HID created");

  pInput=hid->inputReport(1);
	pOutput=hid->outputReport(1);

  hid->pnp(0x1,0x2e5,0xabcd,0x0110);
	hid->hidInfo(0x00,0x01);

  hid->manufacturer();
	hid->manufacturer(manufacturerName);
  
  hid->reportMap((uint8_t*)reportMapGamepad, sizeof(reportMapGamepad));
	hid->startServices();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAppearance(HID_GAMEPAD);// WALKING DEVICE 0x440;
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMaxPreferred(0x12);

  pAdvertising->start();

	BLESecurity *pSecurity = new BLESecurity();
	pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
	pSecurity->setCapability(ESP_IO_CAP_NONE);
	pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined!");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Starting BLE");
  setupGamepadBLE();
}

void toggleAButton(){
  //function to test gamepad on android & PC - use 3v to onto D23 to toggle A press on
  if(sendValues){
    if (digitalRead(23) == HIGH) {
      uint8_t gamepadReport[] = {BUTTON_A_INDEX, 0, 0, 0};
      pInput->setValue(gamepadReport, sizeof(gamepadReport));
      pInput->notify();
    }
    else{
      uint8_t gamepadReport[] = {0, 0, 0, 0};
      pInput->setValue(gamepadReport, sizeof(gamepadReport));
      pInput->notify();
    }
}
}

void analogStockMovement(){
   // Simulate analog stick movement
    xAxis = (xAxis + 1) % 256;
    yAxis = (yAxis + 1) % 256;
    uint8_t gamepadReport[] = {0, 0, xAxis, yAxis};
    pInput->setValue(gamepadReport, sizeof(gamepadReport));
    pInput->notify();
    }

void loop() {
 
}

  
