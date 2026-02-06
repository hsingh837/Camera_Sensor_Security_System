#include <Arduino.h>

/*
  integrity_node – main.cpp (skeleton)

  Purpose:
  - Non-blocking event loop
  - Stubbed sensor reads (no wiring yet)
  - Structured event output over Serial
*/

// -----------------------------
// Timing configuration
// -----------------------------
const unsigned long SENSOR_POLL_MS = 100;   // poll sensors at 10 Hz
const unsigned long STATUS_PRINT_MS = 1000; // heartbeat

unsigned long lastSensorPoll = 0;
unsigned long lastStatusPrint = 0;

// -----------------------------
// Event state
// -----------------------------
bool obstructionDetected = false;
bool movementDetected    = false;
bool vibrationDetected   = false;
bool interactionDetected = false;

// -----------------------------
// Stubbed sensor functions
// (replace later with real IO)
// -----------------------------
bool readObstructionSensor() {
  return false;
}

bool readMovementSensor() {
  return false;
}

bool readVibrationSensor() {
  return false;
}

bool readInteractionInput() {
  return false;
}

// -----------------------------
// Event reporting
// -----------------------------
void emitEvent(const char* eventName, bool state) {
  Serial.print("[EVENT] ");
  Serial.print(eventName);
  Serial.print(" = ");
  Serial.println(state ? "TRUE" : "FALSE");
}

// -----------------------------
// Setup
// -----------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("=== integrity_node starting ===");
  Serial.println("Mode: software-only (no wiring)");
}

// -----------------------------
// Main loop
// -----------------------------
void loop() {
  unsigned long now = millis();

  // ---- Sensor polling ----
  if (now - lastSensorPoll >= SENSOR_POLL_MS) {
    lastSensorPoll = now;

    bool newObstruction = readObstructionSensor();
    bool newMovement    = readMovementSensor();
    bool newVibration   = readVibrationSensor();
    bool newInteraction = readInteractionInput();

    if (newObstruction != obstructionDetected) {
      obstructionDetected = newObstruction;
      emitEvent("OBSTRUCTION", obstructionDetected);
    }

    if (newMovement != movementDetected) {
      movementDetected = newMovement;
      emitEvent("MOVEMENT", movementDetected);
    }

    if (newVibration != vibrationDetected) {
      vibrationDetected = newVibration;
      emitEvent("VIBRATION", vibrationDetected);
    }

    if (newInteraction != interactionDetected) {
      interactionDetected = newInteraction;
      emitEvent("INTERACTION", interactionDetected);
    }
  }

  // ---- Heartbeat / status ----
  if (now - lastStatusPrint >= STATUS_PRINT_MS) {
    lastStatusPrint = now;
    Serial.println("[STATUS] integrity_node alive");
  }
}
