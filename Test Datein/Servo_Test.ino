/**********************************************************************
Ein Servo benötigt ein PWM-Signal mit 50Hz (=20ms) und 1-2ms Pulsdauer

**********************************************************************/
#include "Freenove_4WD_Car_For_ESP32.h"
const int Servo1=13;
const int Servo2=12;
const int Servo3=14;
const int Servo4=27;
const int Servo5=26;

char eingabe;

void setup()
{
  Serial.begin(115200);  // Serielle Kommunikation starten

  // ~~~~~Daumen-Servo1
  // // // PCA9685_Setup();       //Initializes the chip that controls the motor
  // // Servo_1_Angle(0);//Set servo 1 Angle
  // // Servo_2_Angle(180);//Set servo 2 Angle
  // // delay(1000);
  // // ~~~~~Daumen-Servo1
  // ledcAttach(Servo1, 50, 12); // 50Hz, 12Bit => 0-4095
  // // // minimalwert 1ms von 20ms => 1ms/20ms * 4096 = 205
  // ledcWrite(Servo1, 205);
  // delay(2000);
  // // // mittelstellung 1,5ms => 1,5/20*4096 = 306
  // ledcWrite(Servo1, 306);
  // delay(1000);
  // // // ende 2ms/20ms * 4096 
  // ledcWrite(Servo1, 409);
  // delay(2000);




  // ~~~~~Zeigefinger-Servo2
  // ledcAttach(Servo2, 50, 12);

  // ledcWrite(Servo2, 230);
  // delay(2000);
  // ledcWrite(Servo2, 470);
  // delay(2000);


  //*************** Hand auf zu test programm*********

  ledcAttach(Servo3, 50, 12);
  ledcAttach(Servo2, 50, 12);
  ledcAttach(Servo1, 50, 12);
  ledcAttach(Servo4, 50, 12);  
  ledcAttach(Servo5, 50, 12);
  

  // alle aus
  ledcWrite(Servo5, 300);
  ledcWrite(Servo4, 300);
  ledcWrite(Servo3, 409);
  ledcWrite(Servo2, 409);
  ledcWrite(Servo1, 409);

  delay(1000);

  // ledcWrite(Servo5, 400);
  ledcWrite(Servo4, 250);
  ledcWrite(Servo3, 300);
  ledcWrite(Servo2, 300);
  ledcWrite(Servo1, 205);


}

void loop()
{
  uint16_t Wert = analogRead(A0);
  uint16_t Position = map (Wert, 0, 4095, 204, 409);
  // serielle Ausgabe der beiden Werte
  // steuerung des Servos, angezeigte Werte notieren

  delay(1000);
  if (Serial.available() > 0) {  // Prüfen, ob Daten verfügbar sind
    eingabe = Serial.read();     // Zeichen lesen

    if (eingabe == 'z') {
      Serial.println("Du hast 'p' gedrückt – mache Aktion 1");
      // Hier Code für Aktion 1 einfügen (z. B. LED an)
    }
    else if (eingabe == 'z') {
      Serial.println("Du hast 's' gedrückt – mache Aktion 2");
      // Hier Code für Aktion 2 einfügen (z. B. LED aus)
    }
  }
  // // Servo 1 motion path; 90°- 0°- 180°- 90°
  // Servo_Sweep(1, 90, 0);
  // Servo_Sweep(1, 0, 180);
  // Servo_Sweep(1, 180, 90);

  // // Servo 2 motion path; 90°- 150°- 90°
  // Servo_Sweep(2, 90, 150);
  // Servo_Sweep(2, 150, 90);
 
}
