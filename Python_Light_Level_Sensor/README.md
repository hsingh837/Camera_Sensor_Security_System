## Python OpenCV Light-Level Sensor – Environmental Vision Node

This directory contains the **Python-based light-level monitoring component** of the CAMSENS project. It implements a **PC-side environmental vision node** that detects rapid or sustained changes in ambient brightness, records synchronized video, and logs structured light-level events to CSV for downstream analysis.

Over time, this component has evolved from a single-camera prototype into a **robust, multi-camera, threaded sensing system**. The files in this folder intentionally represent **distinct architectural stages**, not redundant scripts. Each version captures a deliberate design milestone.

This module complements motion detection by answering **environmental context questions** that are difficult to infer from motion alone.

---

## System Role

At a system level, this project answers:

* Has the ambient lighting changed significantly?
* Did lighting changes coincide with motion or audio events?
* Was the camera blinded, covered, or exposed to sudden illumination?

It acts as an **environmental awareness layer** within the broader CAMSENS sensing pipeline.

---

## High-Level Architecture

```
[ Webcam / USB Camera(s) ]
           |
           v
+----------------------------------+
|  Python OpenCV Light-Level Node   |
|  - Grayscale averaging            |
|  - Threshold evaluation           |
|  - Multi-camera support           |
|  - Optional threaded capture      |
|  - Video recording                |
+----------------------------------+
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
│  └─ LightLog[index].csv
├─ Videos/
│  ├─ Cam1_OutputVideo[index].mp4
│  ├─ Cam2_OutputVideo[index].mp4
│  └─ ...
├─ Light_Level.py
├─ Light_Level_DualCam.py
├─ Light_Level_Threaded.py
└─ README.md
```

---

## File Breakdown

### 1️⃣ `Light_Level.py` — Single-Camera Baseline

**Purpose:**
The original, foundational implementation of the light-level sensor using **one camera (Camera 0)**.

**Responsibilities:**

* Capture frames from the webcam
* Compute per-frame grayscale averages
* Accumulate brightness statistics per second
* Detect brightness changes using absolute and relative thresholds
* Log results to CSV
* Display a live preview

**Why this file still matters:**

* Serves as a clean reference implementation
* Minimal logic, easy to reason about
* Useful for algorithm testing without multi-camera or concurrency complexity

This script established the **core light-level detection logic** used by all later versions.

---

### 2️⃣ `Light_Level_DualCam.py` — Dual-Camera, Single-Threaded

**Purpose:**
Extend the light-level sensor to support **two cameras simultaneously**, while preserving a single unified timeline and CSV log.

**Key features:**

* Supports **Camera 0 (required)** and **Camera 1 (optional)**

* Gracefully falls back to single-camera mode if Camera 1 is unavailable

* Independent brightness tracking per camera

* One CSV row per second with synchronized output:

  ```
  time, Cam1, Cam2
  ```

* Independent video recording per camera

* Separate indexing for video files and CSV logs

* Output directories resolved **relative to the script location**

**Design highlights:**

* Single authoritative clock for all sensing
* Camera 2 never blocks Camera 1
* Robust handling of missing or disconnected hardware

**Limitations:**

* Camera capture is performed sequentially
* A slow or blocking camera read can delay the main loop

This version validated **multi-camera correctness and fault tolerance** before introducing concurrency.

---

### 3️⃣ `Light_Level_Threaded.py` — Dual-Camera with Threaded Capture

**Purpose:**
Introduce **threading at the camera I/O layer** to improve performance, stability, and timing accuracy while keeping sensing logic unchanged.

**What threading changes:**

* Each camera runs in its **own background capture thread**
* Frames are continuously acquired and buffered
* The main loop always consumes the **latest available frame**

**What threading does *not* change:**

* Light-level detection logic
* CSV format and logging cadence
* Video recording behavior
* User interaction model (`R` to record, `L` to start sensing)

---

## Why Threading Matters

Threading was introduced **only after the sensing logic was proven correct**. This ensures concurrency is a performance upgrade rather than a workaround for logic bugs.

### Performance & Stability Benefits

* **Non-blocking capture:** A slow or stalled camera can no longer freeze the main loop
* **Improved timing accuracy:** One-second sensing windows are more consistent
* **Better multi-camera fairness:** Each camera captures at its own cadence
* **USB and driver resilience:** Temporary stalls do not halt sensing

In practice, this results in smoother previews, more reliable CSV data, and cleaner separation between I/O and computation.

---

## Current Detection Strategy

Across all versions, the system:

* Converts frames to grayscale
* Computes mean brightness per frame
* Aggregates brightness over one-second windows
* Flags a change when either:

  * Absolute brightness delta exceeds a threshold, or
  * Relative brightness change exceeds a percentage threshold

Detection results are logged once per second and mirrored to the terminal.

---

## Future Applications Enabled

The current architecture enables straightforward expansion to:

* Higher camera counts
* Heterogeneous sensors (thermal, IR, depth)
* Sensor fusion (light + motion + audio)
* Real-time event correlation
* Long-running unattended operation
* Migration to embedded Linux or ROS-style pipelines

Because threading is isolated to the capture layer, these upgrades can be added **without rewriting the sensing logic**.

---

## Architectural Philosophy

This component follows a layered design:

1. **Camera I/O layer** (threaded where appropriate)
2. **Single time authority** for sensing
3. **Deterministic sensor logic**
4. **Structured data logging** (CSV as ground truth)

This mirrors real-world vision systems used in surveillance, robotics, and industrial monitoring.

---

## Summary

This Python OpenCV light-level sensor provides:

* Ambient light monitoring
* Detection of camera obstruction or exposure
* Structured CSV logs
* Video context for environmental events
* Robust single- and multi-camera support
* Optional threaded capture for performance

It acts as the **environmental context companion** to the C++ motion sensor and is a key building block within the CAMSENS project.
