# CAMSENS

### Camera and Sensor-Based Distributed Security System

**CAMSENS** is a modular, distributed security system project built as a hands-on learning platform for **systems engineering, embedded development, computer vision, data analysis, and infrastructure orchestration**.

Rather than focusing on a single technology, CAMSENS intentionally combines **physical sensors**, **computer vision**, **centralized data storage**, and **analysis-driven alerting** into a cohesive, extensible system. The project evolves in phases and is designed to scale from software simulation to real-world hardware deployment.

---

## 🎯 Project Motivation & Learning Goals

CAMSENS began as a way to learn **C and C++**, but has since grown into a full-stack systems project intended to develop practical skills in:

* **Embedded systems** (Arduino, real sensors)
* **Computer vision** (C++ & Python with OpenCV)
* **Data analysis** (MATLAB)
* **Databases & SQL**
* **Infrastructure & DevOps** (Docker, Linux, VPNs)
* **Networking & APIs** (SMS alerting)
* **Hardware design** (future PCB design in Altium)

The goal is not just to detect events, but to **design a realistic, maintainable security system architecture** from the ground up.

---

## 🧠 System Overview

At a high level, CAMSENS is composed of **five cooperating layers**, each with a clear responsibility.

```
[ Physical Sensors ]
        |
        v
+------------------------+
|  Arduino Sensor Nodes  |
+------------------------+
        |
        |  Events
        v
+------------------------+
|  OpenCV (C++ / Python) |
+------------------------+
        |
        |  Structured Data
        v
+------------------------+
|  SQL Database          |
+------------------------+
        |
        v
+------------------------+
|  MATLAB Analysis       |
|  + Alert Rules         |
+------------------------+
        |
        v
+------------------------+
|  SMS / Notifications   |
+------------------------+
```

Each layer is intentionally decoupled so that components can be developed, tested, and replaced independently.

---

## 🔩 Physical Sensing Layer (Arduino)

The Arduino layer interfaces directly with the real world and acts as an **event generator**, not a decision engine.

### Capabilities

* Thermal presence detection
* Temperature and humidity monitoring
* Audio activity detection
* Physical integrity & tamper detection
* Environmental hazard detection (gas, flame, water)
* Local user interaction (buttons, RFID, keypad)
* Physical outputs (buzzers, relays, indicators)

### Design Philosophy

* Node-based firmware architecture
* Non-blocking, event-driven logic
* Minimal processing on microcontrollers
* Clear separation between *sensing* and *analysis*

📁 See `Arduino/README.md` for detailed node breakdowns.

---

## 👁️ Vision & Perception Layer (OpenCV)

CAMSENS uses **two independent OpenCV subsystems**, each serving a different perceptual role.

### C++ OpenCV – Motion Detection

* Real-time camera capture
* Significant motion detection
* Evidence-grade video recording
* Sequential CSV and video output
* Deterministic, timed execution

This serves as the **primary visual motion sensor**.

---

### Python OpenCV – Light Level & Environmental Context

* Ambient brightness monitoring
* Detection of sudden illumination changes
* Camera obstruction or exposure events
* Synchronized video recording and logging

This subsystem provides **environmental context** that motion detection alone cannot capture.

---

## 🗄️ Data Centralization Layer (SQL)

As the project evolved, it became clear that **CSV-based workflows do not scale**:

* Multiple languages writing to CSVs is unsafe
* MATLAB is inefficient when reading many separate files
* Cross-process coordination is difficult

### Current Direction

CAMSENS is transitioning to a **centralized SQL database**, running as a Docker service, which will:

* Act as the single source of truth for all events
* Accept writes from:

  * Arduino nodes
  * C++ OpenCV programs
  * Python OpenCV programs
* Be queried directly by MATLAB for analysis

This layer also serves as the primary vehicle for learning **SQL and data modeling**.

---

## 📊 Analysis & Alerting Layer (MATLAB)

MATLAB is used as the **analysis and decision engine** of CAMSENS.

### Responsibilities

* Query centralized sensor and vision data
* Correlate events across subsystems
* Detect anomalies and patterns
* Apply configurable thresholds and rules
* Decide when alerts should be triggered

### Alerting

Rather than sending messages directly, MATLAB communicates with a **containerized alert service**, which then uses an SMS API (e.g., Twilio or Textbelt) to send notifications.

This separation keeps:

* MATLAB focused on analysis
* Messaging focused on delivery and reliability

---

## 🐳 Infrastructure & Deployment

CAMSENS' messaging alerts protocol is soon to be developed, securely self-hosted, and deployed on a **custom built dual-boot Windows / Ubuntu PC**, which also hosts other services such as Immich, NextCloud, and more.

### Infrastructure Highlights

* Dockerized services
* Mesh VPN (Tailscale) for secure remote connectivity
* CAMSENS runs as one containerized service among others
* Designed for headless operation and remote monitoring

This layer provides and supplements existing hands-on experience with **Linux, Docker, networking, and service orchestration**.

---

## 🔮 Future Hardware Integration

Once the software and firmware layers are stable, CAMSENS is designed to evolve into a **single integrated hardware platform**.

### Planned Future Work

* PCB design using **Altium**
* Consolidation of sensors and microcontrollers
* Cleaner power and signal routing
* Portable deployment
* Potential migration of OpenCV workloads to a **Raspberry Pi**
* Arduino devices acting as sensor co-processors

This phase is intentionally deferred until the system’s behavior is fully understood in software.

---

## 📁 Repository Structure (High Level)

```
CAMSENS/
├─ Arduino/                 # Embedded firmware (node-based)
├─ C++OCV Motion Sensor/    # C++ OpenCV motion detection
├─ PythonOCV & LVL Sensor/  # Python OpenCV light-level sensing
├─ MATLABAnalysis_TextAlerts/
├─ SimulatedData_InputLogging/
├─ Docker/ (planned)
└─ README.md
```

Each major subsystem has its own README with detailed documentation.

---

## 🚀 Current Status

* ✅ OpenCV motion detection (C++)
* ✅ OpenCV light-level sensing (Python)
* ✅ Arduino hardware acquired and architecture validated
* ✅ Modular firmware design established
* ⚠️ SQL data layer in active development
* ⚠️ MATLAB alerting integrated via containerized messaging
* 🔮 PCB design planned for later phase

---

## 🧭 Closing Notes

CAMSENS is intentionally **iterative**. Some subsystems are fully operational, others establish infrastructure for future capabilities. This README file reflects both **what exists today** and **what the system is designed to grow into**.

The project prioritizes:

* Architectural clarity
* Realistic constraints
* Incremental validation
* Learning through implementation

## Author: hsingh837