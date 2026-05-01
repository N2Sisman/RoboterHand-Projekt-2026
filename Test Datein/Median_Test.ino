#include "MedianFilter.h"
#include <Wire.h>
#include "Freenove_4WD_Car_For_ESP32.h"

//            von 5 bis 15, startwert
MedianFilter flexFilter(15, 2980);

const int flex = 35;
int ServoWert;

void setup() {
  Serial.begin(115200);
  pinMode(flex, INPUT);
  PCA9685_Setup();
}

void loop() {
  int roherWert = analogRead(flex);
  int saubererWert = flexFilter.in(roherWert);

  Serial.printf("Roh: %d, Gefiltert: %d\n", roherWert, saubererWert);

  ServoWert = map(saubererWert, 2970, 2990, 75, 130);
  ServoWert = constrain(ServoWert, 75, 130);

  Servo_Angle(1, ServoWert);

  delay(70);
}
