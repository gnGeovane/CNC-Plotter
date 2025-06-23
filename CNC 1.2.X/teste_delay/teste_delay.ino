#include "stepMotor.h"

int passosPorVolta = 200;
long Delay;

StepperSR rapido(passosPorVolta, 7, 8, 9);

void setup() {

  Serial.begin(115200);

  rapido.setSpeed(200);

  //rapido.setAcel(30, 3);
  rapido.step(100, 'Y');
  rapido.Off();
}

void loop() {

  //rapido.resetAcelParameters(100);


  
  
}
