#ifndef STEPPER_H
#define STEPPER_H

#include <Arduino.h>

class Stepper {
private:
    int stepsPerRevolution;
    int stepPins[4];
    int currentStep;
    long stepDelay; // Delay entre passos
    unsigned long lastStepTime; // Momento do último passo
    bool halfStepMode; // Ativa/desativa modo meio-passo

    // Sequências para modo inteiro e meio-passo
    const int fullStepSequence[4][4] = {
        {0, 1, 1, 0},
        {0, 1, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 1, 0}
    };
    const int halfStepSequence[8][4] = {
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 1},
        {0, 0, 0, 1},
        {1, 0, 0, 1},
        {1, 0, 0, 0},
        {1, 0, 1, 0},
        {0, 0, 1, 0}
    };

public:
    // Definição dos pinos do motor
    Stepper(int stepsPerRevolution, int pin1, int pin2, int pin3, int pin4, bool halfStepMode);

    // Define a velocidade do motor (RPM)
    void setSpeed(long rpm);

    // Realiza os passos
    void step(int steps);
};

#endif // STEPPER_H
