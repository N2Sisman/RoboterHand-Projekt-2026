/**********
Dieser code wurde mit KI erweitert und verbessert.

alle werte auf eine eigene ebene gemacht um einfacher zu überschauen und kontrollieren


=== HAND 1 ===
Servo-Werte:
Daumen:        120 (zu) → 80  (auf)
Zeigefinger:   120 (zu) → 60  (auf)
Mittelfinger:  120 (zu) → 80  (auf)
Ringfinger:     70 (zu) → 115 (auf)
Kleiner Finger: 70 (zu) → 140 (auf)

=== HAND 2 ===
Servo-Werte: (müssen angepasst werden)
Daumen:        120 (zu) → 80  (auf)
Zeigefinger:   120 (zu) → 60  (auf)
Mittelfinger:  120 (zu) → 80  (auf)
Ringfinger:     70 (zu) → 115 (auf)
Kleiner Finger: 70 (zu) → 140 (auf)
*****/

/********LIBS************/
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "Freenove_4WD_Car_For_ESP32.h"
#include "MedianFilter.h"
/************************/

/*********W-LAN DATEN*********/
const char* ssid = "Nisa";
const char* password = "12345678";
/****************************/

WebServer server(80);

/*********BETRIEBSMODI*******/
enum Mode { MODE_MANUAL,
            MODE_FLEX,
            MODE_CAMERA };
Mode aktuellerModus = MODE_MANUAL;

/*********ANZAHL HAENDE*********/
int numHaende = 1;  // 1 oder 2
/*******************************/

// ============================================================
//  HAND 1
// ============================================================
int servoPins[5] = { 0, 1, 2, 3, 4 };
int aktuelleWerte[5] = { 80, 60, 80, 115, 140 };  // Startposition = offen
int gespeichertePose[5] = { 90, 90, 90, 90, 90 };

const int servoLow[5] = { 80, 60, 80, 115, 140 };
const int servoHigh[5] = { 120, 120, 120, 70, 70 };

const int flexPins[5] = { A0, A3, A6, A7, A4 };

MedianFilter flexFilter0(15, 3200);
MedianFilter flexFilter1(15, 3170);
MedianFilter flexFilter2(15, 3170);
MedianFilter flexFilter3(15, 3170);
MedianFilter flexFilter4(15, 3040);

MedianFilter* flexFilters[5] = {
  &flexFilter0, &flexFilter1, &flexFilter2,
  &flexFilter3, &flexFilter4
};

int roheWerte[5] = { 0, 0, 0, 0, 0 };
int saubereWerte[5] = { 0, 0, 0, 0, 0 };
int lastWerte[5] = { 0, 0, 0, 0, 0 };
int wertDiff[5] = { 0, 0, 0, 0, 0 };

const int diffMin[5] = { 0, 0, 0, 0, 0 };
const int diffMax[5] = { 20, 20, 20, 20, 20 };

// ============================================================
//  HAND 2
// ============================================================
int servoPins2[5] = { 5, 6, 7, 8, 9 };  // <-- PCA9685 Kanäle anpassen
int aktuelleWerte2[5] = { 80, 60, 80, 115, 140 };
int gespeichertePose2[5] = { 90, 90, 90, 90, 90 };

const int servoLow2[5] = { 80, 60, 80, 115, 140 };
const int servoHigh2[5] = { 120, 120, 120, 70, 70 };

const int flexPins2[5] = { A5, A18, A19, A17, A16 };  // <-- analoge Pins anpassen!

MedianFilter flexFilter2_0(15, 3200);
MedianFilter flexFilter2_1(15, 3170);
MedianFilter flexFilter2_2(15, 3170);
MedianFilter flexFilter2_3(15, 3170);
MedianFilter flexFilter2_4(15, 3040);

MedianFilter* flexFilters2[5] = {
  &flexFilter2_0, &flexFilter2_1, &flexFilter2_2,
  &flexFilter2_3, &flexFilter2_4
};

int roheWerte2[5] = { 0, 0, 0, 0, 0 };
int saubereWerte2[5] = { 0, 0, 0, 0, 0 };
int lastWerte2[5] = { 0, 0, 0, 0, 0 };
int wertDiff2[5] = { 0, 0, 0, 0, 0 };
// ============================================================

int ServoWert;
int zeit;

unsigned long letzteMessung = 0;
unsigned long letzteDebug = 0;
const int MESS_INTERVALL = 50;
const int DEBUG_INTERVALL = 500;

/****************HTML************************/
void handleRoot() {
  String html = "<!DOCTYPE html><html lang='de'><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>RoboHand Controller</title>";
  html += "<style>";
  html += "body { font-family: sans-serif; text-align: center; background: #f4f4f9; padding: 20px; }";
  html += ".section { background: white; padding: 15px; border-radius: 10px; margin-bottom: 20px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
  html += "button { padding: 10px 20px; margin: 5px; font-size: 16px; cursor: pointer; border-radius: 5px; border: none; transition: 0.3s; }";
  html += ".btn-mode { background: #2196F3; color: white; }";
  html += ".btn-pose { background: #4CAF50; color: white; }";
  html += ".btn-save { background: #f44336; color: white; }";
  html += ".btn-hand { background: #9C27B0; color: white; }";
  html += ".btn-hand.active { background: #6A0080; outline: 3px solid #000; }";
  html += "input[type=range] { width: 60%; height: 20px; }";
  html += "button:disabled { background: #ccc; cursor: not-allowed; }";
  html += ".hand-block { border: 2px solid #ddd; border-radius: 8px; padding: 10px; margin-top: 10px; }";
  html += ".hand-block h4 { margin: 0 0 8px 0; color: #555; }";
  html += ".slider-row { display: flex; align-items: center; justify-content: center; margin: 6px 0; }";
  html += ".slider-label { display: inline-block; width: 130px; text-align: right; margin-right: 10px; }";
  html += "</style></head><body>";

  // --- Anzahl Hände ---
  html += "<div class='section'>";
  html += "<h3>&#x1F91A; Anzahl H&auml;nde</h3>";
  html += "<button class='btn-hand" + String(numHaende == 1 ? " active" : "") + "' id='btn1hand' onclick=\"setHands(1)\">&#x2460; 1 Hand</button>";
  html += "<button class='btn-hand" + String(numHaende == 2 ? " active" : "") + "' id='btn2hand' onclick=\"setHands(2)\">&#x2461; 2 H&auml;nde</button>";
  html += "<p>Aktiv: <b id='handText'>" + String(numHaende) + " Hand" + String(numHaende == 2 ? "&auml;nde" : "") + "</b></p>";
  html += "</div>";

  // --- Betriebsmodus ---
  html += "<div class='section'>";
  html += "<h3>Betriebsmodus</h3>";
  html += "<div style='display:flex; justify-content:center; align-items:center; flex-wrap:wrap; gap:8px;'>";
  html += "<button class='btn-mode' onclick=\"setMode('manual')\">Website-Steuerung</button>";
  html += "<button class='btn-mode' onclick=\"setMode('flex')\">Flex-Sensor Modus</button>";
  html += "<button class='btn-mode' onclick=\"setMode('camera')\">Kamera Modus</button>";
  html += "</div>";

  String statusString = "Website";
  if (aktuellerModus == MODE_FLEX) statusString = "Flex-Sensoren";
  else if (aktuellerModus == MODE_CAMERA) statusString = "Kamera Modus";

  html += "<p>Aktueller Status: <b id='statusText'>" + statusString + "</b></p>";
  html += "</div>";

  // --- Manuelle Steuerung ---
  String fingerNames[] = { "Daumen", "Zeigefinger", "Mittelfinger", "Ringfinger", "Kleiner Finger" };

  html += "<div class='section' id='manualControls'>";
  html += "<h3>Manuelle Fingersteuerung</h3>";

  // Hand 1 Schieberegler
  html += "<div class='hand-block'><h4>&#x1F91A; Hand 1</h4>";
  for (int i = 0; i < 5; i++) {
    html += "<div class='slider-row'>";
    html += "<span class='slider-label'>" + fingerNames[i] + "</span>";
    html += "<input type='range' min='0' max='180' value='" + String(aktuelleWerte[i])
            + "' onchange='sendServo(1," + String(i) + ", this.value)'>";
    html += "</div>";
  }
  html += "</div>";

  // Hand 2 Schieberegler
  html += "<div class='hand-block' id='hand2sliders'><h4>&#x1F91A; Hand 2</h4>";
  for (int i = 0; i < 5; i++) {
    html += "<div class='slider-row'>";
    html += "<span class='slider-label'>" + fingerNames[i] + "</span>";
    html += "<input type='range' min='0' max='180' value='" + String(aktuelleWerte2[i])
            + "' onchange='sendServo(2," + String(i) + ", this.value)'>";
    html += "</div>";
  }
  html += "</div>";
  html += "</div>";  // end manualControls

  // --- Posen ---
  html += "<div class='section'>";
  html += "<h3>Posen-Speicher</h3>";
  html += "<b>Hand 1:</b><br>";
  html += "<button class='btn-pose' id='p1' onclick=\"runPose(1,'peace')\">&#x270C;&#xFE0F; Peace</button>";
  html += "<button class='btn-pose' id='p2' onclick=\"runPose(1,'fist')\">&#x270A; Faust</button>";
  html += "<button class='btn-pose' id='p3' onclick=\"runPose(1,'load')\">Speicher laden</button>";
  html += "<button class='btn-save' onclick=\"runPose(1,'save')\">Aktuelle Pose speichern</button>";

  html += "<br><br><div id='pose2block'><b>Hand 2:</b><br>";
  html += "<button class='btn-pose' id='p4' onclick=\"runPose(2,'peace')\">&#x270C;&#xFE0F; Peace</button>";
  html += "<button class='btn-pose' id='p5' onclick=\"runPose(2,'fist')\">&#x270A; Faust</button>";
  html += "<button class='btn-pose' id='p6' onclick=\"runPose(2,'load')\">Speicher laden</button>";
  html += "<button class='btn-save' onclick=\"runPose(2,'save')\">Aktuelle Pose speichern</button>";
  html += "</div>";
  html += "</div>";

  // --- JavaScript ---
  html += "<script>";
  html += "var isManual = " + String(aktuellerModus == MODE_MANUAL ? "true" : "false") + ";";
  html += "var numHaende = " + String(numHaende) + ";";

  html += "function updateUI() {";
  html += "  const manualIds = ['p1','p2','p3','p4','p5','p6'];";
  html += "  manualIds.forEach(id => {";
  html += "    const el = document.getElementById(id);";
  html += "    if(el) el.disabled = !isManual;";
  html += "  });";
  html += "  document.getElementById('manualControls').style.opacity = isManual ? '1' : '0.3';";
  html += "  const h2 = document.getElementById('hand2sliders');";
  html += "  const p2 = document.getElementById('pose2block');";
  html += "  const show2 = (numHaende === 2);";
  html += "  if(h2) h2.style.display = show2 ? '' : 'none';";
  html += "  if(p2) p2.style.display = show2 ? '' : 'none';";
  html += "}";

  html += "function setHands(n) {";
  html += "  numHaende = n;";
  html += "  document.getElementById('btn1hand').classList.toggle('active', n===1);";
  html += "  document.getElementById('btn2hand').classList.toggle('active', n===2);";
  html += "  document.getElementById('handText').innerHTML = n + ' Hand' + (n===2?'&auml;nde':'');";
  html += "  fetch('/setHands?val=' + n);";
  html += "  updateUI();";
  html += "}";

  html += "function setMode(m) {";
  html += "  isManual = (m === 'manual'); updateUI();";
  html += "  fetch('/setMode?val=' + m);";
  html += "  let text = 'Website';";
  html += "  if(m === 'flex')   text = 'Flex-Sensoren';";
  html += "  if(m === 'camera') text = 'Kamera Modus';";
  html += "  document.getElementById('statusText').innerText = text;";
  html += "}";

  html += "function sendServo(hand, id, val) {";
  html += "  if(isManual) fetch('/setServo?hand=' + hand + '&id=' + id + '&val=' + val);";
  html += "}";

  html += "function runPose(hand, p) { fetch('/setPose?hand=' + hand + '&type=' + p); }";
  html += "window.onload = updateUI;";
  html += "</script></body></html>";

  server.send(200, "text/html", html);
}

/****** Handler: Anzahl Hände ******/
void handleSetHands() {
  int val = server.arg("val").toInt();
  if (val == 1 || val == 2) {
    numHaende = val;
    if (aktuellerModus == MODE_FLEX) {
      for (int i = 0; i < 5; i++) {
        wertDiff[i] = 0;
        lastWerte[i] = 0;
        wertDiff2[i] = 0;
        lastWerte2[i] = 0;
      }
    }
    Serial.println("Anzahl Haende: " + String(numHaende));
  }
  server.send(200, "text/plain", "OK");
}

void handleSetMode() {
  String val = server.arg("val");
  if (val == "manual") aktuellerModus = MODE_MANUAL;
  else if (val == "flex") aktuellerModus = MODE_FLEX;
  else if (val == "camera") aktuellerModus = MODE_CAMERA;

  if (val == "flex") {
    for (int i = 0; i < 5; i++) {
      wertDiff[i] = 0;
      lastWerte[i] = 0;
      wertDiff2[i] = 0;
      lastWerte2[i] = 0;
    }
  }
  Serial.println("Modus gewechselt auf: " + val);
  server.send(200, "text/plain", "OK");
}

void handleSetServo() {
  if (aktuellerModus != MODE_MANUAL) {
    server.send(200, "text/plain", "OK");
    return;
  }

  int hand = server.arg("hand").toInt();
  int id = server.arg("id").toInt();
  int val = server.arg("val").toInt();

  if (hand == 2 && numHaende < 2) {
    server.send(200, "text/plain", "OK");
    return;
  }

  int mapped = val;
  if (hand == 1) {
    aktuelleWerte[id] = val;
    if (id == 0) mapped = map(val, 0, 180, 80, 120);
    if (id == 1) mapped = map(val, 0, 180, 60, 120);
    if (id == 2) mapped = map(val, 0, 180, 80, 120);
    if (id == 3) mapped = map(val, 0, 180, 115, 70);
    if (id == 4) mapped = map(val, 0, 180, 140, 70);
    Servo_Angle(servoPins[id], mapped);
  } else {
    aktuelleWerte2[id] = val;
    if (id == 0) mapped = map(val, 0, 180, 80, 120);
    if (id == 1) mapped = map(val, 0, 180, 60, 120);
    if (id == 2) mapped = map(val, 0, 180, 80, 120);
    if (id == 3) mapped = map(val, 0, 180, 115, 70);
    if (id == 4) mapped = map(val, 0, 180, 140, 70);
    Servo_Angle(servoPins2[id], mapped);
  }
  server.send(200, "text/plain", "OK");
}

void handleSetPose() {
  int hand = server.arg("hand").toInt();
  String type = server.arg("type");

  if (hand == 2 && numHaende < 2) {
    server.send(200, "text/plain", "OK");
    return;
  }

  int* werte = (hand == 1) ? aktuelleWerte : aktuelleWerte2;
  int* pins = (hand == 1) ? servoPins : servoPins2;
  int* gespeic = (hand == 1) ? gespeichertePose : gespeichertePose2;

  if (type == "save") {
    for (int i = 0; i < 5; i++) gespeic[i] = werte[i];
    Serial.println("Pose gespeichert (Hand " + String(hand) + ")!");
  } else if (aktuellerModus == MODE_MANUAL) {
    if (type == "peace") {
      int p[] = { 120, 60, 80, 115, 140 };
      for (int i = 0; i < 5; i++) {
        werte[i] = p[i];
        Servo_Angle(pins[i], p[i]);
      }
    } else if (type == "fist") {
      int p[] = { 120, 120, 120, 70, 70 };
      for (int i = 0; i < 5; i++) {
        werte[i] = p[i];
        Servo_Angle(pins[i], p[i]);
      }
    } else if (type == "load") {
      for (int i = 0; i < 5; i++) {
        werte[i] = gespeic[i];
        Servo_Angle(pins[i], werte[i]);
      }
    }
  }
  server.send(200, "text/plain", "OK");
}

void leseFlex() {
  for (int i = 0; i < 5; i++) {
    roheWerte[i] = analogRead(flexPins[i]);
    saubereWerte[i] = flexFilters[i]->in(roheWerte[i]);
  }
  if (numHaende == 2) {
    for (int i = 0; i < 5; i++) {
      roheWerte2[i] = analogRead(flexPins2[i]);
      saubereWerte2[i] = flexFilters2[i]->in(roheWerte2[i]);
    }
  }
}

void debugAusgabe() {
  if (millis() - letzteDebug < DEBUG_INTERVALL) return;
  letzteDebug = millis();

  String fingerKuerzel[] = { "DAU", "ZEI", "MIT", "RIN", "KLE" };
  Serial.println("=== Flex-Sensor Messwerte (Diff-Modus) ===");
  for (int i = 0; i < 5; i++) {
    Serial.print("[H1-");
    Serial.print(fingerKuerzel[i]);
    Serial.print("] Roh: ");
    Serial.print(roheWerte[i]);
    Serial.print("  | Gef: ");
    Serial.print(saubereWerte[i]);
    Serial.print("  | Diff: ");
    Serial.print(wertDiff[i]);
    Serial.print("  | Servo: ");
    Serial.println(aktuelleWerte[i]);
  }
  if (numHaende == 2) {
    for (int i = 0; i < 5; i++) {
      Serial.print("[H2-");
      Serial.print(fingerKuerzel[i]);
      Serial.print("] Roh: ");
      Serial.print(roheWerte2[i]);
      Serial.print("  | Gef: ");
      Serial.print(saubereWerte2[i]);
      Serial.print("  | Diff: ");
      Serial.print(wertDiff2[i]);
      Serial.print("  | Servo: ");
      Serial.println(aktuelleWerte2[i]);
    }
  }
  Serial.println("------------------------------------------");
}

void setup() {
  Serial.begin(115200);
  PCA9685_Setup();

  for (int i = 0; i < 5; i++) {
    pinMode(flexPins[i], INPUT);
    if (numHaende == 2) pinMode(flexPins2[i], INPUT);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\ndu bist Verbunden! \nWebsite IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/setHands", handleSetHands);
  server.on("/setMode", handleSetMode);
  server.on("/setServo", handleSetServo);
  server.on("/setPose", handleSetPose);
  server.begin();
}

void loop() {
  server.handleClient();

  // =====================
  // 1. FLEX-SENSOR LOGIK
  // =====================
  if (aktuellerModus == MODE_FLEX) {
    if (millis() - letzteMessung >= MESS_INTERVALL) {
      letzteMessung = millis();

      leseFlex();

      // Hand 1
      for (int i = 0; i < 5; i++) {
        int diff = saubereWerte[i] - lastWerte[i];
        lastWerte[i] = saubereWerte[i];
        wertDiff[i] = constrain(wertDiff[i] + diff, diffMin[i], diffMax[i]);
        int servoZiel = map(wertDiff[i], diffMin[i], diffMax[i], servoLow[i], servoHigh[i]);
        aktuelleWerte[i] = servoZiel;
        Servo_Angle(servoPins[i], servoZiel);
      }

      // Hand 2 – nur wenn ausgewählt
      if (numHaende == 2) {
        for (int i = 0; i < 5; i++) {
          int diff = saubereWerte2[i] - lastWerte2[i];
          lastWerte2[i] = saubereWerte2[i];
          wertDiff2[i] = constrain(wertDiff2[i] + diff, diffMin[i], diffMax[i]);
          int servoZiel = map(wertDiff2[i], diffMin[i], diffMax[i], servoLow2[i], servoHigh2[i]);
          aktuelleWerte2[i] = servoZiel;
          Servo_Angle(servoPins2[i], servoZiel);
        }
      }

      debugAusgabe();
    }
  }

  // =====================
  // 2. KAMERA MODUS LOGIK
  // =====================
  else if (aktuellerModus == MODE_CAMERA) {
    if (Serial.available() > 0) {
      String data = Serial.readStringUntil('\n');
      data.trim();

      // Format 1 Hand:  "v0,v1,v2,v3,v4"
      // Format 2 Hände: "v0,v1,v2,v3,v4,v5,v6,v7,v8,v9"
      int commas[9];
      int count = 0;
      for (int c = 0; c < data.length() && count < 9; c++) {
        if (data[c] == ',') commas[count++] = c;
      }

      if (count >= 4) {
        // Hand 1
        int v[5];
        v[0] = data.substring(0, commas[0]).toInt();
        v[1] = data.substring(commas[0] + 1, commas[1]).toInt();
        v[2] = data.substring(commas[1] + 1, commas[2]).toInt();
        v[3] = data.substring(commas[2] + 1, commas[3]).toInt();
        v[4] = (count >= 5)
                 ? data.substring(commas[3] + 1, commas[4]).toInt()
                 : data.substring(commas[3] + 1).toInt();

        v[0] = map(v[0], 180, 0, 120, 80);
        v[1] = map(v[1], 0, 180, 120, 60);
        v[2] = map(v[2], 0, 180, 120, 80);
        v[3] = map(v[3], 0, 180, 70, 115);
        v[4] = map(v[4], 0, 180, 70, 140);

        for (int i = 0; i < 5; i++) {
          aktuelleWerte[i] = v[i];
          Servo_Angle(servoPins[i], v[i]);
        }

        // Hand 2 – nur wenn ausgewählt und 10 Werte gesendet wurden
        if (numHaende == 2 && count >= 9) {
          int w[5];
          w[0] = data.substring(commas[4] + 1, commas[5]).toInt();
          w[1] = data.substring(commas[5] + 1, commas[6]).toInt();
          w[2] = data.substring(commas[6] + 1, commas[7]).toInt();
          w[3] = data.substring(commas[7] + 1, commas[8]).toInt();
          w[4] = data.substring(commas[8] + 1).toInt();

          w[0] = map(w[0], 0, 180, 120, 80);
          w[1] = map(w[1], 0, 180, 120, 60);
          w[2] = map(w[2], 0, 180, 120, 80);
          w[3] = map(w[3], 0, 180, 70, 115);
          w[4] = map(w[4], 0, 180, 70, 140);

          for (int i = 0; i < 5; i++) {
            aktuelleWerte2[i] = w[i];
            Servo_Angle(servoPins2[i], w[i]);
          }
        }
      }
    }
  }
}
