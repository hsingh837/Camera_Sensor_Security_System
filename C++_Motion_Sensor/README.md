## C++ OpenCV Motion Sensor – PC Vision Node

This project implements the **primary PC-side motion detection and video recording system** for the security platform. It uses OpenCV and C++ to perform real-time camera monitoring, detect significant motion events, record video clips, and log structured motion data for downstream analysis.

This module serves as the **vision intelligence layer**, complementing Arduino-based physical sensors and feeding motion events into higher-level analysis pipelines.

---

## System Role

At a system level, this project answers:

* Is there **significant visual motion** in the camera’s field of view?
* When did motion begin and end?
* What video evidence corresponds to detected motion?
* How should motion events be logged for later correlation?

This module focuses on **camera-based perception**, not physical sensor readings.

---

## High-Level Architecture

```
[ Webcam / USB Camera ]
           |
           v
+---------------------------+
|  C++ OpenCV Motion Node   |
|  - Frame differencing     |
|  - Thresholding           |
|  - Video recording        |
+---------------------------+
           |
           | CSV + Video Files
           v
+---------------------------+
|  MATLAB / Analysis Layer  |
+---------------------------+
```

---

## Folder Structure

```
C++OCV Motion Sensor/
├─ build/
├─ Output Data/
│  ├─ Data1.csv
│  ├─ Data2.csv
│  └─ ...
├─ Output Videos/
│  ├─ Video1.mp4
│  ├─ Video2.mp4
│  └─ ...
├─ src/
│  ├─ main.cpp
│  └─ Recording.cpp
├─ CMakeLists.txt
├─ photoname.jpg
└─ README.md
```

---

## File Breakdown

### `src/main.cpp`

The main application entry point.

**Responsibilities:**

* Initialize the camera
* Control program state (idle, recording, motion detection)
* Capture frames continuously
* Coordinate motion detection logic
* Write per-second motion status to CSV
* Trigger automatic termination after a fixed duration

This file defines the **runtime behavior and control flow** of the motion sensor.

---

### `src/Recording.cpp`

Encapsulates video recording logic.

**Responsibilities:**

* Configure OpenCV `VideoWriter`
* Write frames to disk
* Manage codec, framerate, and output resolution

This separation keeps recording logic isolated from detection logic.

---

### `Output Data/`

Contains **sequential CSV logs** produced by each run.

Each CSV:

* Is uniquely numbered (`Data1.csv`, `Data2.csv`, …)
* Contains one row per second
* Logs whether motion was detected during that second

These files are intended for **offline analysis and correlation**.

---

### `Output Videos/`

Contains **sequentially recorded video files**.

Each video:

* Corresponds to a single execution
* Is synchronized with the CSV log
* Preserves visual evidence of motion events

---

### `CMakeLists.txt`

Defines the build configuration.

**Responsibilities:**

* Locate OpenCV
* Enforce C++17
* Define the executable target
* Control compiler and linker behavior

---

## Current Detection Strategy

The system currently uses:

* Grayscale frame differencing
* Binary thresholding
* Pixel-change ratio evaluation

Motion is considered “significant” when the fraction of changed pixels exceeds a defined threshold within a time window.

---

## Planned Evolution

Future enhancements may include:

* Adaptive background modeling
* Region-of-interest masking
* Confidence scoring for motion events
* Synchronization with Arduino integrity sensors
* Event-level metadata enrichment

This project is intentionally structured to allow these upgrades without architectural changes.

---

## Summary

This C++ OpenCV module provides:

* Real-time motion detection
* Evidence-grade video recording
* Structured, time-aligned CSV logs
* A clean interface to downstream analytics

It acts as the **visual backbone** of the security system.

