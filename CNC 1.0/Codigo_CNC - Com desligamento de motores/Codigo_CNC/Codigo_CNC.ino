#include "halfStepper.h"

#define LINE_BUFFER_LENGTH 512 // tamanho máximo de caracteres de uma linha

const int PassosPorVolta = 48;

Stepper myStepperZ(PassosPorVolta, 10,11,12,13, true);            
Stepper myStepperY(PassosPorVolta, 9,8,7,6, true);  
Stepper myStepperX(PassosPorVolta, 2,3,4,5, true);

struct point { 
  float x; 
  float y; 
  float z; 
};

struct point actuatorPos;

// Configurações de movimento:
float StepInc = 1;
int StepDelay = 0;
int LineDelay = 50; // delay para dar tempo ao código em java receber "Ok" e enviar a próxima linha.
int penDelay = 5;

// variáveis de verificação de eixos
int verifica_X = 0;
int verifica_Y = 0;
int caneta = 1;     // Iniciada em 1, pois a caneta inicialmente estará levantada.

// N° de passos para UP e DOWN da caneta:
const int penZUp = 10.0;
const int penZDown = -10.0;

float StepsPerMillimeterX = 6.85; 
float StepsPerMillimeterY = 6.40;

// Valores em milímetros do limite da área útil de impressão:
float Xmin = 0, Xmax = 85;
float Ymin = 0, Ymax = 85;
float Zmin = 0, Zmax = 1;

long Tempo_Resfriamento = 5; // Em minutos.

float Xpos = Xmin;
float Ypos = Ymin;
float Zpos = Zmax; 

boolean detalhar = false; // Variável para debug. Edite para "true" para receber dados adicionais de variáveis e vetores.

void setup() {

  Serial.begin( 9600 );

  myStepperX.setSpeed(150); 
  myStepperY.setSpeed(150);
  myStepperZ.setSpeed(150);  
/*
  myStepperX.setSpeed(150); 
  myStepperY.setSpeed(150);
  myStepperZ.setSpeed(150); */  

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

  char line[ LINE_BUFFER_LENGTH ];   // Vetor que guardará a linha serial recebida.
  char c;                            // Auxiliar para veerificar cada caractere.
  int lineIndex;                     // Índice para controlar a posição do vetor 'line'.
  bool lineIsComment;                // Flag para identificar e ignorar comentários e ';' nas linhas recebidas.

  lineIndex = 0;
  lineIsComment = false;

  while (1) {

    while ( Serial.available()>0 ) {   // Verifica se há dados disponíveis na porta serial. Se sim, inicia o processamento.
      c = Serial.read();               // C guardará 1 caractere por vez, para verificação individual.
      if (( c == '\n') || (c == '\r') ) {             
        if ( lineIndex > 0 ) {                        
          line[ lineIndex ] = '\0';                   
          if (detalhar) { 
            Serial.print( "Recebido : "); 
            Serial.println( line ); 
          }
          processIncomingLine( line, lineIndex );
          lineIndex = 0;
        } 
        
        lineIsComment = false;
        Serial.println("ok");    // feedback para o código em java enviar a próxima linha.
      }
      else {
        if ( (lineIsComment) ) {   // Ignora caracteres não relacionados às coordenadas. Verifica um caractere de cada vez.
          if ( c == ')' )  lineIsComment = false;     // Fim do comentário entre parênteses, código continua lendo coordenadas.
        }
        else {
          if ( c <= ' ' ) {                           
          } 
          else if ( c == '/' ) {                    
          } 
          else if ( c == 'I' ) {                    
          }
          else if ( c == 'F' ) {                    
          }
          else if ( c == 'J' ) {                    
          }
          else if ( c == ';' ) {
          }
          else if ( c == '(' ) {   // Início do comentário entre parênteses. Código ignora todos os caracteres até encontrar ')'.
            lineIsComment = true; 
          }
          else if ( lineIndex >= LINE_BUFFER_LENGTH-1 ) {
            Serial.println( "ERROR - lineBuffer chegou no limite de caracteres" );
            lineIsComment = false;
          } 
          else if ( c >= 'a' && c <= 'z' ) {   // Transforma letras minúsculas em maiúsculas.
            line[ lineIndex++ ] = c-'a'+'A';
          } 
          else {
            line[ lineIndex++ ] = c;
          }
        }
      }
    }
  }
}

// Funções

// **************************************************************************************************************************************************

void processIncomingLine( char* line, int charNB ) { // line: string com coordenadas já formatadas, sem comentários ou caracteres unúteis.
  int currentIndex = 0;                              // charNB: Número de caracteres da linha formatada.
  char buffer[ 64 ];    // buffers que guardam os caracteres numéricos de X e Y.
  char buffer2[ 64 ];

  struct point newPos;

  newPos.x = 0.0;
  newPos.y = 0.0;


/*  
    Enquanto a posição atual no vetor 'line' for menor que o número máximo de caracteres,
    este loop vai verificar cada caractere novamente e executar seus respectivos comandos.
*/

while (currentIndex < charNB) { 
    char currentChar = line[currentIndex++];

    switch (currentChar) {
        
        case 'X':
           
            buffer[0] = line[currentIndex++]; // Pega o valor no formato: 'xx.xx'.
            buffer[1] = line[currentIndex++];
            buffer[2] = line[currentIndex++];
            buffer[3] = line[currentIndex++];
            buffer[4] = line[currentIndex++];
            buffer[5] = '\0'; // Termina a string.

            if (buffer2[4] < '0' || buffer[4] > '9') {   // Substitui o caractere dentro de buffer[4] por '0' caso não seja um dígito de 0 a 9. Ex: X10.0' '.
                buffer2[4] = '0'; 
            }

            newPos.x = atof(buffer); // Transforma a string buffer em um valor float.
            
            verifica_X = 1; 

            

            // Propositalmente sem 'break' para fazer a leitura automática de Y.

        case 'Y':
            currentIndex++;
            buffer2[0] = line[currentIndex++]; // Pega o valor no formato: 'yy.yy'.
            buffer2[1] = line[currentIndex++];
            buffer2[2] = line[currentIndex++];
            buffer2[3] = line[currentIndex++];
            buffer2[4] = line[currentIndex++];
            buffer2[5] = '\0'; // Termina a string.

            if (buffer2[4] < '0' || buffer2[4] > '9') {   // Substitui o caractere dentro de buffer2[4] por '0' caso não seja um dígito de 0 a 9, assim como em case 'X'.
                buffer2[4] = '0'; 
            }

            newPos.y = atof(buffer2);

            verifica_Y = 1;
            

            if ( verifica_Y == 0 ) {   //mantém a posição de Y se não for lida uma coordenada para ele.
              newPos.y = actuatorPos.y;
            } 
            if ( verifica_X == 0 ) {   //mantém a posição de X se não for lida uma coordenada para ele.
              newPos.x = actuatorPos.x;
            }
            
            // Chamada da função que move os motores:
            drawLine(newPos.x, newPos.y);

            // Atribui a posição recém movida como posição atual:
            actuatorPos.x = newPos.x;
            actuatorPos.y = newPos.y;

            // Reset de variáveis de controle:
            verifica_Y = 0;
            verifica_X = 0;
            
            DriversOff('X');
            DriversOff('Y');

            break; 

      case 'U':                           // 'U' e 'D' são para controle de subida e descida da caneta diretamente pelo monitor serial do arduino IDE
            myStepperZ.step(penZUp);        // Utilizo esses comandos apenas para regular a altura da caneta, sem precisar girar o motor Z manualmente para ajustar
            delay(LineDelay);               // Por essa causa, não têm limitações de quantidade de repetições do mesmo comando.
            
            DriversOff('Z');

            Zpos=Zmax;

            break;
        case 'D':
            myStepperZ.step(penZDown); 
            delay(LineDelay);

            DriversOff('Z');

            Zpos=Zmin;

            break;

      case 'Z':                               // movimenta o eixo Z quando Z > 1 e < 0 (Código em Java edita todos os valores de Z para 1 ou 0).
        buffer[0] = line[currentIndex++];     

      if(atoi(buffer) <= 0)
        penDown();

      else if(atoi(buffer) > 0)
        penUp();
        
      break;

    case 'M':
      buffer[0] = line[ currentIndex++ ];        // grava os 3 caracteres posteriores a 'M' no vetor buffer e transforma ele em um número int.
      buffer[1] = line[ currentIndex++ ];
      buffer[2] = line[ currentIndex++ ];
      buffer[3] = '\0';
      switch ( atoi( buffer ) ){
      case 300:   // Funciona semelhante ao case 'U' e 'D'. A diferença é que esse e o case 'Z' têm a limitação de apenas um Up ou um Down em sequência.
        {
          char* indexS = strchr( line+currentIndex, 'S' );
          float Spos = atof( indexS + 1);
          //          Serial.println("ok");
          if (Spos == 30) { 
            penDown(); 
          }
          if (Spos == 50) { 
            penUp(); 
          }
          break;
        }

      case 114: // M114 - Reportar posição atual.
        Serial.print( "Caneta na posicao : X = " );
        Serial.print( actuatorPos.x );
        Serial.print( "  -  Y = " );
        Serial.println( actuatorPos.y );
        break;
      
      case 8: // Parada para esfriar motores.
        Serial.println();
        Serial.println("Tempo de resfriamento dos motores ativado.");
        
        DriversOff('X');
        DriversOff('Y');
        DriversOff('Z');
        
        for(int i = Tempo_Resfriamento; i > 0; i--) {  // Envia via serial quanto tempo falta para voltar a imprimir, de minuto a minuto.

          Serial.println();
          Serial.println ("Voltando a imprimir em: ");
          Serial.print (i);
          Serial.print (" minuto(s).");
          delay(60000);
        }
      break;

      case 18: // M14 - Desativar todas as bobinas do motor de passo.
        Serial.println("Desativando todas as bobinas dos motores.");
        
        DriversOff('X');
        DriversOff('Y');
        DriversOff('Z');

      break;
      
      default:
        Serial.print( "Comando nao reconhecido : M");
        Serial.println( buffer );
      }
    }
  }
}

// **************************************************************************************************************************************************

// Função que move os motores para as coordenadas recebidas.
void drawLine(float x1, float y1) { 

  if (detalhar)
  {
    Serial.print("fx1, fy1: ");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.println("");
  }  

  // Limitação de coordenadas.
  // Se a coordenada for maior do que o limite, o valor de limite será atribuído a essa coordenada.
  if (x1 >= Xmax) x1 = Xmax; 
  if (x1 <= Xmin) x1 = Xmin; 
  if (y1 >= Ymax) y1 = Ymax; 
  if (y1 <= Ymin) y1 = Ymin;

  if (detalhar)
  {
    Serial.print("Xpos, Ypos: ");
    Serial.print(Xpos);
    Serial.print(",");
    Serial.print(Ypos);
    Serial.println("");
  }

  if (detalhar)
  {
    Serial.print("x1, y1: ");
    Serial.print(x1);
    Serial.print(",");
    Serial.print(y1);
    Serial.println("");
  }

  //  Converte coordenadas para passos.
  x1 = (x1*StepsPerMillimeterX);
  y1 = (y1*StepsPerMillimeterY);
  float x0 = Xpos;
  float y0 = Ypos;

  //  Descobre a mudança em módulo das coordenadas.
  float dx = abs(x1-x0); // abs = valor absoluto, ou simplesmente Módulo.
  float dy = abs(y1-y0);

  // Verificação da direção do movimento
  float sx = x0<x1 ? StepInc : -StepInc; 
  float sy = y0<y1 ? StepInc : -StepInc;

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

// Algoritmo de Bresenham (linha horizontal maior que vertical):
  if (dx > dy) {
    for (i=0; i<dx; ++i) {
      myStepperX.step(sx);
      over+=dy;
      if (over>=dx) {
        over-=dx;
        myStepperY.step(sy);
      }
      delay(StepDelay);
    }
  }

// Algoritmo de Bresenham (linha vertical maior que horizontal):
  else {
    for (i=0; i<dy; ++i) {
      myStepperY.step(sy);
      over+=dx;
      if (over>=dy) {
        over-=dy;
        myStepperX.step(sx);
      }
      delay(StepDelay);
    }    
  }

  if (detalhar)
  {
    Serial.print("dx, dy:");
    Serial.print(dx);
    Serial.print(",");
    Serial.print(dy);
    Serial.println("");
  }

  if (detalhar)
  {
    Serial.print("Indo para (");
    Serial.print(x0);
    Serial.print(",");
    Serial.print(y0);
    Serial.println(")");
  }

  delay(LineDelay);
  
  // Atualiza as variáveis globais para refletir as novas posições após o movimento.
  Xpos = x1;
  Ypos = y1;
}

// **************************************************************************************************************************************************

void penUp() { 

  if(caneta == 0) {
    myStepperZ.step(penZUp);
    delay(LineDelay); 

    DriversOff('Z');
    
    Zpos=Zmax;
    caneta = 1;
  
    if (detalhar) { 
      Serial.println("Pen up!"); 
    } 
  }
}

// **************************************************************************************************************************************************

void penDown() {

  if(caneta == 1) {
    myStepperZ.step(penZDown); 
    delay(LineDelay); 

    DriversOff('Z');

    Zpos=Zmin;
    caneta = 0;

    if (detalhar) { 
      Serial.println("Pen down!"); 
    }  
  }
}

// **************************************************************************************************************************************************

void DriversOff (char eixo) { // Função usada para manter os motores desligados durante o delay entre o recebimento de coordenadas.

  if(eixo == 'X') {
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
  }
  else if (eixo == 'Y') {
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
  }
  else if(eixo == 'Z') {
    digitalWrite(10, LOW);
    digitalWrite(11, LOW);
    digitalWrite(12, LOW);
    digitalWrite(13, LOW);
  }
}
