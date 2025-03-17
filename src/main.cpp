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
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-abcd-1234-abcd-12345678abcd"

// BLE objects
BLEServer *bleServer = nullptr;
BLECharacteristic *reactionCharacteristic = nullptr;
BLEAdvertising *bleAdvertising = nullptr;
bool deviceConnected = false;

// Haptic motor object
DRV2605 haptic;

// Global variable for tracking incoming commands
String receivedCommand = "";

class ReactionGameCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        std::string rxValue = pCharacteristic->getValue();
        if (!rxValue.empty()) {
            receivedCommand = String(rxValue.c_str());
            Serial.print("Received command: ");
            Serial.println(receivedCommand);
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

    BLEService *reactionService = bleServer->createService(SERVICE_UUID);

    reactionCharacteristic = reactionService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

    reactionCharacteristic->setCallbacks(new ReactionGameCallbacks());
    reactionService->start();

    bleAdvertising = bleServer->getAdvertising();
    bleAdvertising->addServiceUUID(SERVICE_UUID);
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

void playHapticFeedback(int buzzCount) {
    for (int i = 0; i < buzzCount; i++) {
        haptic.drv2605_Play_Waveform(10);
        delay(500);
    }
}

void waitForTouch(int touchPin) {
    while (digitalRead(touchPin) == LOW) {}
}

void measureReactionTime(int touchPin, int buzzCount, const char* fingerName) {
    Serial.print("Starting ");
    Serial.print(fingerName);
    Serial.println(" test...");

    playHapticFeedback(buzzCount);
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

void loop() {
    if (receivedCommand == "index") {
        measureReactionTime(INDEX_TOUCH_SENSOR_PIN, 1, "Index");
    } else if (receivedCommand == "middle") {
        measureReactionTime(MIDDLE_TOUCH_SENSOR_PIN, 2, "Middle");
    } else if (receivedCommand == "ring") {
        measureReactionTime(RING_TOUCH_SENSOR_PIN, 3, "Ring");
    } else if (receivedCommand == "pinky") {
        measureReactionTime(PINKY_TOUCH_SENSOR_PIN, 4, "Pinky");
    }
    receivedCommand = "";
}

// #include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>

// BLEServer* pServer = NULL;
// BLEService* pService = NULL;
// BLEAdvertising* pAdvertising = NULL;

// void setup() {
//     Serial.begin(115200);
//     delay(1000);
//     while (!Serial); // Wait for Serial Monitor to open (useful for debugging)
//     Serial.println("Starting BLE Setup...");

//     BLEDevice::init("ESP32C3_Test");
//     Serial.println("BLE Device Initialized");

//     pServer = BLEDevice::createServer();
//     if (pServer) {
//         Serial.println("BLE Server Created Successfully");
//     } else {
//         Serial.println("Failed to Create BLE Server");
//         return;
//     }

//     pService = pServer->createService(BLEUUID((uint16_t)0x180F));
//     if (pService) {
//         Serial.println("BLE Service Created Successfully");
//     } else {
//         Serial.println("Failed to Create BLE Service");
//         return;
//     }

//     pService->start();
//     Serial.println("BLE Service Started");

//     pAdvertising = BLEDevice::getAdvertising();
//     if (pAdvertising) {
//         Serial.println("BLE Advertising Object Created");
//     } else {
//         Serial.println("Failed to Create BLE Advertising Object");
//         return;
//     }

//     pAdvertising->addServiceUUID(BLEUUID((uint16_t)0x180F));
//     Serial.println("Service UUID Added to Advertising");

//     pAdvertising->setScanResponse(true);
//     Serial.println("Scan Response Set");

//     pAdvertising->setMinPreferred(0x06);
//     pAdvertising->setMinPreferred(0x12);
//     Serial.println("Advertising Parameters Set");

//     pAdvertising->start();
//     Serial.println("BLE Advertising Started Successfully");
// }

// void loop() {
//     Serial.println("BLE is Advertising... Waiting for connections...");
//     delay(2000);
// }
