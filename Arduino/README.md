# Arduino Subsystem – Sensor & Control Nodes

This folder contains all **microcontroller-level firmware** for the security system project.

The Arduino subsystem is responsible for **direct interaction with the physical world**: sensing, basic signal processing, immediate responses, and structured event generation. Higher-level interpretation, data fusion, and alerting are handled by OpenCV (C++ / Python) and MATLAB.

This directory is organized as a **set of independent nodes**, each with a clearly defined responsibility. The structure is intentionally modular to support **concurrent operation, future expansion, and hardware changes** without requiring large refactors.

---

## System-Level Role

At a high level, the Arduino layer answers questions like:

* What physical events are occurring right now?
* Has the system been moved, obstructed, or tampered with?
* Is there an environmental or safety hazard present?
* Should the system react immediately (alarm, indicator, relay)?
* How should raw sensor events be forwarded upstream?

The Arduino layer does **not** attempt to perform heavy data analysis or long-term logging. Instead, it produces **time-bounded, structured events** that are consumed by PC-side processes.

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
|   & Messaging        |
+----------------------+
```

Each Arduino node is designed to:

* Operate independently
* Avoid blocking delays
* Publish events instead of raw streams
* Remain replaceable or extensible

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

Each node folder is a **standalone PlatformIO project** unless otherwise noted.

---

## Common Utilities (`common/`)

This folder contains **shared definitions and utilities** used across multiple nodes.

### Purpose

The `common` folder exists to prevent duplicated logic and ensure consistent behavior across the Arduino subsystem.

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

### What these files do today

* Define shared sensor interfaces
* Provide non-blocking timing helpers
* Centralize tunable thresholds
* Establish consistent event identifiers
* Track high-level system state

### Planned evolution

In the future, this folder will likely grow to include:

* Standardized message serialization
* Versioned protocol definitions
* Cross-node health and heartbeat logic

---

## Thermal Node (`thermal_node/`)

### Purpose

This node monitors **thermal activity** using two independent mechanisms:

* Infrared thermal body detection
* Rapid ambient temperature change detection

It exists to detect the **presence of heat-emitting bodies** and **abnormal thermal transitions**.

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

* Acquire thermal sensor data
* Detect thermal bodies independently of motion
* Monitor temperature deltas over short windows
* Fuse thermal signals into single events

### Planned evolution

Future revisions will define:

* Sensor placement assumptions
* Calibration routines
* Event confidence scoring
* Timestamp alignment with other nodes

---

## Audio Node (`audio_node/`)

### Purpose

This node handles **acoustic sensing**, including:

* Sound level detection
* Sustained audio classification
* Audio recording when appropriate

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

* Sample multiple microphones
* Compute decibel levels
* Detect sustained audio events
* Manage on-device audio recording

### Planned evolution

This node will eventually define:

* Audio file formats
* Storage limits and rotation
* Noise floor calibration
* Metadata tagging for recorded clips

---

## Integrity Node (`integrity_node/`)

### Purpose

This node monitors the **physical integrity and placement** of the system itself.

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

* Detect obstruction via proximity sensing
* Detect shocks or vibration
* Detect movement, tilt, or repositioning

### Planned evolution

Future work will:

* Define acceptable placement envelopes
* Add tamper severity levels
* Correlate integrity events with other nodes

---

## Authorization Node (`auth_node/`)

### Purpose

This node manages **human interaction and access control**.

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

* Authenticate authorized users
* Accept PIN or card input
* Handle manual overrides

### Planned evolution

This node will later:

* Define authorization levels
* Log access attempts
* Integrate with PC-side authentication logic

---

## Hazard Node (`hazard_node/`)

### Purpose

This node detects **non-intrusion environmental hazards**.

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
* Detect open flames
* Detect water exposure

### Planned evolution

Future development may:

* Classify hazard severity
* Trigger automatic system responses
* Feed hazard data into long-term analysis

---

## Output Node (`output_node/`)

### Purpose

This node executes **physical responses** to events.

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

* Activate relays
* Emit audible alerts
* Display system status

### Planned evolution

This node will eventually:

* Support alert prioritization
* Allow configurable output behavior
* Coordinate responses across nodes

---

## Communications Node (`comms_node/`)

### Purpose

This node manages **timekeeping and outbound communication**.

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

* Maintain accurate time
* Provide communication pathways
* Bridge Arduino data to external systems

### Planned evolution

Future work will define:

* Messaging formats
* Retry and buffering behavior
* Integration with MATLAB alerting pipelines

---

## Calibration (`calibration/`)

This folder contains **temporary, task-specific firmware projects** used during setup and validation.

These projects are not part of the deployed system.

---

## Closing Notes

This Arduino subsystem is intentionally designed to:

* Scale horizontally (more nodes)
* Evolve vertically (more logic per node)
* Tolerate hardware changes
* Integrate cleanly with PC-side software

As the system matures, this README will expand to include:

* Pin mappings
* Event schemas
* Communication protocols
* Timing diagrams

