from flask import Flask, render_template
import threading
import cv2
import numpy as np
import serial
import time

app = Flask(__name__)

# Counters
dry_waste_count = 0
wet_waste_count = 0
total_people = 0

# Arduino setup
arduino = serial.Serial('COM9', 9600)
time.sleep(2)

# Function for detection
def camera_loop():
    global dry_waste_count, wet_waste_count, total_people
    cap = cv2.VideoCapture(0)

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        # Green range
        lower_green = np.array([40, 70, 70])
        upper_green = np.array([80, 255, 255])
        mask_green = cv2.inRange(hsv, lower_green, upper_green)

        # Blue range
        lower_blue = np.array([100, 150, 0])
        upper_blue = np.array([140, 255, 255])
        mask_blue = cv2.inRange(hsv, lower_blue, upper_blue)

        green_count = cv2.countNonZero(mask_green)
        blue_count = cv2.countNonZero(mask_blue)

        if green_count > 2000:
            arduino.write(b'2')
            wet_waste_count += 1
            total_people += 1
            time.sleep(1)
        elif blue_count > 2000:
            arduino.write(b'1')
            dry_waste_count += 1
            total_people += 1
            time.sleep(1)
        else:
            arduino.write(b'0')

        cv2.imshow("Dustbin Camera", frame)
        if cv2.waitKey(30) & 0xFF == 27:
            break

    cap.release()
    cv2.destroyAllWindows()

@app.route('/')
def index():
    return f"""
    <h1>Smart Dustbin Dashboard</h1>
    <p>Total People: {total_people}</p>
    <p>Dry Waste Count: {dry_waste_count}</p>
    <p>Wet Waste Count: {wet_waste_count}</p>
    """

if __name__ == "__main__":
    # Run camera in background thread
    t = threading.Thread(target=camera_loop)
    t.start()

    app.run(host="0.0.0.0", port=5000)
