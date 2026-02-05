## Python OpenCV Light-Level Sensor – Environmental Vision Node

This project implements a **PC-side light-level monitoring system** using Python and OpenCV. It detects rapid or sustained changes in ambient brightness, records video alongside measurements, and logs structured light-level events to CSV.

This module complements motion detection by answering **environmental context questions** that are difficult to infer from motion alone.

---

## System Role

At a system level, this project answers:

* Has the ambient lighting changed significantly?
* Did lighting changes coincide with motion or audio events?
* Was the camera blinded, covered, or exposed to sudden illumination?

It acts as an **environmental awareness layer** for the overall system.

---

## High-Level Architecture

```
[ Webcam / USB Camera ]
           |
           v
+-----------------------------+
|  Python OpenCV Light Node   |
|  - Grayscale averaging      |
|  - Threshold evaluation     |
|  - Video recording          |
+-----------------------------+
           |
           | CSV + Video Files
           v
+-----------------------------+
|  MATLAB / Analysis Layer    |
+-----------------------------+
```

---

## Folder Structure

```
PythonOCV & LVL Sensor/
├─ .venv/
├─ .vscode/
├─ Data/
│  └─ LightLog.csv
├─ Videos/
│  ├─ Output1.mp4
│  ├─ Output2.mp4
│  └─ ...
├─ Light_Level.py
├─ Recording.py
├─ Webcam1.py
└─ README.md
```

---

## File Breakdown

### `Light_Level.py`

Primary execution script.

**Responsibilities:**

* Capture frames from the webcam
* Compute per-frame grayscale averages
* Accumulate brightness statistics per second
* Detect brightness changes using absolute and relative thresholds
* Log results to CSV
* Coordinate recording and sensor state

This file defines the **light-level detection logic**.

---

### `Recording.py`

Handles video recording functionality.

**Responsibilities:**

* Initialize `VideoWriter`
* Write frames while recording is active
* Manage file naming and output location

---

### `Webcam1.py`

Handles camera setup and frame acquisition.

**Responsibilities:**

* Initialize webcam device
* Abstract camera read logic
* Provide frames to other modules

---

### `Data/`

Stores **CSV light-level logs**.

Each log:

* Contains one row per second
* Indicates whether a brightness change was detected
* Is intended for correlation with motion, audio, and thermal data

(Currently sequential naming is implemented; this may evolve further.)

---

### `Videos/`

Stores recorded video files associated with light-level monitoring.

Videos provide **visual context** for detected lighting events.

---

## Current Detection Strategy

The system:

* Converts frames to grayscale
* Computes mean brightness per frame
* Aggregates brightness over one-second windows
* Flags a change when:

  * Absolute brightness delta exceeds a threshold, or
  * Relative brightness change exceeds a percentage threshold

---

## Planned Evolution

Future enhancements may include:

* Adaptive baseline modeling
* Automatic exposure-change detection
* Synchronization with motion sensor timelines
* Unified event IDs across PC-side modules
* Configurable sensitivity profiles

---

## Summary

This Python OpenCV module provides:

* Ambient light monitoring
* Detection of camera obstruction or exposure
* Structured CSV logs
* Video context for environmental events

It acts as the **environmental context companion** to the C++ motion sensor.