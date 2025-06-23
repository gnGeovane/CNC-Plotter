import java.awt.event.KeyEvent;
import java.io.File;
import javax.swing.JOptionPane;
import processing.serial.*;

Serial port = null;

// select and modify the appropriate line for your operating system
// leave as null to use interactive port (press 'p' in the program)
String portname = null;
boolean streaming = false;
float speed = 0.001;
String[] gcode;
int i = 0;

long startTime = 0; // Para controlar o temporizador de 15 minutos
final long fifteenMinutes = 20 * 60 * 1000; // 20 minutos em milissegundos
boolean m8Sent = false; // Para verificar se o comando M8 já foi enviado

// Caminho para salvar o arquivo editado
String savePath = "E:/CNC/Imagens para impressão/";

void openSerialPort() {
  if (portname == null) return;
  if (port != null) port.stop();

  port = new Serial(this, portname, 115200);
  port.bufferUntil('\n');
}

void selectSerialPort() {
  String result = (String) JOptionPane.showInputDialog(null,
    "Selecione a porta serial que corresponde à sua placa Arduino.",
    "Selecione a porta Serial",
    JOptionPane.QUESTION_MESSAGE,
    null,
    Serial.list(),
    0);

  if (result != null) {
    portname = result;
    openSerialPort();
  }
}

void setup() {
  size(350, 400);
  openSerialPort();
}

void draw() {
  background(200);
  fill(0);
  int y = 24, dy = 12;
  text("INSTRUÇÕES", 12, y);
  y += dy;
  text("p: Selecione a Porta Serial", 12, y);
  y += dy;
  text("1: Alterar Velocidade para 0.001 pol (1 mil) por passo", 12, y);
  y += dy;
  text("2: Alterar Velocidade para 0.010 pol (10 mil) por passo", 12, y);
  y += dy;
  text("3: Alterar Velocidade para 0.100 pol (100 mil) por passo", 12, y);
  y += dy;
  text("Use as setas para mover os eixos X-Y", 12, y);
  y += dy;
  text("page up e page down: para eixo Z", 12, y);
  y += dy;
  text("h: Resetar Eixos", 12, y);
  y += dy;
  text("0: Reiniciar placa", 12, y);
  y += dy;
  text("g: Carregar arquivo para impressão", 12, y);
  y += dy;
  text("x: Parar impressão", 12, y);
  y += dy;
  text("e: Editar um arquivo para o formato padrão", 12, y);
  y += dy;
  y = height - dy;
  text("Velocidade atual: " + speed + " Polegadas por passo", 12, y);
  y -= dy;
  text("Porta selecionada: " + portname, 12, y);
  y -= dy;

  // Verificar o temporizador de 15 minutos
  if (streaming && !m8Sent && millis() - startTime >= fifteenMinutes) {
    if (port != null) {
      port.write("M8\n");
      println("Comando M8 enviado após 20 minutos.");
      m8Sent = true; // Marca que o comando foi enviado
    }
  }
}

void keyPressed() {
  if (key == '1') speed = 0.001;
  if (key == '2') speed = 0.01;
  if (key == '3') speed = 0.1;

  if (!streaming) {
    if (keyCode == LEFT) port.write("G1 X00.00\n");
    if (keyCode == RIGHT) port.write("G1 X900.00\n");
    if (keyCode == UP) port.write("G1 X00.00 Y900.00\n");
    if (keyCode == DOWN) port.write("G1 Y0.00\n");
    if (keyCode == KeyEvent.VK_PAGE_UP) port.write("G1 Z1.000 \n");
    if (keyCode == KeyEvent.VK_PAGE_DOWN) port.write("G1 Z0.000 \n");
    if (key == 'h') port.write("G1 Z1.000 X0.000 Y0.000 M18\n");
    if (key == '0') openSerialPort();
    if (key == 'p') selectSerialPort();
    if (key == '$') port.write("$$\n");
  }

  if (!streaming && key == 'g') {
    gcode = null;
    i = 0;
    selectInput("Selecione um arquivo para processar:", "fileSelected");
  }

  if (key == 'e') {
    selectInput("Selecione o arquivo a ser editado:", "editFile");
  }

  if (key == 'x') {
    streaming = false;
  }
}

void editFile(File selection) {
  if (selection == null) {
    println("Nenhum arquivo selecionado.");
    return;
  }

  // Verificar se o caminho de salvamento é válido
  File saveDirectory = new File(savePath);
  if (!saveDirectory.exists() || !saveDirectory.isDirectory() || !saveDirectory.canWrite()) {
    println("Caminho de salvamento inválido ou não gravável: " + savePath);
    return;
  }

  String[] lines = loadStrings(selection.getAbsolutePath());
  if (lines == null) {
    println("Não foi possível carregar o arquivo.");
    return;
  }

  String[] output = new String[lines.length + 2];
  int index = 0;

  String lastX = null, lastY = null, lastZ = null;

  for (String line : lines) {
    line = line.trim();
    if (line.isEmpty()) continue;

    line = line.replaceAll("([A-Z])", " $1").trim();

    String[] tokens = split(line, ' ');
    StringBuilder editedLine = new StringBuilder("G01");

    String currentX = null, currentY = null, currentZ = null;

    for (String token : tokens) {
      if (token.startsWith("X") || token.startsWith("Y") || token.startsWith("Z")) {
        try {
          char axis = token.charAt(0);
          String coord = token.substring(1);
          float value = Float.parseFloat(coord);

          if (axis == 'Z') {
            value = (value >= 0) ? 1 : -1;
            currentZ = "Z" + (int) value;
          } else {
            String formattedValue = nf(value, 2, 2).replace(',', '.');
            if (axis == 'X') currentX = "X" + formattedValue;
            if (axis == 'Y') currentY = "Y" + formattedValue;
          }
        } catch (NumberFormatException e) {
          // Ignorar valores mal formatados
        }
      }
    }

    if (currentX == null) currentX = lastX;
    if (currentY == null) currentY = lastY;
    if (currentZ == null) currentZ = lastZ;

    if (currentX != null) lastX = currentX;
    if (currentY != null) lastY = currentY;
    if (currentZ != null) lastZ = currentZ;

    if (currentX != null) editedLine.append(" ").append(currentX);
    if (currentY != null) editedLine.append(" ").append(currentY);
    if (currentZ != null) editedLine.append(" ").append(currentZ);

    if (!editedLine.toString().equals("G01")) {
      output[index++] = editedLine.toString();
    }
  }

  // Adicionar comandos finais
  output[index++] = "M18";
  output[index++] = "M5";

  // Ajustar o tamanho do array de saída
  output = subset(output, 0, index);

  // Salvar o arquivo editado
  File outputFile = new File(savePath + "Arquivo_editado.txt");
  saveStrings(outputFile.getAbsolutePath(), output);
  println("Arquivo salvo em: " + outputFile.getAbsolutePath());
}

  void fileSelected(File selection) {
    
      if (selection == null) {
      println("Window was closed or the user hit cancel.");
      } else {
      println("User selected " + selection.getAbsolutePath());
      gcode = loadStrings(selection.getAbsolutePath());
      if (gcode == null) return;
      streaming = true;
      startTime = millis();
      m8Sent = false;
      stream();
    }
  }

  void stream() {
    if (!streaming) return;

    while (true) {
      if (i == gcode.length) {
        streaming = false;
        return;
      }

      if (gcode[i].trim().length() == 0) i++;
      else break;
    }

    println(gcode[i]);
    port.write(gcode[i] + '\n');
    i++;
  }

  void serialEvent(Serial p) {
    String s = p.readStringUntil('\n');
    println(s.trim());

    if (s.trim().startsWith("ok")) stream();
    if (s.trim().startsWith("error")) stream();
  }
