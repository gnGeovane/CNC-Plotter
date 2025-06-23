#include "halfStepper.h"

Stepper::Stepper(int stepsPerRevolution, int pin1, int pin2, int pin3, int pin4, bool halfStepMode) {
    this->stepsPerRevolution = stepsPerRevolution;
    this->stepPins[0] = pin1;
    this->stepPins[1] = pin2;
    this->stepPins[2] = pin3;
    this->stepPins[3] = pin4;
    this->currentStep = 0;
    this->stepDelay = 0;
    this->lastStepTime = 0;
    this->halfStepMode = halfStepMode;

    // Define os pinos como saída
    for (int i = 0; i < 4; i++) {
        pinMode(stepPins[i], OUTPUT);
        digitalWrite(stepPins[i], LOW);
    }
}

void Stepper::setSpeed(long rpm) {
    // Multiplica o RPM se estiver no modo meio-passo
    if (halfStepMode) {
        rpm *= 2;
    }
    this->stepDelay = 60L * 1000L * 1000L / stepsPerRevolution / rpm;
}

void Stepper::step(float steps) {
    float stepsLeft = abs(steps); // Número de passos restantes
    float direction = (steps > 0) ? 1 : -1; // Direção do movimento

    // Multiplica o número de passos se estiver no modo meio-passo
    if (halfStepMode) {
        stepsLeft *= 2;
    }

    while (stepsLeft > 0) {
        unsigned long now = micros();
        if (now - lastStepTime >= stepDelay) {
            lastStepTime = now;

            // Avança para o próximo passo
            currentStep += direction;

            // Ajusta para o intervalo correto
            int sequenceLength = halfStepMode ? 8 : 4;
            if (currentStep >= sequenceLength) currentStep = 0;
            if (currentStep < 0) currentStep = sequenceLength - 1;

            // Atualiza os pinos do motor
            for (int i = 0; i < 4; i++) {
                int state = halfStepMode ? halfStepSequence[currentStep][i] : fullStepSequence[currentStep][i];
                digitalWrite(stepPins[i], state);
            }

            stepsLeft--;
        }
    }
}
