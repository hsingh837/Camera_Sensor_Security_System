import cv2
import numpy as np
import csv
import os
import time

# ===================== CONFIG =====================
DURATION_SEC = 120
THRESH_ABS = 8.0
THRESH_REL = 0.1
# ==================================================c

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

OUTPUT_VIDEO_DIR = os.path.join(BASE_DIR, "Videos")
OUTPUT_DATA_DIR  = os.path.join(BASE_DIR, "Data")


os.makedirs(OUTPUT_VIDEO_DIR, exist_ok=True)
os.makedirs(OUTPUT_DATA_DIR, exist_ok=True)

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

# ---------- Camera setup ----------
cap1 = cv2.VideoCapture(0)
if not cap1.isOpened():
    print("Error: Camera 0 could not be opened.")
    exit(1)

cap2 = cv2.VideoCapture(1)
cam2_available = cap2.isOpened()

if not cam2_available:
    print("Camera 1 not detected. Running in single-camera mode.")


w = int(cap1.get(cv2.CAP_PROP_FRAME_WIDTH))
h = int(cap1.get(cv2.CAP_PROP_FRAME_HEIGHT))

# Failsafe: if the driver reports 0, fall back to a single read (no break here)
if w == 0 or h == 0:
    ret1, frame1 = cap1.read()
    if not ret1:
        print("Error: Could not read from Camera 0.")
        exit(1)
    h, w = frame1.shape[:2]

fps = 30

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


print("Press R to record and L to start light level sensor. ESC to exit.")

recording = False
sensor_active = False
start_time = None
last_second = 0

# Per-camera accumulators
accum1_sum = accum1_cnt = 0
accum2_sum = accum2_cnt = 0
prev_avg1 = prev_avg2 = None

csv_file = open(csv_path, "w", newline="")
writer = csv.writer(csv_file)
if cam2_available:
    writer.writerow(["time", "Cam1", "Cam2"])
else:
    writer.writerow(["time", "Cam1"])


while True:
    ret1, frame1 = cap1.read()
    if not ret1:
        break

    if cam2_available:
        ret2, frame2 = cap2.read()
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
        print(f"Light Level Sensor started, logging data to {csv_path}.. will automatically terminate after 2 minutes (120 seconds)")

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


        elapsed = int(time.time() - start_time) if start_time is not None else 0


        if elapsed > last_second:
            # --- Camera 1 (always required) ---
            if accum1_cnt == 0:
                last_second = elapsed
                continue

            avg1 = accum1_sum / accum1_cnt
            det1 = light_changed(avg1, prev_avg1)

            # --- Camera 2 (optional) ---
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

csv_file.close()
cap1.release()
if cam2_available:
    cap2.release()

out1.release()
if out2 is not None:
    out2.release()

cv2.destroyAllWindows()

print("Program exited.")
