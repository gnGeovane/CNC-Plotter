/*
=====================================================================
  Biblioteca para Controle de Motor de Passo Unipolar com Full/Half-Step
=====================================================================
  
  Autor: Geovane A. Silva
  Data de Criação: Fevereiro de 2025
  Versão: 1.0
  
  Licença:
  Este código é distribuído sob a licença MIT, permitindo uso livre
  para qualquer finalidade, desde que seja mantida a atribuição ao autor.

 */


class Stepper {

private:

  //============================== Variáveis =======================================
  
  int StepperPins[4];
  int passosPorVolta;
  int currentStep;           // grava em qual passo da matriz o motor parou
  long stepDelay;            // Delay entre passos
  unsigned long lastTime;    // Marca de tempo do último passo
  unsigned long actualTime;  // Marca o tempo atual

  //===============================================================================
  /*
  // Sequência para meio-passo
  const int halfStepSequence[8][4] = {
    { 0, 1, 1, 0 },
    { 0, 1, 0, 0 },
    { 0, 1, 0, 1 },
    { 0, 0, 0, 1 },
    { 1, 0, 0, 1 },
    { 1, 0, 0, 0 },
    { 1, 0, 1, 0 },
    { 0, 0, 1, 0 }
  }; */

  // Sequência para modo inteiro
  /*const int fullStepSequence[4][4] = {
    { 0, 1, 1, 0 },
    { 0, 1, 0, 1 },
    { 1, 0, 0, 1 },
    { 1, 0, 1, 0 }
  };
  */
  // Sequência para modo de meio-passo ordenado
  const int halfStepSequence[8][4] = {
    { 1, 0, 0, 0 },
    { 1, 1, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 1, 1 },
    { 0, 0, 0, 1 },
    { 1, 0, 0, 1 }
  };
  const int fullStepSequence[4][4] = {
    { 1, 1, 0, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 1, 1 },
    { 1, 0, 0, 1 }
  };

public:

Stepper(int passosPorVolta, int pin1, int pin2, int pin3, int pin4) {

    this->passosPorVolta = passosPorVolta;
    this->StepperPins[0] = pin1;
    this->StepperPins[1] = pin2;
    this->StepperPins[2] = pin3;
    this->StepperPins[3] = pin4;

    for (int i = 0; i < 4; i++) {
      pinMode(this->StepperPins[i], OUTPUT);
      digitalWrite(this->StepperPins[i], LOW);
    }
}

void setSpeed(long rpm) {

    stepDelay = (60000000L / passosPorVolta / rpm) / 2;
}

void setSpeedFull(long rpm) {

    stepDelay = (60000000L / passosPorVolta / rpm);
}

void step(int steps) {

    int stepsLeft = abs(steps);
    int direcao = (steps > 0) ? 1 : -1;

    while (stepsLeft > 0) {

      unsigned long actualTime = micros();
      if (actualTime - lastTime >= stepDelay) {

        lastTime = actualTime;
        currentStep += direcao;

        if (currentStep >= 8) currentStep = 0;
        if (currentStep < 0) currentStep = 7;

        for (int i = 0; i < 4; i++) {
          digitalWrite(StepperPins[i], halfStepSequence[currentStep][i]);
        }

        stepsLeft--;
      }
    }
}

void stepFull(int steps) {

    int stepsLeft = abs(steps);
    int direcao = (steps > 0) ? 1 : -1;

    while (stepsLeft > 0) {

      unsigned long actualTime = micros();
      if (actualTime - lastTime >= stepDelay) {

        lastTime = actualTime;
        currentStep += direcao;

        if (currentStep >= 4) currentStep = 0;
        if (currentStep < 0) currentStep = 3;

        for (int i = 0; i < 4; i++) {
          digitalWrite(StepperPins[i], fullStepSequence[currentStep][i]);
        }

        stepsLeft--;
      }
    }
}

void Off() {  // Função usada para manter os motores desligados durante delays pelo código main.

    for(int i = 0; i < 4; i++) {
      digitalWrite(this->StepperPins[i], LOW);
    }
}

};  // End Public




