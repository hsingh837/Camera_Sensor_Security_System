# CAMSENS
### Camera and Sensor-Based Security System Project

**CAMSENS** is an exploratory project that I undertook to learn C, C++, and further my skills in Python with. I also wanted to gain insight into the fundamentals of MATLAB, Microcontroller implementation, and user front-end/back-end interactions. This Project is designed to combine sensor simulation, computer vision, and data analysis to build a customizable security system. It's an evolving platform to learn and apply:

- **C** – for simulating sensor data
- **C++** and **Python** – for real-time video processing with OpenCV
- **MATLAB** – for data analysis and alert systems
- **Arduino** – for physical sensor integration

---

## 📌 Current Progress

### ✅ C: Simulated Sensor Data Input and Logging
- Created a C program that simulates 100 iterations of sensor data over time.
- No delay was added, but a 1-second interval is planned.
- This served as the foundational layer for future OpenCV development.

### ✅ Python (OpenCV): Light Level Sensor
- Recording begins on keypress `r`, ends on `Esc`.
- Currently working on detecting and logging light level changes.
- Next step: Write intensity/timestamp logs to `.csv`.

### ✅ C++ (OpenCV): Motion Detection
- In development: records video when motion is detected, up to 30 seconds (potential weakness in security).
- Stops recording when no motion is detected.

### ✅ MATLAB: Data Analysis & Alerts
- Developing scripts to analyze `.csv` sensor logs and identify anomalies.
- Future goal: Send alerts (email/SMS) when discrepancies are detected.
- Basic UI in progress for user alert preferences.

---

## 🔧 In Progress / Planned

- Integrate Arduino to support thermal, sound, and environmental sensors.
- Centralized logging system across all components.
- Develop modular structure to allow plug-and-play of different sensors.
- AI/ML integration for anomaly detection and smarter alerting.
- Workflow automation.

---

## 🚀 Goals

- Unify all programs into a cohesive security system.
- Log all sensor events to a central database or file system.
- Analyze patterns via MATLAB and trigger alerts based on defined thresholds.
- Explore AI enhancements and user-friendly UI/UX.

---

## 🛠 Technologies Used

- C / C++
- Python (OpenCV)
- MATLAB
- Arduino
- Bash / Git for automation

---

## 👨‍💻 Author

Developed by hsingh837.

---

