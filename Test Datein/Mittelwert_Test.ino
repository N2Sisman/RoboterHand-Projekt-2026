// Das ist der originelle Test code vom Mittelwert.
// Wurde dann von der KI korrigiert und in den code eingebaut.

int counter = 0;
int lastThree[3] = { 3000, 3000, 3000 };
int sensorErgebnis = 3100;
const int flexPin = A5;

void setup() {
  Serial.begin(115200);
}

void loop() {
  // sensor wird gelesen

  int flexValue;
  flexValue = analogRead(flexPin);
  Serial.print("sensor: ");
  Serial.println(flexValue);

  ServoWert = map(flexValue, 3120, 3140, 75, 130);
  ServoWert = constrain(ServoWert, 75, 130);


  counter++;
  int modCounter = counter % 3;  // entweder 0, 1, 2

  switch (modCounter) {
    case 0:
      ServoWert = ++ServoWert;
      lastThree[0] = ServoWert;
      break;

    case 1:
      ServoWert = ++ServoWert;
      lastThree[1] = ServoWert;
      break;

    case 2:
      ServoWert = ++ServoWert;
      lastThree[2] = ServoWert;
      break;
  }

  float average = 0;
  for (int i = 0; i < 3; i++) {
    average += lastThree[i];
  }
  average = average / 3;


  Servo_Angle(1, average);
  Serial.printf("%i\n%i\n%i\n ", lastThree[0], lastThree[1], lastThree[2]);
  Serial.printf("Average: %f \n", average);
}
```
