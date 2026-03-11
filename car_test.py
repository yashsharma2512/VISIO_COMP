import cv2
import numpy as np
import tensorflow as tf
import serial
import time

# ===============================
# SETTINGS
# ===============================

MODEL_PATH = "keras_model.h5"
LABELS_PATH = "labels.txt"

SERIAL_PORT = "COM29"      # change this
BAUD_RATE = 115200
CAMERA_INDEX = 2
IMG_SIZE = 224
CONF_THRESHOLD = 0.70
SEND_DELAY = 0.2

# ===============================
# LOAD MODEL + LABELS
# ===============================

model = tf.keras.models.load_model(MODEL_PATH, compile=False)

with open(LABELS_PATH, "r") as f:
    class_names = [line.strip() for line in f.readlines()]

# ===============================
# SERIAL SETUP
# ===============================

arduino = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)

# ===============================
# CAMERA SETUP
# ===============================

cap = cv2.VideoCapture(CAMERA_INDEX)

last_command = ""
last_sent_time = 0


def clean_label(label):
    # Teachable Machine labels are often like "0 forward"
    parts = label.split(" ", 1)
    if len(parts) == 2 and parts[0].isdigit():
        return parts[1].strip().lower()
    return label.strip().lower()
 
 
def label_to_command(label):
    label = clean_label(label)

    # Change these names to match your model classes
    if label == "forward":
        return "forward"
    elif label == "backward":
        return "backward"
    elif label == "left":
        return "left"
    elif label == "right":
        return "right"
    elif label == "stop":
        return "stop"
    else:
        return "stop"


def send_command(command):
    global last_command, last_sent_time
    now = time.time()

    if command != last_command or (now - last_sent_time) > SEND_DELAY:
        arduino.write((command + "\n").encode())
        print("Sent:", command)
        last_command = command
        last_sent_time = now


while True:
    ret, frame = cap.read()
    if not ret:
        break

    display = frame.copy()

    # Prepare image for Teachable Machine
    img = cv2.resize(frame, (IMG_SIZE, IMG_SIZE))
    img = np.asarray(img, dtype=np.float32)
    img = (img / 127.5) - 1
    img = np.expand_dims(img, axis=0)

    # Prediction
    prediction = model.predict(img, verbose=0)
    index = int(np.argmax(prediction))
    confidence = float(prediction[0][index])
    raw_label = class_names[index]
    label = clean_label(raw_label)

    if confidence >= CONF_THRESHOLD:
        command = label_to_command(raw_label)
    else:
        command = "stop"

    send_command(command)

    cv2.putText(display, f"Label: {label}", (20, 40),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
    cv2.putText(display, f"Confidence: {confidence*100:.1f}%", (20, 80),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
    cv2.putText(display, f"Command: {command}", (20, 120),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)

    cv2.imshow("Teachable Machine Robot Control", display)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

send_command("stop")
cap.release()
cv2.destroyAllWindows()
arduino.close()
