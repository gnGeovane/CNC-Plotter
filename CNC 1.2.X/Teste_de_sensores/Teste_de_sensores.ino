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
  pinMode(10, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
/*
  StepperX.setSpeed(A00);
  StepperY.setSpeed(105);
  StepperZ.setSpeed(170);

  StepperXlevantado.setSpeed(200);
  StepperYlevantado.setSpeed(195);*/
}

void loop() {

  pin1 = digitalRead(9);
  pin2 = digitalRead(10);
  pin3 = digitalRead(A0);

  Serial.print("pino 9 (X) = ");
  Serial.println(pin1);
  Serial.print("pino 10 (Y) = ");
  Serial.println(pin2);
  Serial.print("pino A0 (bot) = ");
  Serial.println(pin3);
  delay(1000);
  /*
  for(int i = 0; i < 100; i++) {
  StepperXlevantado.step(1);
  leitura = analogRead(9);
  if(leitura >= 937 && leitura <= 950){
          Serial.println("Dois sensores");
      }
      if (leitura >= 650 && leitura <= 750) {
          Serial.println("eixo X");
      }
      if (leitura >= 920 && leitura <= 935) {
        Serial.println("eixo Y");
      } else Serial.println("sem deteccao...");
  }
  StepperX.Off();
  delay(500);
  */
  /*
  if(digitalRead(10) == 1) {
    Serial.println("Pino 9 = 1");
    delay(500);
  }
  
  if(digitalRead(10) == 0) {
    Serial.println("Pino 9 = 0");
    delay(500);
  }

  if(digitalRead(9) == 1) {

    leitura = digitalRead(9);
    Serial.print("Leitura Digital: ");
    Serial.println(leitura);

    leituraAnalogica = analogRead(9);

    Serial.print("Leitura analogica: ");
    Serial.println(leituraAnalogica);
    delay(500);
    
  }
*/
}
