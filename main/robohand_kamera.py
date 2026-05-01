# Dieser Code wurde mit KI erweitert und verbessert.

import cv2
import mediapipe as mp
import serial
import time

# ============================================================
#  KONFIGURATION – hier anpassen
# ============================================================
NUM_HAENDE   = 1          # 1 oder 2
SERIAL_PORT  = '/dev/ttyUSB0'
BAUD_RATE    = 115200
SEND_DELAY   = 0.05       # Daten 20× pro Sekunde senden
CAM_WIDTH    = 640
CAM_HEIGHT   = 480
# ============================================================

print(f"Starte RoboHand Kamera-Modus mit {NUM_HAENDE} Hand/Händen")

# 1. SERIELLE VERBINDUNG
esp32 = serial.Serial(SERIAL_PORT, BAUD_RATE)
print("Warte auf ESP32...")
time.sleep(3)

# 2. MEDIAPIPE & WEBCAM
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH,  CAM_WIDTH)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT)

mp_drawing = mp.solutions.drawing_utils
mp_hands   = mp.solutions.hands

# max_num_hands richtet sich automatisch nach der Konfiguration
hand_detector = mp_hands.Hands(
    max_num_hands         = NUM_HAENDE,
    min_detection_confidence = 0.7,
    min_tracking_confidence  = 0.5
)

last_send_time = 0

# Farben für die zwei Hände
FARBE_HAND1 = (0, 255, 0)   # grün
FARBE_HAND2 = (0, 100, 255) # orange

draw_spec_h1 = mp_drawing.DrawingSpec(color=FARBE_HAND1, thickness=2, circle_radius=3)
draw_spec_h2 = mp_drawing.DrawingSpec(color=FARBE_HAND2, thickness=2, circle_radius=3)

print("MediaPipe Hands geladen. Starte Tracking...")
print(f"Drücke 'q' zum Beenden | '1'/'2' um Anzahl Hände umzuschalten\n")


def berechne_winkel(lm):
    """
    Berechnet eine einfache Offen/Zu-Klassifikation (0 oder 180)
    für alle 5 Finger aus einer Landmark-Liste.
    Gibt eine Liste mit 5 Winkeln zurück.
    """
    angles = []

    # Daumen: X-Koordinate (bewegt sich seitwärts)
    if lm[4].x > lm[2].x:
        angles.append(180)  # offen
    else:
        angles.append(0)    # geschlossen

    # Zeige-, Mittel-, Ring-, Kleiner Finger: Y-Koordinate
    finger_tips   = [8, 12, 16, 20]
    finger_joints = [6, 10, 14, 18]

    for i in range(4):
        if lm[finger_tips[i]].y < lm[finger_joints[i]].y:
            angles.append(180)  # Fingerspitze höher als Gelenk → offen
        else:
            angles.append(0)    # geschlossen

    return angles


def zeichne_hand(frame, hand_landmarks, farbe_punkt, farbe_linie, label=""):
    """Zeichnet die Hand-Landmarks mit individuellen Farben."""
    mp_drawing.draw_landmarks(
        frame,
        hand_landmarks,
        mp_hands.HAND_CONNECTIONS,
        mp_drawing.DrawingSpec(color=farbe_punkt, thickness=2, circle_radius=4),
        mp_drawing.DrawingSpec(color=farbe_linie, thickness=2)
    )
    # Label oben links an der Handgelenk-Position
    if label:
        wrist = hand_landmarks.landmark[0]
        h, w, _ = frame.shape
        cx, cy = int(wrist.x * w), int(wrist.y * h)
        cv2.putText(frame, label, (cx - 20, cy - 15),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, farbe_punkt, 2)


# ============================================================
#  HAUPTSCHLEIFE
# ============================================================
while True:
    success, frame = cap.read()
    if not success:
        continue

    # Bild spiegeln damit es intuitiver ist
    frame = cv2.flip(frame, 1)

    RGB_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result    = hand_detector.process(RGB_frame)

    angles_hand1 = None
    angles_hand2 = None

    if result.multi_hand_landmarks:
        detected = len(result.multi_hand_landmarks)

        # Hand 1
        hand1 = result.multi_hand_landmarks[0]
        zeichne_hand(frame, hand1,
                     FARBE_HAND1, (0, 200, 0), "Hand 1")
        angles_hand1 = berechne_winkel(hand1.landmark)

        # Hand 2 – nur wenn Modus auf 2 Hände und zwei erkannt wurden
        if NUM_HAENDE == 2 and detected >= 2:
            hand2 = result.multi_hand_landmarks[1]
            zeichne_hand(frame, hand2,
                         FARBE_HAND2, (0, 60, 200), "Hand 2")
            angles_hand2 = berechne_winkel(hand2.landmark)

    # Info-Overlay
    cv2.putText(frame, f"Modus: {NUM_HAENDE} Hand/Haende", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
    if angles_hand1:
        cv2.putText(frame, f"H1: {angles_hand1}", (10, 60),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, FARBE_HAND1, 2)
    if angles_hand2:
        cv2.putText(frame, f"H2: {angles_hand2}", (10, 90),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.55, FARBE_HAND2, 2)

    # ---- Daten senden ----
    current_time = time.time()
    if angles_hand1 and current_time - last_send_time > SEND_DELAY:

        if NUM_HAENDE == 1 or angles_hand2 is None:
            # Format: "v0,v1,v2,v3,v4\n"
            data_string = "{},{},{},{},{}\n".format(*angles_hand1)
        else:
            # Format: "v0,v1,v2,v3,v4,v5,v6,v7,v8,v9\n"
            data_string = "{},{},{},{},{},{},{},{},{},{}\n".format(
                *angles_hand1, *angles_hand2
            )

        print("Sende:", data_string.strip())
        esp32.write(data_string.encode('utf-8'))
        last_send_time = current_time

    cv2.imshow("RoboHand Kamera-Modus", frame)

    taste = cv2.waitKey(1) & 0xFF
    if taste == ord('q'):
        break
    elif taste == ord('1'):
        NUM_HAENDE = 1
        hand_detector.close()
        hand_detector = mp_hands.Hands(
            max_num_hands=1,
            min_detection_confidence=0.7,
            min_tracking_confidence=0.5
        )
        print("Umgeschaltet auf: 1 Hand")
    elif taste == ord('2'):
        NUM_HAENDE = 2
        hand_detector.close()
        hand_detector = mp_hands.Hands(
            max_num_hands=2,
            min_detection_confidence=0.7,
            min_tracking_confidence=0.5
        )
        print("Umgeschaltet auf: 2 Hände")

# ---- Aufräumen ----
cap.release()
cv2.destroyAllWindows()
hand_detector.close()
esp32.close()
print("Programm beendet!")
