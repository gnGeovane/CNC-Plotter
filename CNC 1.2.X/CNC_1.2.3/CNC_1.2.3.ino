/*
=====================================================================
  MINI CNC PLOTTER PARA CONFECÇÃO DE PLACAS DE CIRCUITO IMPRESSO
=====================================================================
  
  Autor: Geovane A. Silva
  Data de Criação: Janeiro de 2025
  Versão: 2.3
  
  Licença:
  Este código é distribuído sob a licença MIT, permitindo uso livre
  para qualquer finalidade, desde que seja mantida a atribuição ao autor.

*/

#include "stepMotor.h"

#define TAMANHO_DA_LINHA 256                            // tamanho máximo de caracteres de uma linha
#define Xpos_mm (actuatorPos.x / passosPorMilimetroX)  // pos é a posição em mm
#define Ypos_mm (actuatorPos.y / passosPorMilimetroY)  // enquanto actuatorPos é a posição em passos

const int PassosPorVolta = 200;

StepperSR StepperXY(PassosPorVolta, 11, 12, 13);
StepperSR StepperXYlevantado(PassosPorVolta, 11, 12, 13);
StepperSR StepperDebug(PassosPorVolta, 11, 12, 13);
Stepper StepperZ(PassosPorVolta, 8, 6, 7, 5);

// Sensores de fim de curso
const int x0 = 9;
const int y1 = 10;
const int bot = A0;

struct point {
    float x;
    float y;
    float z;
};

struct point actuatorPos;

// N° de passos para UP e DOWN da caneta:
const int penZUp = 50;
const int penZDown = -50;

const float passosPorMilimetroX = 24.56;
const float passosPorMilimetroY = 23.57;

// Valores em milímetros do limite da área útil de impressão:
const float Xmin = 0.00, Xmax = 92.00;
const float Ymin = 0.00, Ymax = 84.00;
const float Zmin = 0.00, Zmax = 1.00;

int Zpos_mm = Zmax;
int caneta = 1;  // Iniciada em 1, pois a caneta inicialmente estará levantada.

int contagem = 0;

const int drawSpeed = 180;
const int moveSpeed = 230;

unsigned long timeOff;

void setup() {

  pinMode(x0, INPUT_PULLUP);
  pinMode(y1, INPUT_PULLUP);
  pinMode(bot, INPUT_PULLUP);

  StepperXY.setSpeed(drawSpeed);
  StepperXYlevantado.setSpeed(moveSpeed);
  StepperDebug.setSpeed(moveSpeed);
  StepperZ.setSpeed(120);

  //limitStep, mult
  StepperXY.         setAcel(30, 3);
  StepperXYlevantado.setAcel(60, 2);
  StepperDebug.setAcel(60, 2);

  StepperXY.Off();

  Serial.begin(115200);
  Serial.println("Mini CNC Plotter a seu comando!");
  Serial.print("X vai de ");
  Serial.print(Xmin);
  Serial.print(" a ");
  Serial.print(Xmax);
  Serial.println(" mm.");
  Serial.print("Y vai de ");
  Serial.print(Ymin);
  Serial.print(" a ");
  Serial.print(Ymax);
  Serial.println(" mm.");
}


void loop() {

 receberLinha();

}

//============================ Funções ====================================

void receberLinha() {

  char linha[TAMANHO_DA_LINHA];  // Vetor que guardará a linha serial recebida.
  char c;                        // Auxiliar para verificar cada caractere.
  int posicao_linha = 0;         // Índice para controlar a posição do vetor 'linha'.
  bool LinhaComentada = false;   // Flag para identificar e ignorar comentários e ';' nas linhas recebidas.

  while (1) {  // Loop infinito

    if(millis() >= (timeOff + 50)) StepperXY.Off();

    while (Serial.available() > 0) {  // Enquanto houver dados na porta serial...
      c = Serial.read();              // Lê um caractere por vez.
      if ((c == '\n') || (c == '\r')) {
        if (posicao_linha > 0) {
          linha[posicao_linha] = '\0';  // Finaliza a string.
          processarLinha(linha, posicao_linha);
          posicao_linha = 0;
        }
        LinhaComentada = false;
        Serial.println("ok");  // Feedback para o código em Java.
      } else {
        if (LinhaComentada) {                  // Se estiver dentro de um comentário...
          if (c == ')') LinhaComentada = false;  // Fim do comentário.
        } else {
          if ((c == ' ')) {
          } // Ignora espaços
          else if (c == '(') {  // Início do comentário.
            LinhaComentada = true;
          }
          else if (posicao_linha >= TAMANHO_DA_LINHA - 1) {
            Serial.println("over - lineBuffer chegou no limite de caracteres");
            LinhaComentada = false;
          }
          else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {  
            // Se for uma letra, insere um espaço antes (se não for o primeiro caractere)
            if (posicao_linha > 0) {
              linha[posicao_linha++] = ' ';
            }
            // Em seguida, insere a letra convertida para maiúscula, se necessário.
            if (c >= 'a' && c <= 'z') {
              linha[posicao_linha++] = c - 'a' + 'A';
            } else {
              linha[posicao_linha++] = c;
            }
          }
          else {
            linha[posicao_linha++] = c;
          }
        }
      }
    }
  }
}

void processarLinha(char* linha, int Numero_De_Caracteres) {

  struct point newPos;
  newPos.x = 0.0;
  newPos.y = 0.0;

  int verifica_X = 0;
  int verifica_Y = 0;

  // **1️⃣ Processa os comandos diretamente**
  for (int i = 0; i < Numero_De_Caracteres; i++) {
    switch (linha[i]) {

      case 'M': {
        int valorM = atoi(&linha[i + 1]);  // Converte a substring para inteiro após 'M'

        switch (valorM) {
          case 114:  // M114 - Reportar posição atual
            Serial.print("Caneta na posicao : X = ");
            Serial.print(Xpos_mm);
            Serial.print("  -  Y = ");
            Serial.println(Ypos_mm);
            break;
 
          case 18:  // M18 - Fim do desenho
            Serial.println("Desenho finalizado!!!");
            StepperXYlevantado.Off();
            StepperZ.Off();
            break;
          
          case 300:
            break;

          default:
            Serial.print("Comando M desconhecido: M");
            Serial.println(valorM);
        }
        break;
      }

      case 'S': {

        int valorZ = atoi(&linha[i + 1]); 

        if(valorZ == 30) penDown();
        else if (valorZ == 50) penUp();
        break;
      }
      case 'Z': {
        int valorZ = atoi(&linha[i + 1]);  // Converte a substring para inteiro após 'Z'

        if (valorZ <= 0)
          penDown();
        else
          penUp();
        break;
      }

      case 'U':  // Levanta a caneta
        StepperZ.step(penZUp);
        StepperZ.Off();
        Zpos_mm = Zmax;
        break;

      case 'D':  // Abaixa a caneta
        StepperZ.step(penZDown);
        StepperZ.Off();
        Zpos_mm = Zmin;
        break;

      case 'X':
      case 'Y': {
        if (linha[i] == 'X') {
          newPos.x = atof(&linha[i + 1]);  // Converte a substring para float após 'X'
          verifica_X = 1;
        } else {
          newPos.y = atof(&linha[i + 1]);  // Converte a substring para float após 'Y'
          verifica_Y = 1;
        }
        break;
      }
    }
  }

  // **2️⃣ Verifica se X e Y foram alterados antes de mover os motores**
  if (verifica_Y == 0) {
    newPos.y = Ypos_mm;
  }
  if (verifica_X == 0) {
    newPos.x = Xpos_mm;
  }

  // Chamada da função que move os motores:
  drawLine(newPos.x, newPos.y);

  // Reset de variáveis de controle:
  verifica_Y = 0;
  verifica_X = 0;

  timeOff = millis();
  //StepperXY.Off();
}



// **************************************************************************************************************************************************

// Função que move os motores para as coordenadas recebidas.
void drawLine(float targetX, float targetY) {

  // Limitação de coordenadas (em mm).
  targetX = (targetX >= Xmax) ? Xmax : (targetX <= Xmin) ? Xmin : targetX;
  targetY = (targetY >= Ymax) ? Ymax : (targetY <= Ymin) ? Ymin : targetY;

  // Converte as coordenadas de mm para passos.
  targetX = round(targetX * passosPorMilimetroX);
  targetY = round(targetY * passosPorMilimetroY);

  // Calcula a variação (em passos) entre a posição atual e a posição destino.
  int disX = abs(targetX - actuatorPos.x);
  int disY = abs(targetY - actuatorPos.y);

  // Determina a direção do movimento para cada eixo.
  int dirX = (actuatorPos.x < targetX) ? 1 : -1;
  int dirY = (actuatorPos.y < targetY) ? 1 : -1;
  
  // Utilização de referências &stepperX e &stepperY para facilitar o código
  StepperSR &motor = caneta ? StepperXYlevantado : StepperXY;

  long over = 0;
  int corrigido = 0;
  int n = (disX > disY) ? disX : disY;  // Número de iterações = maior entre disX e disY

  bool canGoX;
  bool canGoY;

  motor.resetAcelParameters(n);

  for (int i = 0; i < n; i++) {

      // Verificação de correção de posição
      if (verify() && !corrigido) {
        corrigido = 1;
        contagem = 0;
      }
    contagem++;

    motor.aceleration(i);

    // Eixo principal: X
    if (disX >= disY) {
      canGoX = true;
      actuatorPos.x += dirX;
      over += disY;
      if (over >= disX) {
        over -= disX;
        canGoY = true;
        actuatorPos.y += dirY;
      }
    } // Eixo principal: Y
    else {
      canGoY = true;
      actuatorPos.y += dirY;
      over += disX;
      if (over >= disY) {
        over -= disY;
        canGoX = true;
        actuatorPos.x += dirX;
      }
    }
    motor.oneStep(canGoX, canGoY, dirX, dirY);
    canGoX = false;
    canGoY = false;
  }
} // Fim de drawLine

// **************************************************************************************************************************************************

int verify() {
      
    bool canGoX;
    bool canGoY;
    int pen = caneta;

    if (((!digitalRead(x0) || !digitalRead(y1) || !digitalRead(bot))) || contagem == 10000) {
      // Pause
      if (!digitalRead(bot)) {
        StepperDebug.Off();
        delay(500);
        while (digitalRead(bot)) {}
      }

      if (!pen) {
      penUp();
      }

      // Move os eixos até os sensores
      while (digitalRead(x0) || digitalRead(y1)) {
        
        if (digitalRead(x0)) canGoX = true;
          else canGoX = false;
        if (digitalRead(y1)) canGoY = true;
          else canGoY = false;

        StepperDebug.oneStep(canGoX, canGoY, -1, 1);
      }

      // Move os eixos até desativar os sensores (posição home)
      while (!digitalRead(x0) || !digitalRead(y1)) {
        
        if (!digitalRead(x0)) canGoX = true; 
          else canGoX = false;
        if (!digitalRead(y1)) canGoY = true;
        else canGoY = false;

        StepperDebug.oneStep(canGoX, canGoY, 1, -1);
      }

      int contadorX = abs(actuatorPos.x - (Xmin * passosPorMilimetroX));
      int contadorY = abs((Ymax * passosPorMilimetroY) - actuatorPos.y);

      long over = 0;
      int n = (contadorX > contadorY) ? contadorX : contadorY;  // Número de iterações = maior entre contadorX e contadorY
      
      StepperDebug.resetAcelParameters(n);

      for (int i = 0; i < n; i++) {
        StepperDebug.aceleration(i);

      if (contadorX >= contadorY) {
          // Eixo principal: X
          canGoX = true;
          over += contadorY;
          if (over >= contadorX) {
              over -= contadorX;
              canGoY = true;
          }
      } 
      else {
        // Eixo principal: Y
        canGoY = true;
        over += contadorX;
        if (over >= contadorY) {
            over -= contadorY;
            canGoX = true;
        }
      }
      StepperDebug.oneStep(canGoX, canGoY, 1, -1);
      canGoX = false;
      canGoY = false;
    }
      if (!pen) {
        penDown();
      }
      return 1;
    }
    else return 0;
}


// **************************************************************************************************************************************************

void penUp() {

  if (!caneta) {
    StepperZ.step(penZUp);
    StepperZ.Off();

    Zpos_mm = Zmax;
    caneta = 1;
  }
}

// **************************************************************************************************************************************************

void penDown() {

  if (caneta) {
    StepperZ.step(penZDown);
    StepperZ.Off();

    Zpos_mm = Zmin;
    caneta = 0;
  }
}


