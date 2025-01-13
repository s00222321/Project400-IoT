// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>

// // Pin definitions
// #define TOUCH_SENSOR_PIN 2  // Grove port for the touch sensor
// #define LED_PIN 3           // Grove port for the LED

// // BLE service and characteristic UUIDs
// #define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
// #define CHARACTERISTIC_UUID "abcd1234-abcd-1234-abcd-12345678abcd"

// // BLE objects
// BLEServer *bleServer = nullptr;
// BLECharacteristic *reactionCharacteristic = nullptr;

// void setupBLE() {
//   // Initialize BLE
//   BLEDevice::init("Project400");
//   bleServer = BLEDevice::createServer();

//   // Create a BLE service
//   BLEService *reactionService = bleServer->createService(SERVICE_UUID);

//   // Create a BLE characteristic
//   reactionCharacteristic = reactionService->createCharacteristic(
//       CHARACTERISTIC_UUID,
//       BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

//   // Start the service
//   reactionService->start();

//   // Start advertising
//   BLEAdvertising *advertising = bleServer->getAdvertising();
//   advertising->addServiceUUID(SERVICE_UUID);
//   advertising->setScanResponse(true);
//   advertising->setMinPreferred(0x06);  // Minimum advertising interval
//   advertising->setMaxPreferred(0x12); // Maximum advertising interval
//   advertising->start();

//   Serial.println("BLE setup complete. Waiting for connection...");
// }

// void setup() {
//   // Initialize serial monitor
//   Serial.begin(115200);
//   while (!Serial) {
//     ; // Wait for serial connection
//   }
//   Serial.println("Reaction Game with BLE Started");

//   // Set up the pins
//   pinMode(TOUCH_SENSOR_PIN, INPUT);
//   pinMode(LED_PIN, OUTPUT);

//   // Turn off the LED initially
//   digitalWrite(LED_PIN, LOW);

//   // Setup BLE
//   setupBLE();
// }

// void loop() {
//   // Wait for the touch sensor to be pressed (light turns off)
//   if (digitalRead(TOUCH_SENSOR_PIN) == HIGH) {
//     digitalWrite(LED_PIN, LOW); // Turn off the LED
//     Serial.println("LED OFF: Touch detected. Lift your finger to start the game.");
    
//     // Wait for the touch to be released
//     while (digitalRead(TOUCH_SENSOR_PIN) == HIGH) {
//       // Do nothing, wait for the finger to be lifted
//     }
//     Serial.println("Finger lifted. Starting the game...");
//   }

//   // Wait a random delay (simulate anticipation)
//   unsigned long delayTime = random(2000, 5000); // 2 to 5 seconds
//   delay(delayTime);

//   // Turn on the LED to signal the start of the reaction test
//   digitalWrite(LED_PIN, HIGH);
//   Serial.println("LED ON: React now!");

//   // Start timing
//   unsigned long startTime = millis();

//   // Wait for the touch sensor to be pressed (reaction)
//   while (digitalRead(TOUCH_SENSOR_PIN) == LOW) {
//     // Do nothing, wait for the touch
//   }

//   // Calculate reaction time
//   unsigned long reactionTime = millis() - startTime;
//   Serial.print("Reaction Time: ");
//   Serial.print(reactionTime);
//   Serial.println(" ms");

//   // Send reaction time via BLE
//   String reactionTimeStr = String(reactionTime) + " ms";
//   reactionCharacteristic->setValue(reactionTimeStr.c_str());
//   reactionCharacteristic->notify();
//   Serial.println("Reaction time sent over BLE");

//   // Turn off the LED after recording the reaction
//   digitalWrite(LED_PIN, LOW);
//   Serial.println("LED OFF: Wait for the next round.");

//   // Wait for the touch to be released before restarting
//   while (digitalRead(TOUCH_SENSOR_PIN) == HIGH) {
//     // Do nothing, wait for the finger to be lifted
//   }

//   Serial.println("Ready for the next round.\n");
// }

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <drv2605.h>

// Pin definitions
#define TOUCH_SENSOR_PIN 2     // Grove port for the touch sensor

// BLE service and characteristic UUIDs
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-abcd-1234-abcd-12345678abcd"

// BLE objects
BLEServer *bleServer = nullptr;
BLECharacteristic *reactionCharacteristic = nullptr;

// Haptic motor object
DRV2605 haptic;

void setupBLE() {
  // Initialize BLE
  BLEDevice::init("Project400");
  bleServer = BLEDevice::createServer();

  // Create a BLE service
  BLEService *reactionService = bleServer->createService(SERVICE_UUID);

  // Create a BLE characteristic
  reactionCharacteristic = reactionService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  // Start the service
  reactionService->start();

  // Start advertising
  BLEAdvertising *advertising = bleServer->getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);  // Minimum advertising interval
  advertising->setMaxPreferred(0x12); // Maximum advertising interval
  advertising->start();

  Serial.println("BLE setup complete. Waiting for connection...");
}

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial connection
  }
  Serial.println("Reaction Game with BLE Started");

  // Set up the pins
  pinMode(TOUCH_SENSOR_PIN, INPUT);

  // Initialize haptic motor
  if (haptic.init(false, true) != 0) {
    Serial.println("Haptic motor initialization failed!");
  }
  if (haptic.drv2605_AutoCal() != 0) {
    Serial.println("Haptic motor auto calibration failed!");
  }

  // Setup BLE
  setupBLE();
}

void loop() {
  // Wait for the touch sensor to be pressed
  if (digitalRead(TOUCH_SENSOR_PIN) == HIGH) {
    Serial.println("Touch detected. Lift your finger to start the game.");

    // Wait for the touch to be released
    while (digitalRead(TOUCH_SENSOR_PIN) == HIGH) {
      // Do nothing, wait for the finger to be lifted
    }
    Serial.println("Finger lifted. Starting the game...");
  }

  // Wait a random delay (simulate anticipation)
  unsigned long delayTime = random(2000, 5000); // 2 to 5 seconds
  delay(delayTime);

  // Buzz the haptic motor once
  haptic.drv2605_Play_Waveform(118); // Play a haptic effect (effect 118)
  Serial.println("Buzz! React now!");

  // Start timing
  unsigned long startTime = millis();

  // Wait for the touch sensor to be pressed (reaction)
  while (digitalRead(TOUCH_SENSOR_PIN) == LOW) {
    // Do nothing, wait for the touch
  }

  // Calculate reaction time
  unsigned long reactionTime = millis() - startTime;
  Serial.print("Reaction Time: ");
  Serial.print(reactionTime);
  Serial.println(" ms");

  // Send reaction time via BLE
  String reactionTimeStr = String(reactionTime) + " ms";
  reactionCharacteristic->setValue(reactionTimeStr.c_str());
  reactionCharacteristic->notify();
  Serial.println("Reaction time sent over BLE");

  // Wait for the touch to be released before restarting
  while (digitalRead(TOUCH_SENSOR_PIN) == HIGH) {
    // Do nothing, wait for the finger to be lifted
  }

  Serial.println("Ready for the next round.\n");
}
