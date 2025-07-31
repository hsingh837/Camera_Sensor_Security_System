import cv2 as cv
import numpy as np
import os as OS

OS.chdir(OS.path.dirname(OS.path.abspath(__file__)))
OS.makedirs("Videos", exist_ok = True)

cap = cv.VideoCapture(0)

if not cap.isOpened():
    print("Cannot open camera")
    exit()

fourcc = cv.VideoWriter_fourcc(*'mp4v')
out = None
recording = False

while True:
    ret, frame = cap.read()

    if not ret:
        print("No frame, exiting ")
        break

    cv.imshow('frame', frame)
    key = cv.waitKey(1) & 0xFF

    if key == ord('r') and not recording:
        existing_files = [f for f in OS.listdir("Videos") if f.startswith("Output") and f.endswith(".mp4")]
        next_index = len(existing_files) + 1
        filename = f"Videos/Output{next_index}.mp4"

        recording = True
        out = cv.VideoWriter(filename, fourcc, 20.0, (frame.shape[1], frame.shape[0]))
        print("Recording Started:", {filename})
    
    elif key == 27: #esc key
        if recording:
            recording = False
            out.release()
            print("Recording Stopped:", {filename})
        break

    if recording:
        out.write(frame)

cap.release()
if out:
    out.release()
cv.destroyAllWindows()