#This is our actual light level sensor code. 


#I want to start off with the recording code we had from before, and modify it/build atop it.

import cv2 as cv
import numpy as np
import time, csv, os as OS

#These are my 'tunable values', as in the sensitivity of the light level sensor

THRESH_ABS = 8.0
THRESH_REL = 0.1
CSV_NAME = "LightLog.csv"

#End of tunables

def changed(curr_avg, prev_avg):
    if prev_avg is None or np.isnan(curr_avg) or np.isnan(prev_avg):
        return False
    diff = abs(curr_avg - prev_avg)
    rel = diff / max (1.0, prev_avg)
    return (diff >= THRESH_ABS) or (rel >= THRESH_REL)

OS.chdir(OS.path.dirname(OS.path.abspath(__file__)))
OS.makedirs("Videos", exist_ok = True)


cap = cv.VideoCapture(0)
if not cap.isOpened():
    print("Cannot open camera")
    raise SystemExit

fourcc = cv.VideoWriter_fourcc(*'mp4v')
out = None
recording = False

#State of Light sensor
sensor_active = False
sensor_start = None
sensor_file = None
sensor_writer = None
last_second_written = -1 #writing rows 0 - 30
accum_sum = 0.0
accum_count = 0
prev_avg = None

print("Preview running. Press 'r' to start recording, and 'L' to start light sensor (while recording)")


while True:
    ok, frame = cap.read()
    if not ok:
        print("No frame, exiting...")
        break

    cv.imshow('frame', frame)
    key = cv.waitKey(1) & 0xFF


    #handling keypresses
    if key == ord('r') and not recording:
        #initialize recording to save with the next sequential filename
        existing = [f for f in OS.listdir("Videos") if f.startswith("Output") and f.endswith(".mp4")]
        filename = f"Videos/Output{len(existing) + 1}.mp4"

        out = cv.VideoWriter(filename, fourcc, 20.0, (frame.shape[1], frame.shape[0]))
        if not out.isOpened():
            print("Failed to open VideoWriter")
        else:
            recording = True
            print(f"Recording Started: {filename}")

    elif key in (ord('l'), ord('L')) and recording and not sensor_active: #dealing with uppercase and lowercase keypress, should do this for recording too.
        #start light sensor logging
        sensor_file = open(CSV_NAME, "w", newline="")
        sensor_writer = csv.writer(sensor_file)
        sensor_writer.writerow(["time", "Detection"]) #The header of the .csv file
        sensor_active = True
        sensor_start = time.time()
        last_second_written = -1
        accum_sum = 0.0
        accum_count = 0
        prev_avg = None
        print(f"Light sensor started --> logging to {CSV_NAME}")

    elif key == 27: #ESC key
        if recording:
            recording = False
            if out is not None:
                out.release()
                print("Recording Stopped")
        
        #finalizing the sensor and its data logging upon stopping recording

        if sensor_active:
            #flushing remaining seconds up to 30 if we have data
            now = time.time()
            elapsed = now - sensor_start
            
            #calculating a final with accumulated samples (if accumulated samples are present)
            if accum_count > 0 and last_second_written < 30:
                curr_avg = accum_sum / accum_count
                det = "C" if changed(curr_avg, prev_avg) else "NC"
                sensor_writer.writerow([last_second_written + 1, det])
                prev_avg = curr_avg
                last_second_written += 1
            sensor_file.close()
            sensor_active = False
            print(f"Light Sensor stopped --> saved {CSV_NAME} (rows: {last_second_written + 1} incl. header)")
        break

    #writing the frames while recording
    if recording and out is not None:
        out.write(frame)

    #light sensor processing (running while active)
    if sensor_active:
        #per-frame brightness accumulation initiation
        gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY) 
        accum_sum += float(gray.mean())
        accum_count += 1 

        #calculating how many seconds elapsed since sensor start
        elapsed = time.time() - sensor_start
        current_sec = int(elapsed) #e.g. 0, 1, 2....


        #writing rows in the .csv for seconds completed
        while current_sec > last_second_written and last_second_written < 30:
            if accum_count > 0:
                curr_avg = accum_sum / accum_count
            else:
                curr_avg = float("nan")

            #for 1st 'completed' second (@t = 0), no previous baseline for brightness change so it MUST be NC --> no change
            det = "C" if (prev_avg is not None and changed(curr_avg, prev_avg)) else "NC"


            #time column in .csv, next second index but we start at 0
            t_value = last_second_written + 1
            sensor_writer.writerow([t_value, det])
            print(f"[Sensor] t = {t_value:2d}s avg = {curr_avg:.1f} det = {det}")


            #preparing for the next interval
            prev_avg = curr_avg
            last_second_written += 1
            accum_sum = 0.0
            accum_count = 0

            #We stop automatically at 30 seconds
            if last_second_written >= 30:
                sensor_file.close()
                sensor_active = False
                print(f"Light sensor complete --> saved {CSV_NAME}")
                break

#finalize all and cleanup
cap.release()
if out is not None:
    out.release()
cv.destroyAllWindows()


#CNN - Convolutional Neural Network