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
unsigned long lastCommandTime = 0;

void playHapticFeedback(int buzzCount, int intensity)
{
    for (int i = 0; i < buzzCount; i++)
    {
        haptic.drv2605_Play_Waveform(intensity);
        delay(500);
    }
}

void waitForTouch(int touchPin)
{
    while (digitalRead(touchPin) == LOW)
    {
    }
}

void measureReactionTime(int touchPin, int buzzCount, const char *fingerName, int hapticIntensity, String gameMode, String hand)
{
    Serial.print("Starting ");
    Serial.print(fingerName);
    Serial.println(" test...");

    playHapticFeedback(buzzCount, hapticIntensity);
    Serial.println("Buzz! React now!");

    unsigned long startTime = millis();
    bool timeoutOccurred = false;
    
    while (millis() - startTime < 10000) // 10 second timeout
    {
        if (digitalRead(touchPin) == HIGH)
        {
            unsigned long reactionTime = millis() - startTime;

            String reactionTimeStr = String(fingerName) + " " + String(reactionTime) + " " + gameMode.charAt(0) + " " + hand.charAt(0); // length issue so had to shorten gameMode and hand to 1 char
            reactionCharacteristic->setValue(reactionTimeStr.c_str());
            reactionCharacteristic->notify();
            Serial.println("Reaction time sent over BLE");
            return; // Exit function once reaction is registered
        }
    }

    // If no touch detected in 10 seconds, send a cancel message
    reactionCharacteristic->setValue("cancelled");
    reactionCharacteristic->notify();
    Serial.println("Timeout! Cancel message sent over BLE.");
}

class ReactionGameCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        std::string rxValue = pCharacteristic->getValue();
        if (!rxValue.empty())
        {
            String command = String(rxValue.c_str());
            Serial.print("Received command: ");
            Serial.println(command);
            lastCommandTime = millis(); // Reset the timeout timer

            // Split by commas
            int firstComma = command.indexOf(',');
            int secondComma = command.indexOf(',', firstComma + 1);

            String finger = command.substring(0, firstComma);
            String gameMode = command.substring(firstComma + 1, secondComma);
            String hand = command.substring(secondComma + 1);

            // Call measureReactionTime directly
            if (finger == "index")
            {
                measureReactionTime(INDEX_TOUCH_SENSOR_PIN, 1, "Index", 1, gameMode, hand);
            }
            else if (finger == "middle")
            {
                measureReactionTime(MIDDLE_TOUCH_SENSOR_PIN, 2, "Middle", 1, gameMode, hand);
            }
            else if (finger == "ring")
            {
                measureReactionTime(RING_TOUCH_SENSOR_PIN, 3, "Ring", 1, gameMode, hand);
            }
            else if (finger == "pinky")
            {
                measureReactionTime(PINKY_TOUCH_SENSOR_PIN, 4, "Pinky", 1, gameMode, hand);
            }
        }
    }
};

class HapticControlCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        std::string rxValue = pCharacteristic->getValue();
        if (!rxValue.empty())
        {
            int intensity = atoi(rxValue.c_str());
            playHapticFeedback(3, 20 - intensity);
        }
    }
};

class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer) override
    {
        deviceConnected = true;
        Serial.println("Device connected");
    }

    void onDisconnect(BLEServer *pServer) override
    {
        deviceConnected = false;
        Serial.println("Device disconnected, restarting advertising...");
        delay(500);
        bleAdvertising->start();
    }
};

void setupBLE()
{
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

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
    }

    Serial.println("Reaction Game with BLE Started");

    pinMode(INDEX_TOUCH_SENSOR_PIN, INPUT);
    pinMode(RING_TOUCH_SENSOR_PIN, INPUT);
    pinMode(MIDDLE_TOUCH_SENSOR_PIN, INPUT);
    pinMode(PINKY_TOUCH_SENSOR_PIN, INPUT);

    if (haptic.init(false, true) != 0)
    {
        Serial.println("Haptic motor initialization failed!");
    }
    if (haptic.drv2605_AutoCal() != 0)
    {
        Serial.println("Haptic motor auto calibration failed!");
    }

    setupBLE();
}

void loop()
{
    // Main loop does nothing, all actions are handled in callbacks
    delay(1000); // Just to avoid busy-waiting
}
