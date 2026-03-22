import socket
import cv2
import time
from cvzone.ClassificationModule import Classifier

# ------------------- CONFIG -------------------
ESP32_IP = "192.168.4.1"
PORT = 4210

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Optional: reduce delay (important for real-time)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1024)

# ------------------- CAMERA + MODEL -------------------
cap = cv2.VideoCapture(2)
classifier = Classifier("gesmodel.h5", "labelsg.txt")

labels = ["LEFT", "RIGHT", "FORWARD"]

# ------------------- CONTROL SETTINGS -------------------
CONFIDENCE_THRESHOLD = 0.75   # adjust if jittery
SEND_INTERVAL = 0.05         # 50 ms → ~20 FPS control

last_sent_time = 0
last_cmd = "STOP"

# ------------------- SEND FUNCTION -------------------
def send_command(cmd):
    try:
        sock.sendto(cmd.encode(), (ESP32_IP, PORT))
    except:
        pass

# ------------------- MAIN LOOP -------------------
while True:
    success, img = cap.read()
    if not success:
        continue

    prediction, index = classifier.getPrediction(img)

    confidence = prediction[index]
    cmd = labels[index]

    # ------------------- FILTERING -------------------
    if confidence < CONFIDENCE_THRESHOLD:
        cmd = "STOP"

    # ------------------- RATE LIMITING -------------------
    current_time = time.time()

    if current_time - last_sent_time > SEND_INTERVAL:
        send_command(cmd)
        last_sent_time = current_time

        # Debug print (only when changed)
        if cmd != last_cmd:
            print(f"Sent: {cmd} | Confidence: {confidence:.2f}")
            last_cmd = cmd

    # ------------------- DISPLAY -------------------
    cv2.putText(img, f"{cmd} ({confidence:.2f})", (20, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    cv2.imshow("Gesture Control", img)

    if cv2.waitKey(1) & 0xFF == 27:
        break

cap.release()
cv2.destroyAllWindows()