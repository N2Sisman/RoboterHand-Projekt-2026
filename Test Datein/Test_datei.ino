/**********************************************************************
  Product     : Freenove 4WD Car for ESP32
  Description : use servo.
  Auther      : www.freenove.com
  Modification: 2024/08/12


 Alles mit Website wurde von der KI programmiert.
**********************************************************************/
#include <Wire.h>
#include "Freenove_4WD_Car_For_ESP32.h"
//Wlan Libs
#include <WiFi.h>
#include <WebServer.h>


/*************Flex-Sensor Initialisierung************/
const int flexPin_Pointer = A7;  //flex sensor ist da angebunden pin 26
/****************************************************/


/************Mittelwert berechnung *******************/
int counter = 0;
int lastThree[3] = { 90, 90, 90 };
/*****************************************************/

/***************Servo************/
int ServoWert;
/*******************************/

/*************Weserver***********/
// WLAN-Daten
const char* ssid = "Nisa";
const char* password = "12345678";

// Erstellt ein Webserver-Objekt auf Port 80 (Standart für HTTP)
WebServer server(80);
/**********************************/





/******************Funktionen für Website*********************/
//Funktion die aufgerufen wird wenn jemand die Startseite aufruft
void handleRoot() {
  //HTML-Code für die Website
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Freenove Webserver</title></head>";
  html += "<body style='font-family: Arial; text-align: center; margin-top: 50px;'>";
  html += "<h1>Freenove Car Kamera-Steuerung \U0001F697</h1>";
  html += "<p>Klicke auf einen Button, um den Servo zu bewegen:</p><br>";
  
  // Button 1: Sendet den Wert 50 (z.B. nach links schauen)
  html += "<a href='/stellServo?winkel=50' style='padding: 15px 32px; font-size: 16px; background-color: #4CAF50; color: white; text-decoration: none; border-radius: 8px;'>Kamera auf 50 Grad</a>";
  
  html += "<br><br><br>"; 
  
  // Button 2: Sendet den Wert 90 (geradeaus)
  html += "<a href='/stellServo?winkel=90' style='padding: 15px 32px; font-size: 16px; background-color: #f44336; color: white; text-decoration: none; border-radius: 8px;'>Kamera geradeaus (90 Grad)</a>";
  
  html += "</body></html>";
  
  // Sende den Code an den Browser (Status 200 = OK)
  server.send(200, "text/html", html);
}

// Diese Funktion wird aufgerufen, wenn eine unbekannte Seite aufgerufen wird
void handleNotFound() {
  server.send(404, "text/plain", "Fehler 404: Seite nicht gefunden.");
}

// Funktion für button werte an Servo senden
void handleServo() {
  // sucht in der url nach dem wort winkel und schnappt sich die Zahl dahinter
  if (server.hasArg("winkel")) {
    // der wert wird unter winkelZahl gespeichert
    int VServo_Pointer = server.arg("winkel").toInt();  // .toInt() verwandelt den text zu int
    
    // FREENOVE BEFEHL: Steuert den Zeige finger
    Servo_Angle(1, VServo_Pointer);
    
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(400, "text/plain", "Fehler: Kein Winkel angegeben.");
  }
}



/*******************************************************************/

void setup() {
  Serial.begin(115200);
  delay(10);

  PCA9685_Setup();          //Initializes the chip that controls the motor
  pinMode(flexPin_Pointer, INPUT);  // FlexSensor


  /********************Webserver****************/
  // Verbinde mit dem WLAN
  Serial.println("\nVerbinde mit WLAN...");
  WiFi.begin(ssid, password);

  // Warte, bis die Verbindung hergestellt ist
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }

  // Erfolgreich verbunden, IP-Adresse ausgeben
  Serial.println("\nErfolgreich verbunden!");
  Serial.print("IP-Adresse deines ESP32: ");
  Serial.println(WiFi.localIP());

  // Lege fest, welche Funktion bei welcher URL aufgerufen wird
  server.on("/", handleRoot);         // Startseite
  server.onNotFound(handleNotFound);  // Unbekannte Seiten

  // Starte den Server
  server.begin();
  Serial.println("Webserver gestartet.");
  /***************************************************/

  // Servo_Angle(0, 0);    //Set servo 1 Angle
  // Servo_Angle(1, 180);  //Set servo 2 Angle
  delay(1000);
}

void loop() {
  
  /*****************Website**********************/
  // Prüfe kontinuierlich, ob jemand die Website besuchen möchte
  server.handleClient();
  /**********************************************/
  Manuell_Serial();
  delay(50);
}

void Mediabewegung() {

}

/*********Mittelwert Berechnung*********/
// Test programm wurde von Ki verbessert
void Mittelwert_Bewegung() {
  int flexValue = analogRead(flexPin_Pointer);
  Serial.print("sensor: ");
  Serial.println(flexValue);

  counter++;
  int modCounter = counter % 3;  // 0, 1 oder 2
  Serial.printf("Counter : %i\n", counter);
  Serial.printf("modCounter : %i\n", modCounter);
  delay(1999);
  // 3. neuen ServoWert im Puffer speichern
  lastThree[modCounter] = flexValue;

  // 4. Mittelwert bilden
  float average = 0;
  for (int i = 0; i < 3; i++) {
    average += lastThree[i];
  }
  average /= 3;

  ServoWert = map(average, 3180, 3200, 75, 130);
  ServoWert = constrain(ServoWert, 75, 130);



  // 6. Debug-Ausgaben
  Serial.printf("%i\n%i\n%i\n", lastThree[0], lastThree[1], lastThree[2]);
  Serial.printf("Average: %f\n", average);

  Servo_Angle(1, ServoWert);
  delay(50);
}

void AufZu() {

  Servo_Angle(0, 50);   //Set servo 1 Angle
  Servo_Angle(1, 90);   //Set servo 2 Angle
  Servo_Angle(2, 80);   //Set servo 1 Angle
  Servo_Angle(3, 125);  //Set servo 2 Angle
  Servo_Angle(4, 115);  //Set servo 1 Angle

  delay(1000);
  Servo_Angle(0, 120);  //Set servo 1 Angle
  Servo_Angle(1, 130);  //Set servo 2 Angle
  Servo_Angle(2, 115);  //Set servo 1 Angle
  Servo_Angle(3, 80);   //Set servo 2 Angle
  Servo_Angle(4, 70);   //Set servo 1 Angle
}



//Erster Test des Flex SensorpexPin_Pointer);
  Serial.print("sensor: ");
  Serial.println(flexValue);
  ServoWert = map(flexValue, 3130, 3180, 75, 130);
  ServoWert = constrain(ServoWert, 75, 130);
  Servo_Angle(1, ServoWert);
}

void Manuell_Serial() {

  if (Serial.available() > 0) {
    char Zeichen = Serial.read();
    if (Zeichen == 'q') {  // Daumen
      ServoWert = Serial.parseInt();
      ServoWert = constrain(ServoWert, 80, 120); // daumen 120 zu 80 auf
      Servo_Angle(0, ServoWert);
    }
    if (Zeichen == 'w') {  // Zeigefinger
      ServoWert = Serial.parseInt();
      ServoWert = constrain(ServoWert, 60, 120); //zeige finger 120 zu 60 auf
      Servo_Angle(1, ServoWert);
    }

    if (Zeichen == 'e') {  // Zeigefinger
      ServoWert = Serial.parseInt();
      ServoWert = constrain(ServoWert, 70, 115); //mitelfinger 120 zu 80 auf
      Servo_Angle(2, ServoWert);
    }

    if (Zeichen == 'r') {  // Ringfinger OK
      ServoWert = Serial.parseInt();
      ServoWert = constrain(ServoWert, 70, 120); //ring finger 70 zu 115 auf
      Servo_Angle(3, ServoWert);
    }

    if (Zeichen == 't') {  // Zeigefinger
      ServoWert = Serial.parseInt();
      ServoWert = constrain(ServoWert, 70, 130); //kleiner finger 70 zu 140 auf
      Servo_Angle(4, ServoWert);
    }

    Serial.println(ServoWert);
  }
}



/*******
// Servo 1 motion path; 90°- 0°- 180°- 90°
Servo_Sweep(1, 90, 0);129
Servo_Sweep(1, 0, 180);
Servo_Sweep(1, 180, 90);
delay(500);
// Servo 2 motion path; 90°- 150°- 90°
Servo_Sweep(2, 90, 150);
Servo_Sweep(2, 150, 90);

**/
