import cv2
import numpy as np
import csv
import os
import time
import threading

# ===================== CONFIG =====================
DURATION_SEC = 120
THRESH_ABS = 8.0
THRESH_REL = 0.1
# ==================================================

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_VIDEO_DIR = os.path.join(BASE_DIR, "Videos")
OUTPUT_DATA_DIR  = os.path.join(BASE_DIR, "Data")

os.makedirs(OUTPUT_VIDEO_DIR, exist_ok=True)
os.makedirs(OUTPUT_DATA_DIR, exist_ok=True)

# ==================================================
# Utility functions
# ==================================================

def next_index(prefix, suffix, folder):
    idx = 1
    while os.path.exists(os.path.join(folder, f"{prefix}{idx}{suffix}")):
        idx += 1
    return idx

def light_changed(curr, prev):
    if prev is None:
        return "NC"
    abs_change = abs(curr - prev)
    rel_change = abs_change / max(prev, 1e-6)
    return "C" if (abs_change >= THRESH_ABS or rel_change >= THRESH_REL) else "NC"

# ==================================================
# Threaded camera class
# ==================================================

class CameraStream:
    def __init__(self, src):
        self.cap = cv2.VideoCapture(src)
        if not self.cap.isOpened():
            raise RuntimeError(f"Camera {src} could not be opened")

        self.ret, self.frame = self.cap.read()
        self.running = True
        self.lock = threading.Lock()

        self.thread = threading.Thread(target=self.update, daemon=True)
        self.thread.start()

    def update(self):
        while self.running:
            ret, frame = self.cap.read()
            if ret:
                with self.lock:
                    self.ret = ret
                    self.frame = frame

    def read(self):
        with self.lock:
            return self.ret, self.frame

    def release(self):
        self.running = False
        self.thread.join()
        self.cap.release()

# ==================================================
# Camera initialization
# ==================================================

try:
    cam1 = CameraStream(0)
except RuntimeError:
    print("Error: Camera 0 could not be opened.")
    exit(1)

try:
    cam2 = CameraStream(1)
    cam2_available = True
except RuntimeError:
    cam2 = None
    cam2_available = False
    print("Camera 1 not detected. Running in single-camera mode.")

# Frame size from camera 0
w = int(cam1.cap.get(cv2.CAP_PROP_FRAME_WIDTH))
h = int(cam1.cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

if w == 0 or h == 0:
    ret, frame = cam1.read()
    if not ret:
        print("Error: Could not read from Camera 0.")
        exit(1)
    h, w = frame.shape[:2]

fps = 30

# ==================================================
# File paths
# ==================================================

idx_data = next_index("LightLog", ".csv", OUTPUT_DATA_DIR)
idx_vid  = next_index("Cam1_OutputVideo", ".mp4", OUTPUT_VIDEO_DIR)

csv_path = os.path.join(OUTPUT_DATA_DIR, f"LightLog{idx_data}.csv")
v1_path  = os.path.join(OUTPUT_VIDEO_DIR, f"Cam1_OutputVideo{idx_vid}.mp4")
v2_path  = os.path.join(OUTPUT_VIDEO_DIR, f"Cam2_OutputVideo{idx_vid}.mp4")

fourcc = cv2.VideoWriter_fourcc(*"mp4v")
out1 = cv2.VideoWriter(v1_path, fourcc, fps, (w, h))
out2 = None

if cam2_available:
    out2 = cv2.VideoWriter(v2_path, fourcc, fps, (w, h))

# ==================================================
# State variables
# ==================================================

print("Press R to record and L to start light level sensor. ESC to exit.")

recording = False
sensor_active = False
start_time = None
last_second = 0

accum1_sum = accum1_cnt = 0
accum2_sum = accum2_cnt = 0
prev_avg1 = prev_avg2 = None

csv_file = open(csv_path, "w", newline="")
writer = csv.writer(csv_file)

if cam2_available:
    writer.writerow(["time", "Cam1", "Cam2"])
else:
    writer.writerow(["time", "Cam1"])

# ==================================================
# Main loop
# ==================================================

while True:
    ret1, frame1 = cam1.read()
    if not ret1:
        break

    if cam2_available:
        ret2, frame2 = cam2.read()
        if not ret2:
            cam2_available = False

    cv2.imshow("Camera 1", frame1)
    if cam2_available:
        cv2.imshow("Camera 2", frame2)

    key = cv2.waitKey(1) & 0xFF

    if key == ord('r') and not recording:
        recording = True
        print("Recording started, video saving to ./Videos")

    if key == ord('l') and recording and not sensor_active:
        sensor_active = True
        start_time = time.time()
        last_second = -1
        print(f"Light Level Sensor started, logging data to {csv_path}")

    if key == 27:
        break

    if recording:
        out1.write(frame1)
        if cam2_available:
            out2.write(frame2)

    if sensor_active:
        gray1 = cv2.cvtColor(frame1, cv2.COLOR_BGR2GRAY)
        accum1_sum += gray1.mean()
        accum1_cnt += 1

        if cam2_available:
            gray2 = cv2.cvtColor(frame2, cv2.COLOR_BGR2GRAY)
            accum2_sum += gray2.mean()
            accum2_cnt += 1

        elapsed = int(time.time() - start_time)

        if elapsed > last_second:
            if accum1_cnt == 0:
                last_second = elapsed
                continue

            avg1 = accum1_sum / accum1_cnt
            det1 = light_changed(avg1, prev_avg1)

            if cam2_available:
                if accum2_cnt == 0:
                    last_second = elapsed
                    continue

                avg2 = accum2_sum / accum2_cnt
                det2 = light_changed(avg2, prev_avg2)

                row = [elapsed, det1, det2]
                print(f"{elapsed}, {det1}, {det2}")
            else:
                row = [elapsed, det1]
                print(f"{elapsed}, {det1}")

            writer.writerow(row)

            prev_avg1 = avg1
            if cam2_available:
                prev_avg2 = avg2

            accum1_sum = accum1_cnt = 0
            accum2_sum = accum2_cnt = 0
            last_second = elapsed

        if elapsed >= DURATION_SEC:
            break

# ==================================================
# Cleanup
# ==================================================

csv_file.close()
cam1.release()
if cam2_available:
    cam2.release()

out1.release()
if out2 is not None:
    out2.release()

cv2.destroyAllWindows()
print("Program exited cleanly.")
