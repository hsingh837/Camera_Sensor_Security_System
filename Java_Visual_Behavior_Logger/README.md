# AutonomousVisionLogger (Java OpenCV)

### Autonomous Visual Telemetry & Behavior Logging Subsystem

**AutonomousVisionLogger** is the Java-based OpenCV perception module within the **CAMSENS** distributed security system. It is designed as an **independent, batch-executed visual sensor**, focused on extracting *temporal behavior* and *scene dynamics* from one or more webcams.

Unlike the C++ and Python OpenCV components, this subsystem does **not** operate in real time, nor does it interact directly with other sensors or services during execution. Instead, it runs autonomously for a fixed duration, logs structured visual telemetry to CSV files, and exits cleanly.

---

## 🎯 Purpose Within CAMSENS

This Java module exists to:

* Maximize the informational value of standard webcams
* Extract **behavioral features over time**, not just per-frame detections
* Produce **analysis-ready CSV outputs**
* Act as a drop-in sensing node within a larger data pipeline

It complements:

* **C++ OpenCV motion detection** (event-focused, real-time)
* **Python OpenCV light-level sensing** (environmental context)

By focusing on *temporal aggregation* and *visual behavior*, this module adds a higher-level perceptual layer to CAMSENS without increasing system coupling.

---

## 🧠 Design Philosophy

Key design principles:

* **Autonomous execution**
  No runtime dependencies on Arduino nodes, databases, or APIs.

* **Deterministic lifecycle**
  Start → observe → log → exit.

* **Evidence-first logging**
  Raw frame-level telemetry is preserved alongside session summaries.

* **Analysis-oriented outputs**
  CSV schemas are designed for MATLAB and SQL ingestion.

* **Incremental complexity**
  Initial implementations prioritize robustness and clarity over model sophistication.

---

## 🧩 Core Capabilities (Planned)

During its execution window, the program may extract:

* Motion intensity and direction (optical flow)
* Scene change and anomaly indicators
* Object persistence and dwell time
* Human presence confidence (non-continuous sampling)
* Aggregate behavior statistics over the session

These features are logged in two forms:

1. **Frame-level telemetry** (`frames.csv`)
2. **Session summary statistics** (`summary.csv`)

---

## 📁 Output Model

Each run generates a dedicated output directory:

```
data/runs/run_YYYY-MM-DD_HH-MM/
├── frames.csv    # Frame-by-frame visual telemetry
└── summary.csv   # Aggregated session metrics
```

CSV files are intentionally version-controlled to support:

* Reproducible analysis
* Offline debugging
* Dataset growth over time

Any video artifacts produced by this module are considered **derived data** and are excluded from version control.

---

## 🧱 Integration with CAMSENS

This module does **not** send alerts or make security decisions.

Instead, its outputs are consumed downstream by:

* The CAMSENS SQL data layer
* MATLAB-based analysis and correlation scripts
* The alerting and notification pipeline

This separation ensures:

* Clear responsibility boundaries
* Easier testing and validation
* Long-term architectural flexibility

---

## 🚧 Development Status

* Project structure established
* Execution model defined
* CSV-based data contract planned
* Feature implementation in progress

This subsystem is intentionally developed **slowly and deliberately** as part of CAMSENS’ long-term evolution.

---

## 📌 Notes

* This project prioritizes **learning and architectural clarity** over short-term optimization.
* Design decisions may evolve as real-world behavior is observed and analyzed.
* Major changes will be documented as the subsystem matures.