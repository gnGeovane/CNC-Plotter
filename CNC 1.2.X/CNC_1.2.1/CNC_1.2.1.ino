/*
=====================================================================
  MINI CNC PLOTTER PARA CONFECÇÃO DE PLACAS DE CIRCUITO IMPRESSO
=====================================================================
  
  Autor: Geovane A. Silva
  Data de Criação: Janeiro de 2025
  Versão: 2.1
  
  Licença:
  Este código é distribuído sob a licença MIT, permitindo uso livre
  para qualquer finalidade, desde que seja mantida a atribuição ao autor.

 */

#include "HalfStepper.h"

#define TAMANHO_DA_LINHA 512                            // tamanho máximo de caracteres de uma linha
#define Xpos_mm (actuatorPos.x / passosPorMilimetroX)  // pos é a posição em mm
#define Ypos_mm (actuatorPos.y / passosPorMilimetroY)  // enquanto actuatorPos é a posição em passos


const int PassosPorVolta = 200;

Stepper StepperX(PassosPorVolta, A0, A1, A2, A3);
Stepper StepperY(PassosPorVolta, 6, 7, 8, 9);
Stepper StepperZ(PassosPorVolta, 10, 11, 12, 13); // 1,2,3,5,12,19

Stepper StepperXlevantado(PassosPorVolta, A0, A1, A2, A3);
Stepper StepperYlevantado(PassosPorVolta, 6, 7, 8, 9);

struct point {
  float x;
  float y;
  float z;
};

struct point actuatorPos;

// Configurações de movimento:
const int LineDelay = 3;  // delay para dar tempo ao código em java receber "Ok" e enviar a próxima linha.
const int penDelay = 0;

// N° de passos para UP e DOWN da caneta:
const int penZUp = 50;
const int penZDown = -50;

const float passosPorMilimetroX = 24.56;
const float passosPorMilimetroY = 23.57; // 26.8

// Valores em milímetros do limite da área útil de impressão:
const float Xmin = 0.00, Xmax = 92.00;
const float Ymin = 0.00, Ymax = 79.00;
const float Zmin = 0.00, Zmax = 1.00;

int Zpos_mm = Zmax;
int caneta = 1;  // Iniciada em 1, pois a caneta inicialmente estará levantada.

int contagem = 0;

void setup() {

  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);

  StepperX.setSpeed(120);
  StepperY.setSpeed(120);
  StepperZ.setSpeed(200);

  StepperXlevantado.setSpeed(200);
  StepperYlevantado.setSpeed(200);

  Serial.begin(9600);
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

  char linha[TAMANHO_DA_LINHA];  // Vetor que guardará a linha serial recebida.
  char c;                        // Auxiliar para verificar cada caractere.
  int posicao_linha = 0;         // Índice para controlar a posição do vetor 'linha'.
  bool LinhaComentada = false;   // Flag para identificar e ignorar comentários e ';' nas linhas recebidas.

  while (1) {  // Loop infinito

    while (Serial.available() > 0) {  // Enquanto houver dados na porta serial...
      c = Serial.read();              // Lê um caractere por vez.
      if ((c == '\n') || (c == '\r')) {
        if (posicao_linha > 0) {
          linha[posicao_linha] = '\0';  // Finaliza a string.
          processarNovaLinha(linha, posicao_linha);
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
            Serial.println("ERROR - lineBuffer chegou no limite de caracteres");
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

//============================ Funções ====================================

void processarNovaLinha(char* linha, int Numero_De_Caracteres) {

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
            StepperXlevantado.Off();
            StepperYlevantado.Off();
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
        delay(LineDelay);
        Zpos_mm = Zmax;
        break;

      case 'D':  // Abaixa a caneta
        StepperZ.step(penZDown);
        StepperZ.Off();
        delay(LineDelay);
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
  drawLine(newPos.x, newPos.y, 0);

  // Reset de variáveis de controle:
  verifica_Y = 0;
  verifica_X = 0;

  StepperX.Off();
  StepperY.Off();
}



// **************************************************************************************************************************************************

// Função que move os motores para as coordenadas recebidas.
void drawLine(float targetX, float targetY, int erro) {

  // Limitação de coordenadas (em mm).
  targetX = (targetX >= Xmax) ? Xmax : (targetX <= Xmin) ? Xmin : targetX;
  targetY = (targetY >= Ymax) ? Ymax : (targetY <= Ymin) ? Ymin : targetY;

  // Converte as coordenadas de mm para passos.
  targetX = round(targetX * passosPorMilimetroX);
  targetY = round(targetY * passosPorMilimetroY);

  if (erro == 0) {

    // Calcula a variação (em passos) entre a posição atual e a posição destino.
    int disX = abs(targetX - actuatorPos.x);
    int disY = abs(targetY - actuatorPos.y);

    // Determina a direção do movimento para cada eixo.
    int dirX = (actuatorPos.x < targetX) ? 1 : -1;
    int dirY = (actuatorPos.y < targetY) ? 1 : -1;

    Bresenham(disX, disY, dirX, dirY);
    
  } else if (erro == 1) {
      int estado_caneta = 1;

      if(!caneta) {
        penUp();
        estado_caneta = 0;
      }
      // Move os eixos até os sensores
      while(!digitalRead(3) || !digitalRead(4)) {

        if (!digitalRead(3))
            StepperXlevantado.step(-1);
        else StepperXlevantado.Off();
        if (!digitalRead(4))
          StepperYlevantado.step(1);
        else StepperYlevantado.Off();
      }
      // Move os eixos até desativar os sensores (posição home)
      while(digitalRead(3) || digitalRead(4)) {

        if (digitalRead(3))
            StepperXlevantado.step(1);
        else StepperXlevantado.Off();
        if (digitalRead(4))
          StepperYlevantado.step(-1);
        else StepperYlevantado.Off();
      }

      int disX = (actuatorPos.x - targetX);
      int disY = abs(targetY - actuatorPos.y);
      int contadorX = disX;
      int contadorY = disY;

      while(contadorX > 0 || contadorY > 0) {

        if (contadorX > 0) {
            StepperXlevantado.step(1);
            contadorX--;
        } else StepperXlevantado.Off();
        if (contadorY > 0) {
          StepperYlevantado.step(-1);
          contadorY--;
        } else StepperYlevantado.Off();
      }

      if(estado_caneta == 0) 
        penDown();
    }
} // Fim de drawLine

void Bresenham(int disX, int disY, int dirX, int dirY) {

  /*
  Utilizei aqui uma técnica de movimento dos motores baseada no algorítimo de Bresenham,
  que é ativada quando um dos eixos tem que se mover para uma distância maior do que outro.

  Dessa forma, quando, por exemplo, X for andar 50mm e Y 10mm, o eixo Y irá andar com passos proporcionais aos de X.
  Nesse caso, Y dará um passo a cada 5 passos de X.

  Essa parte do código tem a finalidade de apenas fazer os dois eixos chegarem no destino ao mesmo tempo, sincronizadamente,
  e fazer as linhas/retas das coordenadas no ângulo correto.
  */

  // Variáveis auxiliares para o algorítimo de Bresenham:
  long i;
  long over = 0;
  int corrigido = 0;
  // Utilização de referências &stepperX e &stepperY para facilitar o código
  Stepper &stepperX = caneta ? StepperXlevantado : StepperX;
  Stepper &stepperY = caneta ? StepperYlevantado : StepperY;

  // Algoritmo de Bresenham (linha horizontal maior que vertical):
  if (disX > disY) {
    for (i = 0; i < disX; ++i) {
    
    //======================================== Verifiação de correção ========================================
        if (((digitalRead(3) || digitalRead(4) || digitalRead(5)) && corrigido == 0) || contagem == 10000) {
          // Pause
          if(digitalRead(5)){
            StepperXlevantado.Off();
            StepperYlevantado.Off();
            delay(500);
            while(!digitalRead(5)) {}
          }
          drawLine( Xmin, Ymax, 1 );
          corrigido = 1;
          contagem = 0;
        }
      contagem++;
    //========================================================================================================

      stepperX.step(dirX);
      actuatorPos.x += dirX;
      over += disY;
      if (over >= disX) {
        over -= disX;
        stepperY.step(dirY);
        actuatorPos.y += dirY;
      }
    }

    // Algoritmo de Bresenham (linha vertical maior que horizontal):
  } else {
    for (i = 0; i < disY; ++i) {

    //======================================== Verifiação de correção ========================================
        if (((digitalRead(3) || digitalRead(4) || digitalRead(5)) && corrigido == 0) || contagem == 10000) {
          // Pause
          if(digitalRead(5)){
            StepperXlevantado.Off();
            StepperYlevantado.Off();
            delay(500);
            while(!digitalRead(5)) {}
          }
          drawLine( Xmin, Ymax, 1 );
          corrigido = 1;
          contagem = 0;
        }
      contagem++;
    //========================================================================================================

      stepperY.step(dirY);
      actuatorPos.y += dirY;
      over += disX;
      if (over >= disY) {
        over -= disY;
        stepperX.step(dirX);
        actuatorPos.x += dirX;
      }
    }
  }

  if(LineDelay > 4) {
  StepperXlevantado.Off();
  StepperYlevantado.Off();
  StepperZ.Off();
  delay(LineDelay);
  }

  corrigido = 0;
}

// **************************************************************************************************************************************************

void penUp() {

  if (caneta == 0) {
    StepperZ.step(penZUp);
    delay(LineDelay);

    StepperZ.Off();

    Zpos_mm = Zmax;
    caneta = 1;
  }
}

// **************************************************************************************************************************************************

void penDown() {

  if (caneta == 1) {
    StepperZ.step(penZDown);
    delay(LineDelay);

    StepperZ.Off();

    Zpos_mm = Zmin;
    caneta = 0;
  }
}





