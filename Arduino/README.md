# Arduino Subsystem – Sensor & Control Nodes

This folder contains all **microcontroller-level firmware** for the security system project.

The Arduino subsystem serves as the **physical sensing and immediate response layer** of the system. It interfaces directly with sensors and actuators, performs lightweight signal processing, and generates **structured, time-bounded events** that are consumed by PC-side software (OpenCV-based vision modules and MATLAB analysis).

The architecture is intentionally **modular and forward-looking**. Some nodes provide fully implemented functionality today, while others establish infrastructure for capabilities that will be added incrementally as additional hardware and complexity are introduced.

---

## System Role

At a system level, the Arduino layer answers questions such as:

* What physical or environmental events are occurring right now?
* Has the system been moved, obstructed, or tampered with?
* Are there thermal, audio, or hazard indicators present?
* Should the system trigger an immediate physical response?
* How should events be timestamped and transmitted upstream?

The Arduino layer does **not** perform heavy data analysis, long-term storage, or high-level decision-making. Those responsibilities are intentionally delegated to PC-side software.

---

## High-Level Architecture

```
[ Sensors / Actuators ]
           |
           v
+----------------------+
|   Arduino Nodes      |
|  (Mega R3 / Uno R4)  |
+----------------------+
           |
           |  Serial / Wi-Fi / Bluetooth
           v
+----------------------+
|   PC (OpenCV)        |
+----------------------+
           |
           v
+----------------------+
|   MATLAB Analysis    |
|   & Alerts           |
+----------------------+
```

Each Arduino node:

* Operates independently
* Uses non-blocking logic
* Generates events rather than raw streams
* Can evolve without impacting other nodes

---

## Folder Structure Overview

```
Arduino/
├─ common/
├─ thermal_node/
├─ audio_node/
├─ integrity_node/
├─ auth_node/
├─ hazard_node/
├─ output_node/
├─ comms_node/
├─ calibration/
└─ README.md
```

Each node folder represents a **standalone PlatformIO project**, unless otherwise noted.

---

## Common Utilities (`common/`)

### Purpose

The `common` folder contains **shared definitions and helpers** used across multiple nodes to ensure consistency and avoid duplicated logic.

### Current contents

```
common/
├─ include/
│  ├─ sensors.h
│  ├─ timing.h
│  ├─ thresholds.h
│  ├─ event_codes.h
│  ├─ node_ids.h
│  └─ system_state.h
└─ src/
   ├─ common.cpp
   ├─ event_codes.cpp
   └─ system_state.cpp
```

### Current responsibilities

* Shared sensor abstractions
* Non-blocking timing utilities
* Centralized tunable thresholds
* Standardized event identifiers
* High-level system state tracking

### Planned evolution

This folder is expected to grow to include:

* Message serialization formats
* Versioned event schemas
* Cross-node heartbeat and health monitoring

---

## Thermal Node (`thermal_node/`)

### Purpose

Monitors **thermal activity** using two independent mechanisms:

* Infrared thermal body detection
* Rapid ambient temperature change detection

### Hardware used

* AMG883 infrared thermal sensor
* Temperature/humidity sensors (DHT11-class)

### Structure

```
thermal_node/
├─ platformio.ini
├─ src/
│  ├─ main.cpp
│  ├─ ir_tracking.cpp
│  ├─ temp_monitor.cpp
│  └─ fusion.cpp
└─ include/
   ├─ ir_tracking.h
   ├─ temp_monitor.h
   └─ fusion.h
```

### Current responsibilities

* Detect heat-emitting bodies (coarse resolution)
* Detect temperature changes exceeding defined thresholds
* Fuse thermal indicators into structured events

### Design notes

The AMG883 provides low-resolution (8×8) thermal data and is used for **presence detection**, not detailed imaging.

### Planned evolution

* Calibration routines
* Confidence scoring
* Correlation with motion and audio data

---

## Audio Node (`audio_node/`)

### Purpose

Detects **audio activity** and characterizes sound presence over time.

### Hardware used

* Multiple basic microphone modules

### Structure

```
audio_node/
├─ platformio.ini
├─ src/
│  ├─ main.cpp
│  ├─ mic_array.cpp
│  ├─ db_analysis.cpp
│  ├─ speech_detect.cpp
│  └─ recorder.cpp
└─ include/
   ├─ mic_array.h
   ├─ db_analysis.h
   ├─ speech_detect.h
   └─ recorder.h
```

### Current responsibilities

* Sample microphone inputs
* Estimate sound level
* Detect sustained audio activity
* Generate audio-presence events

### Planned (future) capabilities

This node is **designed** to later support:

* Audio recording to file
* Voice capture
* Metadata tagging of audio clips

These features will be implemented once external storage and appropriate audio hardware are added.

---

## Integrity Node (`integrity_node/`)

### Purpose

Monitors the **physical integrity, placement, and tampering state** of the system.

### Hardware used

* Ultrasonic sensor
* Laser ranging module
* MPU6050 / GY-521 IMU
* Vibration sensor
* Tilt switch
* IR obstacle avoidance sensor

### Structure

```
integrity_node/
├─ platformio.ini
├─ src/
│  ├─ main.cpp
│  ├─ obstruction.cpp
│  ├─ vibration.cpp
│  └─ imu_monitor.cpp
└─ include/
   ├─ obstruction.h
   ├─ vibration.h
   └─ imu_monitor.h
```

### Current responsibilities

* Detect obstruction or close proximity
* Detect movement, tilt, or repositioning
* Detect shock or vibration

### Planned evolution

* Severity classification
* Correlation with other sensor events
* Integration with local authorization inputs

---

## Authorization Node (`auth_node/`)

### Purpose

Handles **local user interaction and authorization infrastructure**.

### Hardware used

* Buttons
* RFID module
* Keypad module

### Structure

```
auth_node/
├─ platformio.ini
├─ src/
│  ├─ main.cpp
│  ├─ rfid.cpp
│  ├─ keypad.cpp
│  └─ control_button.cpp
└─ include/
   ├─ rfid.h
   ├─ keypad.h
   └─ control_button.h
```

### Current responsibilities

* Detect physical user interaction
* Capture RFID and keypad inputs
* Generate interaction events

### Planned (future) capabilities

This node is designed to later support:

* Full authorization logic
* User roles and permissions
* Arm/disarm policy enforcement
* PC-side authentication integration

---

## Hazard Node (`hazard_node/`)

### Purpose

Detects **environmental and safety hazards** unrelated to intrusion.

### Hardware used

* MQ-2 gas/smoke sensor
* Flame sensor
* Water level / raindrop sensor
* Atmospheric pressure sensor

### Structure

```
hazard_node/
├─ platformio.ini
├─ src/
│  ├─ main.cpp
│  ├─ gas_smoke.cpp
│  ├─ flame.cpp
│  └─ water.cpp
└─ include/
   ├─ gas_smoke.h
   ├─ flame.h
   └─ water.h
```

### Current responsibilities

* Detect gas or smoke
* Detect open flame
* Detect water exposure

### Planned evolution

* Hazard severity classification
* Automatic escalation logic
* Trend-based detection

---

## Output Node (`output_node/`)

### Purpose

Executes **physical responses and system feedback**.

### Hardware used

* Relays
* Buzzers
* LEDs / RGB LEDs / traffic light module
* LCD / OLED displays
* Motors and servos (limited use)

### Structure

```
output_node/
├─ platformio.ini
├─ src/
│  ├─ main.cpp
│  ├─ relay.cpp
│  ├─ buzzer.cpp
│  └─ status_led.cpp
└─ include/
   ├─ relay.h
   ├─ buzzer.h
   └─ status_led.h
```

### Current responsibilities

* Audible alerts
* Visual status indicators
* Relay-based actuation

### Planned evolution

* Alert prioritization
* Configurable response policies
* Coordinated multi-output behavior

---

## Communications Node (`comms_node/`)

### Purpose

Manages **timekeeping and external communication**.

### Hardware used

* ESP8266 Wi-Fi module
* Bluetooth module
* DS1307 / DS1302 RTC modules

### Structure

```
comms_node/
├─ platformio.ini
├─ src/
│  ├─ main.cpp
│  ├─ wifi.cpp
│  ├─ bluetooth.cpp
│  └─ rtc.cpp
└─ include/
   ├─ wifi.h
   ├─ bluetooth.h
   └─ rtc.h
```

### Current responsibilities

* Maintain accurate timestamps
* Provide communication channels
* Forward events to PC-side systems

### Planned evolution

* Message buffering and retries
* Unified event transport format
* Tight integration with MATLAB alerting

---

## Calibration (`calibration/`)

Contains **temporary firmware projects** used to calibrate and validate sensors during setup.

These projects are not part of the deployed system and will evolve freely as hardware is tested.

---

## Closing Notes

