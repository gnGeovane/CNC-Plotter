/*
=====================================================================
  Biblioteca para Controle de Motor de Passo Unipolar com Half-Step
=====================================================================
  
  Autor: Geovane A. Silva
  Data de Criação: Fevereiro de 2025
  Versão: 1.2
  
  Licença:
  Este código é distribuído sob a licença MIT, permitindo uso livre
  para qualquer finalidade, desde que seja mantida a atribuição ao autor.

*/

#ifndef STEPPER_H
#define STEPPER_H

class Stepper {

private:

  //============================== Variáveis =======================================
  
  int StepperPins[4];
  int passosPorVolta;
  int currentStep;           // grava em qual passo da matriz o motor parou
  
  long stepDelay;            // Delay entre passos
  unsigned long lastTime;    // Marca de tempo do último passo
  
  int acel;
  int limitStep;
  int limitStepMin;
  long speed;
  int midStep;
  long maxSpeed;
  long minSpeed;
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

long Delay() {
  return stepDelay;
}

void setAcel(int limitStep, int mult) {
  this-> acel = (stepDelay * mult) / limitStep;
  this-> limitStep = limitStep;
  this-> minSpeed = stepDelay * mult;
  this-> maxSpeed = stepDelay;
}

void resetAcelParameters(int n) {
  this-> speed = minSpeed;
  this-> limitStepMin = n - limitStep;
  this-> midStep = n / 2;
}

void aceleration(int i) {  
  if(i < midStep && speed > maxSpeed && i <= limitStep) {
    speed -= acel;
    delayMicroseconds(speed);
  }
  else if(i > midStep && speed < minSpeed && i >= limitStepMin) {
    speed += acel;
    delayMicroseconds(speed);
  }
  else delayMicroseconds(speed);
}

};  // End class


class StepperSR {

private:

/*
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
*/

const byte halfStepSequence[16] = {
// Motor 1 (Qa - Qd)
  0b10000000,
  0b11000000,
  0b01000000,
  0b01100000,
  0b00100000,
  0b00110000,
  0b00010000,
  0b10010000,
// Motor 2 (Qe - Qh)
  0b00001000,
  0b00001100,
  0b00000100,
  0b00000110,
  0b00000010,
  0b00000011,
  0b00000001,
  0b00001001
};

const byte halfStepSequenceBresenham[9] = {
  0b1000,
  0b1100,
  0b0100,
  0b0110,
  0b0010,
  0b0011,
  0b0001,
  0b1001,
  0b0000
};

//============================== Variáveis =======================================
int currentStepX;
int currentStepY;
int oneCurrentStepX;
int oneCurrentStepY;
unsigned long stepDelay;
int passosPorVolta;

int DATA;
int CLOCK;
int LATCH;

int acel;
int limitStep;
int maxStep;
long maxSpeed;
long minSpeed;

unsigned long lastTime;

int mode;
int mult;

int teste1 = 0;
int teste2 = 0;
//===============================================================================

public:

StepperSR(int passosPorVolta, int CLOCK, int LATCH, int DATA) {
  this->passosPorVolta = passosPorVolta;
  this->DATA = DATA;
  this->CLOCK = CLOCK;
  this->LATCH = LATCH;

  pinMode(DATA, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  digitalWrite(DATA, LOW);
  digitalWrite(CLOCK, LOW); 
  digitalWrite(LATCH, LOW); 
}

void setSpeed(long rpm) {
    stepDelay = (60000000L / passosPorVolta / rpm) / 2;
}

void step(int steps, char motor) {

  int stepsLeft = abs(steps);
  int direcao = (steps > 0) ? 1 : -1;
  unsigned long startTime;

  while (stepsLeft > 0) {
    startTime = micros();

    if (startTime - lastTime >= stepDelay) {

      if (motor == 'X' || motor == 'x') {
        currentStepX += direcao;
        if (currentStepX >= 8) currentStepX = 0;
        if (currentStepX < 0) currentStepX = 7;
        shiftOut(DATA, CLOCK, LSBFIRST, halfStepSequence[currentStepX]);
      }
      if (motor == 'Y' || motor == 'y') {
        currentStepY += direcao;
        if (currentStepY >= 16) currentStepY = 8;
        if (currentStepY < 8) currentStepY = 15;
        shiftOut(DATA, CLOCK, LSBFIRST, halfStepSequence[currentStepY]);
      }
      digitalWrite(LATCH, HIGH);
      digitalWrite(LATCH, LOW);
      stepsLeft--;
      lastTime = micros();
    }
  }
}

void oneStep(bool canGoX, bool canGoY, int dirX, int dirY) {

  unsigned long startTime = micros();
  byte nibbleX;
  byte nibbleY;
  byte combined;

  while (startTime - lastTime <= stepDelay) {startTime = micros();}

  if (canGoX) {
    oneCurrentStepX += dirX;
    if (oneCurrentStepX >= 8) oneCurrentStepX = 0;
    if (oneCurrentStepX < 0) oneCurrentStepX = 7;
    nibbleX = halfStepSequenceBresenham[oneCurrentStepX];
  }
  else nibbleX = halfStepSequenceBresenham[oneCurrentStepX];

  if (canGoY) {
    oneCurrentStepY += dirY;
    if (oneCurrentStepY >= 8) oneCurrentStepY = 0;
    if (oneCurrentStepY < 0) oneCurrentStepY = 7;
    nibbleY = halfStepSequenceBresenham[oneCurrentStepY];
  }
  else nibbleY = halfStepSequenceBresenham[oneCurrentStepY];

  combined = (nibbleX << 4) | nibbleY;
  shiftOut(DATA, CLOCK, LSBFIRST, combined);
  digitalWrite(LATCH, HIGH);
  digitalWrite(LATCH, LOW);
  lastTime = micros();
}

void Off() {  // Função usada para manter os motores desligados durante delays pelo código main.
  shiftOut(DATA, CLOCK, LSBFIRST, 0x00);
  digitalWrite(LATCH, HIGH);
  digitalWrite(LATCH, LOW);
}

long Delay() {
  return stepDelay;
}

void setAcel(int limitStep, int mult) {
  this->limitStep = limitStep;          // Número de passos em que ocorre a aceleração/desaceleração
  this->minSpeed = stepDelay * mult;       // Delay inicial (maior delay = movimento mais lento)
  this->maxSpeed = stepDelay;             // Delay mínimo (velocidade máxima)
  this->mult = mult;
}

void resetAcelParameters(int n, int mode) {
  
  maxStep = n;
  this->mode = mode;
}

void aceleration(int currentStep) {

  int A = limitStep;  
  long delayValue;
  
   if (maxStep < 30) {
    int A = maxStep;  
    delayValue = minSpeed - ((minSpeed - maxSpeed) * currentStep) / A;
  }
  else if (currentStep < A && (mode == 3 || mode == 0)) {
    // Fase de aceleração: diminui linearmente o delay de minSpeed até maxSpeed
    delayValue = minSpeed - ((minSpeed - maxSpeed) * currentStep) / A;
  }
  else if (currentStep >= maxStep - A && (mode == 5 || mode == 0)) {
    // Fase de desaceleração: aumenta linearmente o delay de maxSpeed até minSpeed
    delayValue = maxSpeed + ((minSpeed - maxSpeed) * (currentStep - (maxStep - A))) / A;
  }
  else { // Para ativar apenas a velocidade de cruzeiro, use mode = 4;
    // Velocidade de cruzeiro: delay constante em maxSpeed
    delayValue = maxSpeed;
  }
  delayMicroseconds(delayValue);
}

}; // End class

#endif


/*
const byte halfStepSequence[16] = {
// Motor 1 (Qa - Qd)
  0b10000000,
  0b11000000,
  0b01000000,
  0b01100000,
  0b00100000,
  0b00110000,
  0b00010000,
  0b10010000,
// Motor 2 (Qe - Qh)
  0b00001000,
  0b00001100,
  0b00000100,
  0b00000110,
  0b00000010,
  0b00000011,
  0b00000001,
  0b00001001
};


void step(int steps, char motor) {

int stepsLeft = abs(steps);
int direcao = (steps > 0) ? 1 : -1;

  while (stepsLeft > 0) {
    unsigned long startTime = micros();

    if (startTime - lastTime >= stepDelay) {
      
      if (motor == 'X' || motor == 'x') {
        currentStepX += direcao;
        if (currentStepX >= 8) currentStepX = 0;
        if (currentStepX < 0) currentStepX = 7;
      }
      if (motor == 'Y' || motor == 'y') {
        currentStepY += direcao;
        if (currentStepY >= 8) currentStepY = 0;
        if (currentStepY < 0) currentStepY = 7;
      }

      for(int i = 3; i >= 0; i--) {
        digitalWrite(CLOCK, 0);
        digitalWrite(DATA, halfStepSequence[currentStepY][i]); 
        digitalWrite(CLOCK, 1);
      }
      for(int i = 3; i >= 0; i--) {
        digitalWrite(CLOCK, 0);
        digitalWrite(DATA, halfStepSequence[currentStepX][i]); 
        digitalWrite(CLOCK, 1);
      }
    }

    digitalWrite(LATCH, HIGH);
    digitalWrite(LATCH, LOW);
    stepsLeft--;
    lastTime = micros();
  }
}
*/

