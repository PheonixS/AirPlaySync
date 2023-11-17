#include <Arduino.h>

void setup() {
  pinMode(A0, INPUT);
  Serial.begin(115200);
}

void loop() {
    int sensorValue = analogRead(A0);
    Serial.println(sensorValue);
    delay(50);
}
