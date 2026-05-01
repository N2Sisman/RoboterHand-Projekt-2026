// mit Hilfe von KI programmiert

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "Freenove_4WD_Car_For_ESP32.h" 

// --- WLAN-Daten ---
const char* ssid = "Nisa";
const char* password = "12345678";

WebServer server(80); 

// --- NEU: Unsere Variable für den Button ---
bool pose1 = false; 

// Variablen für die Finger-Werte
int VServo_Thumb = 0;
int VServo_Pointer = 0;
int VServo_Middle = 0;
int VServo_Ring = 0;
int VServo_Pinky = 0;



const int VServo_Thumb_AUF = 120;
const int VServo_Thumb_ZU = 50;

const int VServo_Pointer_AUF = 130;
const int VServo_Pointer_ZU = 90;

const int VServo_Middle_AUF = 115;
const int VServo_Middle_ZU = 80;

const int VServo_Ring_AUF = 80;
const int VServo_Ring_ZU = 125;

const int VServo_Pinky_AUF = 70;
const int VServo_Pinky_ZU = 115;


// Website generieren
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"; 
  html += "<title>Freenove Steuerung</title></head>";
  html += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>";
  html += "<h1>Freenove Pan-Servo \U0001F697</h1>";
  
  // Der Slider (bleibt komplett unverändert)
  html += "<p>Verschiebe den Balken, um die Kamera zu drehen:</p>";
  html += "<input type='range' id='servoSlider' min='0' max='180' step='1' value='" + String(aktuellerServoWinkel) + "' oninput='sendeServoWinkel(this.value)' style='width: 300px; height: 30px;'><br><br>";
  html += "<p>Winkel: <span id='winkelAnzeige'>" + String(aktuellerServoWinkel) + "</span>°</p><br><br>";
  
  // --- NEU: Der Button, der pose1 auslöst ---
  html += "<button onclick='aktivierePose1()' style='padding: 15px 32px; font-size: 16px; background-color: #FF9800; color: white; border: none; border-radius: 8px; cursor: pointer;'>Pose 1 aktivieren</button>";
  
  // --- JAVASCRIPT ---
  html += "<script>";
  // JS-Funktion 1: Für den Slider (bleibt gleich)
  html += "  function sendeServoWinkel(winkel) {";
  html += "    document.getElementById('winkelAnzeige').innerHTML = winkel;";
  html += "    var xhttp = new XMLHttpRequest();";
  html += "    xhttp.open('GET', '/stellServo?winkel=' + winkel, true);";
  html += "    xhttp.send();";
  html += "  }";
  
  // JS-Funktion 2: NEU für den Button
  html += "  function aktivierePose1() {";
  html += "    var xhttp = new XMLHttpRequest();";
  html += "    xhttp.open('GET', '/setPose1', true);"; // Ruft die neue URL auf
  html += "    xhttp.send();";
  html += "  }";
  html += "</script>";
  // ------------------
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Funktion für den Slider
void handleServo() {
  if (server.hasArg("winkel")) {
    int winkelZahl = server.arg("winkel").toInt(); 
    
    // Motor bewegen
    winkelZahl = constrain(winkelZahl, 90, 130);
    Servo_Angle(1, winkelZahl); // (Hier aufpassen, dass es bei dir richtig heißt, z.B. Servo_Angle(1, winkelZahl); falls deine Lib das so verlangt)
    
    aktuellerServoWinkel = winkelZahl;
    Serial.print("Neuer Slider-Winkel: ");
    Serial.println(winkelZahl);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Fehler: Kein Winkel.");
  }
}

// --- NEU: Funktion für den Button ---
void handlePose1() {
  pose1 = true; // Setzt die C++ Variable auf true!
  
  Serial.println("Button geklickt! Variable 'pose1' ist jetzt TRUE.");
  
  // Dem Browser kurz antworten, dass alles geklappt hat
  server.send(200, "text/plain", "Pose 1 gesetzt");
}

void handleNotFound() {
  server.send(404, "text/plain", "Fehler 404: Seite nicht gefunden.");
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // Freenove Setup aktivieren
  PCA9685_Setup(); 

  Serial.println("\nVerbinde mit WLAN...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nErfolgreich verbunden!");
  Serial.print("IP-Adresse deines ESP32: ");
  Serial.println(WiFi.localIP());

  // Routen festlegen
  server.on("/", handleRoot);            
  server.on("/stellServo", handleServo); // Route für den Slider
  server.on("/setPose1", handlePose1);   // NEU: Route für den Button
  server.onNotFound(handleNotFound);     

  server.begin();
  Serial.println("Webserver gestartet.");
}


void loop() {
  server.handleClient();
  
  // Wenn der Button auf der Website gedrückt wurde:
  if (pose1 == true) {
    Serial.println("Führe Pose 1 aus (Peace-Zeichen ✌️)...");
    
    // 1. Die Werte wie gewünscht setzen
    VServo_Pointer = 180;
    VServo_Middle = 180;
    VServo_Thumb = 0;
    VServo_Ring = 0;
    VServo_Pinky = 0;

    // 2. Werte zur Kontrolle im Seriellen Monitor ausgeben
    Serial.print("Pointer: "); Serial.println(VServo_Pointer);
    Serial.print("Middle: "); Serial.println(VServo_Middle);
    Serial.print("Thumb: "); Serial.println(VServo_Thumb);
    
    // 3. HIER DIE MOTOREN BEWEGEN
    // (Passe die Zahlen 0, 1, 2 etc. an die Steckplätze auf deinem Freenove-Board an, 
    // an denen die jeweiligen Finger-Servos stecken!)
  Servo_Angle(0, 50);  //Set servo 1 Angle
  Servo_Angle(1, 130);   //Set servo 2 Angle
  Servo_Angle(2, 115);   //Set servo 1 Angle
  Servo_Angle(3, 125);   //Set servo 2 Angle
  Servo_Angle(4, 115);   //Set servo 1 Angle
    
    // WICHTIG: Danach wieder auf false setzen, sonst macht er das endlos!
    pose1 = false; 
  }

}
