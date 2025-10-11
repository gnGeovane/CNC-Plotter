#include "HalfStepper.h"

int pin1, pin2, pin3;

void setup() {

  Serial.begin(115200);
  Serial.print("começando");
  delay(50);

  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
}
void loop() {

  pin1 = digitalRead(A1);
  pin2 = digitalRead(A2);
  pin3 = digitalRead(A0);

  Serial.print("pino A1 (X) = ");
  Serial.println(pin1);
  Serial.print("pino A2 (Y) = ");
  Serial.println(pin2);
  Serial.print("pino A0 (bot) = ");
  Serial.println(pin3);
  delay(1000);
}
