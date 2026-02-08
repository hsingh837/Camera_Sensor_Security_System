import cv2 as cv
import numpy as np

cap = cv.VideoCapture(1)

if not cap.isOpened():
    print("Cannot open camera")
    exit()

while True:
    #capturing video from webcam frame by frame
    ret, frame = cap.read()


    #if frame read properly ret is true
    if not ret:
        print("Can't receive frame (stream end?). Exiting ...")
        break


    cv.imshow('frame', frame)
    if cv.waitKey(1) == ord('q'):
        break

#when all is done
cap.release()
cv.destroyAllWindows()
