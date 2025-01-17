#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <drv2605.h>

// Pin definitions
#define TOUCH_SENSOR_PIN 2  // Grove port for the touch sensor

// BLE service and characteristic UUIDs
#define DEVICE_NAME "Project400"
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-abcd-1234-abcd-12345678abcd"

// BLE objects
BLEServer *bleServer = nullptr;
BLECharacteristic *reactionCharacteristic = nullptr;

// Haptic motor object
DRV2605 haptic;

// Global variables
bool startGame = false;

class ReactionGameCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue == "start") {
      Serial.println("Received 'start' command from app.");
      startGame = true;  // Set flag to start game
    }
  }
};

void setupBLE() {
  BLEDevice::init(DEVICE_NAME);
  bleServer = BLEDevice::createServer();

  BLEService *reactionService = bleServer->createService(SERVICE_UUID);

  reactionCharacteristic = reactionService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  // Set callback for BLE write events
  reactionCharacteristic->setCallbacks(new ReactionGameCallbacks());

  reactionService->start();

  BLEAdvertising *advertising = bleServer->getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->start();

  Serial.println("BLE setup complete. Waiting for connection...");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("Reaction Game with BLE Started");

  pinMode(TOUCH_SENSOR_PIN, INPUT);

  if (haptic.init(false, true) != 0) {
    Serial.println("Haptic motor initialization failed!");
  }
  if (haptic.drv2605_AutoCal() != 0) {
    Serial.println("Haptic motor auto calibration failed!");
  }

  setupBLE();
}

void loop() {
  if (startGame) {
    Serial.println("Starting reaction game...");

    // Buzz the haptic motor
    haptic.drv2605_Play_Waveform(2);
    Serial.println("Buzz! React now!");

    // Start timing
    unsigned long startTime = millis();

    // Wait for touch sensor press
    while (digitalRead(TOUCH_SENSOR_PIN) == LOW) {}

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

    // Reset the start flag
    startGame = false;
    Serial.println("Waiting for next 'start' command.");
  }
}
