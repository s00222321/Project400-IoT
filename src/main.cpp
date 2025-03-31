#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <drv2605.h>

// Pin definitions
#define PINKY_TOUCH_SENSOR_PIN 2
#define RING_TOUCH_SENSOR_PIN 3
#define MIDDLE_TOUCH_SENSOR_PIN 4
#define INDEX_TOUCH_SENSOR_PIN 10

// BLE service and characteristic UUIDs
#define DEVICE_NAME "Project400"
#define REACTION_SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define REACTION_CHARACTERISTIC_UUID "abcd1234-abcd-1234-abcd-12345678abcd"
#define HAPTIC_SERVICE_UUID "87654321-4321-4321-4321-abcdefabcdef"
#define HAPTIC_CHARACTERISTIC_UUID "dcba4321-dcba-4321-dcba-abcdefabcdef"

// BLE objects
BLEServer *bleServer = nullptr;
BLECharacteristic *reactionCharacteristic = nullptr;
BLECharacteristic *hapticCharacteristic = nullptr;
BLEAdvertising *bleAdvertising = nullptr;
bool deviceConnected = false;

// Haptic motor object
DRV2605 haptic;

// Global variable for tracking incoming commands
String receivedCommand = "";

void playHapticFeedback(int buzzCount, int intensity) {
    for (int i = 0; i < buzzCount; i++) {
        haptic.drv2605_Play_Waveform(intensity);
        delay(500);
    }
}

void waitForTouch(int touchPin) {
    while (digitalRead(touchPin) == LOW) {}
}

void measureReactionTime(int touchPin, int buzzCount, const char* fingerName, int hapticIntensity) {
    Serial.print("Starting ");
    Serial.print(fingerName);
    Serial.println(" test...");

    playHapticFeedback(buzzCount, hapticIntensity);
    Serial.println("Buzz! React now!");

    unsigned long startTime = millis();
    waitForTouch(touchPin);
    unsigned long reactionTime = millis() - startTime;

    Serial.print(fingerName);
    Serial.print(" Reaction Time: ");
    Serial.print(reactionTime);
    Serial.println(" ms");

    String reactionTimeStr = String(fingerName) + " " + String(reactionTime);
    reactionCharacteristic->setValue(reactionTimeStr.c_str());
    reactionCharacteristic->notify();
    Serial.println("Reaction time sent over BLE");
    delay(2000);
}

class ReactionGameCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        std::string rxValue = pCharacteristic->getValue();
        if (!rxValue.empty()) {
            String command = String(rxValue.c_str());
            Serial.print("Received command: ");
            Serial.println(command);
            
            // Call measureReactionTime directly
            if (command == "index") {
                measureReactionTime(INDEX_TOUCH_SENSOR_PIN, 1, "Index", 1);
            } else if (command == "middle") {
                measureReactionTime(MIDDLE_TOUCH_SENSOR_PIN, 2, "Middle", 1);
            } else if (command == "ring") {
                measureReactionTime(RING_TOUCH_SENSOR_PIN, 3, "Ring", 1);
            } else if (command == "pinky") {
                measureReactionTime(PINKY_TOUCH_SENSOR_PIN, 4, "Pinky", 1);
            }
        }
    }
};


void playHapticFeedback(int buzzCount, int intensity);

class HapticControlCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        std::string rxValue = pCharacteristic->getValue();
        if (!rxValue.empty()) {
            int intensity = atoi(rxValue.c_str());
            playHapticFeedback(3, 20-intensity);
        }
    }
};

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("Device connected");
    }

    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("Device disconnected, restarting advertising...");
        delay(500);
        bleAdvertising->start();
    }
};

void setupBLE() {
    BLEDevice::init(DEVICE_NAME);
    bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new ServerCallbacks());

    // Reaction Time Service
    BLEService *reactionService = bleServer->createService(REACTION_SERVICE_UUID);
    reactionCharacteristic = reactionService->createCharacteristic(
        REACTION_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
    reactionCharacteristic->setCallbacks(new ReactionGameCallbacks());
    reactionService->start();

    // Haptic Control Service
    BLEService *hapticService = bleServer->createService(HAPTIC_SERVICE_UUID);
    hapticCharacteristic = hapticService->createCharacteristic(
        HAPTIC_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    hapticCharacteristic->setCallbacks(new HapticControlCallbacks());
    hapticService->start();

    bleAdvertising = bleServer->getAdvertising();
    bleAdvertising->addServiceUUID(REACTION_SERVICE_UUID);
    bleAdvertising->addServiceUUID(HAPTIC_SERVICE_UUID);
    bleAdvertising->setScanResponse(true);
    bleAdvertising->start();

    Serial.println("BLE setup complete. Waiting for connection...");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    Serial.println("Reaction Game with BLE Started");

    pinMode(INDEX_TOUCH_SENSOR_PIN, INPUT);
    pinMode(RING_TOUCH_SENSOR_PIN, INPUT);
    pinMode(MIDDLE_TOUCH_SENSOR_PIN, INPUT);
    pinMode(PINKY_TOUCH_SENSOR_PIN, INPUT);

    if (haptic.init(false, true) != 0) {
        Serial.println("Haptic motor initialization failed!");
    }
    if (haptic.drv2605_AutoCal() != 0) {
        Serial.println("Haptic motor auto calibration failed!");
    }

    setupBLE();
}

void loop() {
    // Main loop does nothing, all actions are handled in callbacks
    delay(1000); // Just to avoid busy-waiting
}

// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>
// #include <drv2605.h>

// // Pin definitions
// #define TOUCH_SENSOR_PIN 2  // Grove port for the touch sensor

// // BLE service and characteristic UUIDs
// #define DEVICE_NAME "Project400"
// #define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
// #define CHARACTERISTIC_UUID "abcd1234-abcd-1234-abcd-12345678abcd"

// // BLE objects
// BLEServer *bleServer = nullptr;
// BLECharacteristic *reactionCharacteristic = nullptr;

// // Haptic motor object
// DRV2605 haptic;

// // Global variables
// bool startGame = false;

// class ReactionGameCallbacks : public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic *pCharacteristic) override {
//     std::string rxValue = pCharacteristic->getValue();
//     if (rxValue == "start") {
//       Serial.println("Received 'start' command from app.");
//       startGame = true;  // Set flag to start game
//     }
//   }
// };

// void setupBLE() {
//   BLEDevice::init(DEVICE_NAME);
//   bleServer = BLEDevice::createServer();

//   BLEService *reactionService = bleServer->createService(SERVICE_UUID);

//   reactionCharacteristic = reactionService->createCharacteristic(
//       CHARACTERISTIC_UUID,
//       BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

//   // Set callback for BLE write events
//   reactionCharacteristic->setCallbacks(new ReactionGameCallbacks());

//   reactionService->start();

//   BLEAdvertising *advertising = bleServer->getAdvertising();
//   advertising->addServiceUUID(SERVICE_UUID);
//   advertising->setScanResponse(true);
//   advertising->start();

//   Serial.println("BLE setup complete. Waiting for connection...");
// }

// void setup() {
//   Serial.begin(115200);
//   while (!Serial) {}

//   Serial.println("Reaction Game with BLE Started");

//   pinMode(TOUCH_SENSOR_PIN, INPUT);

//   if (haptic.init(false, true) != 0) {
//     Serial.println("Haptic motor initialization failed!");
//   }
//   if (haptic.drv2605_AutoCal() != 0) {
//     Serial.println("Haptic motor auto calibration failed!");
//   }

//   setupBLE();
// }

// void loop() {
//   if (startGame) {
//     Serial.println("Starting reaction game...");

//     // Buzz the haptic motor
//     haptic.drv2605_Play_Waveform(2);
//     Serial.println("Buzz! React now!");

//     // Start timing
//     unsigned long startTime = millis();

//     // Wait for touch sensor press
//     while (digitalRead(TOUCH_SENSOR_PIN) == LOW) {}

//     // Calculate reaction time
//     unsigned long reactionTime = millis() - startTime;
//     Serial.print("Reaction Time: ");
//     Serial.print(reactionTime);
//     Serial.println(" ms");

//     // Send reaction time via BLE
//     String reactionTimeStr = String(reactionTime) + " ms";
//     reactionCharacteristic->setValue(reactionTimeStr.c_str());
//     reactionCharacteristic->notify();
//     Serial.println("Reaction time sent over BLE");

//     // Reset the start flag
//     startGame = false;
//     Serial.println("Waiting for next 'start' command.");
//   }
// }