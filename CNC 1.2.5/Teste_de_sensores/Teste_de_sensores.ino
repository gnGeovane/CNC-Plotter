#include "HalfStepper.h"

int pin1, pin2, pin3;

int leitura;
int leituraAnalogica;
int PassosPorVolta = 200;/*
Stepper StepperX(PassosPorVolta, A0, 10, 10, A0);
Stepper StepperY(PassosPorVolta, 6, 7, 8, 9);
Stepper StepperZ(PassosPorVolta, 10, 9, 10, A0);

Stepper StepperXlevantado(PassosPorVolta, A0, 10, 10, A0);
Stepper StepperYlevantado(PassosPorVolta, 6, 7, 8, 9);

*/
void setup() {

  Serial.begin(115200);
  Serial.print("começando");
  delay(50);

  int leitura;
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
