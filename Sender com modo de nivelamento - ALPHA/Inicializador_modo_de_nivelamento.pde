import processing.serial.*;
import java.io.File;
import java.io.PrintWriter;
import javax.swing.JOptionPane;
import java.awt.event.KeyEvent;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

Serial port = null;
String portname = null;
boolean streaming = false;
String[] gcode;       // linhas originais do G‑CODE
int currentCmdIndex = 0;
long startTime = 0;

// Caminho para salvar o arquivo de nivelamento (definido via seleção)
String savePath = "";
PrintWriter levelingFile = null;

// Parâmetros do arquivo de nivelamento
float levelingSpacing = 0;   // Espaçamento (S)
float levelingAreaX = 0;     // Área em X
float levelingAreaY = 0;     // Área em Y
float lastC = 0;             // Último valor de C enviado

// Lista de pontos de nivelamento lidos do arquivo
ArrayList<LevelPoint> levelingPoints = new ArrayList<LevelPoint>();

// Lista de comandos processados (após interpolação e correção)
ArrayList<String> processedCommands = new ArrayList<String>();

// Caminho do arquivo G‑CODE selecionado (arquivo C)
String gcodeFilePath = "";

// Flag para enviar próxima linha somente após "ok"
boolean canSend = true;

// Classe para armazenar cada ponto do arquivo de nivelamento
class LevelPoint {
  float x, y, c;
  LevelPoint(float x, float y, float c) {
    this.x = x;
    this.y = y;
    this.c = c;
  }
}

// Classe para representar um movimento pequeno (agrupado)
class SmallMove {
  float x, y, z;
  boolean hasZ;
  float c;  // valor de C para esse movimento
  SmallMove(float x, float y, float z, boolean hasZ, float c) {
    this.x = x;
    this.y = y;
    this.z = z;
    this.hasZ = hasZ;
    this.c = c;
  }
  String toStringWithoutPrefix() {
    String s = "X" + nf(x, 2, 2).replace(",", ".")
           + " Y" + nf(y, 2, 2).replace(",", ".");
    if (hasZ) s += " Z" + nf(z, 2, 2).replace(",", ".");
    return s;
  }
}

// Classe para representar um segmento de movimento grande (parametrizado por t, de 0 a 1)
class Segment {
  float t0, t1;
  Segment(float t0, float t1) {
    this.t0 = t0;
    this.t1 = t1;
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
  text("INSTRUÇÕES", 12, y); y += dy;
  text("p: Selecionar Porta Serial", 12, y); y += dy;
  text("Setas: mover X-Y", 12, y); y += dy;
  text("PgUp/PgDn: mover Z", 12, y); y += dy;
  text("h: Resetar Eixos", 12, y); y += dy;
  text("0: Reiniciar placa", 12, y); y += dy;
  text("g: Carregar arquivo para impressão", 12, y); y += dy;
  text("x: Parar impressão", 12, y); y += dy;
  text("e: Editar arquivo", 12, y); y += dy;
  text("N: Modo Nivelamento", 12, y); y += dy;
  text("F: Enviar GCODE corrigido", 12, y); y += dy;
  text("Porta: " + portname, 12, height - dy);
}

void openSerialPort() {
  if (portname == null) return;
  if (port != null) port.stop();
  port = new Serial(this, portname, 115200);
  port.bufferUntil('\n');
}

void selectSerialPort() {
  String result = (String) JOptionPane.showInputDialog(null,
    "Selecione a porta serial para sua placa Arduino.",
    "Selecione a porta Serial",
    JOptionPane.QUESTION_MESSAGE,
    null, Serial.list(), 0);
  if (result != null) {
    portname = result;
    openSerialPort();
  }
}

void keyPressed() {
  // Comando N: Configurar parâmetros de nivelamento
  if (key == 'n' || key == 'N') {
    if (savePath.equals("")) {
      selectFolder("Selecione o diretório para salvar o arquivo de nivelamento", "folderSelected");
      return;
    } else {
      if (levelingFile == null) {
        try {
          levelingFile = new PrintWriter(new File(savePath + "Nivelamento.txt"));
        } catch (Exception e) {
          println("Erro ao criar arquivo de nivelamento: " + e.getMessage());
          return;
        }
      }
      String inputSpacing = JOptionPane.showInputDialog("Espaçamento entre pontos (ex.: 5):");
      String inputAreaX = JOptionPane.showInputDialog("Tamanho da área em X (ex.: 50):");
      String inputAreaY = JOptionPane.showInputDialog("Tamanho da área em Y (ex.: 60):");
      if (inputSpacing != null && inputAreaX != null && inputAreaY != null) {
        levelingSpacing = float(inputSpacing);
        levelingAreaX = float(inputAreaX);
        levelingAreaY = float(inputAreaY);
        String command = "N1 S" + inputSpacing + " N2 X" + inputAreaX + " Y" + inputAreaY;
        println("Enviando comando de nivelamento:");
        println(command);
        port.write(command + "\n");
        if (levelingFile != null) {
          levelingFile.println("(Espaçamento) S" + inputSpacing);
          levelingFile.println("(Área) X" + inputAreaX);
          levelingFile.println("(Área) Y" + inputAreaY);
          levelingFile.println();
          levelingFile.println("=========================================");
          levelingFile.println();
          levelingFile.flush();
        }
      } else {
        println("Dados de nivelamento não informados corretamente.");
      }
    }
  }
  
  // Comando F: Processar G‑CODE corrigido
  if (!streaming && (key == 'f' || key == 'F')) {
    gcode = null;
    currentCmdIndex = 0;
    selectInput("Selecione o arquivo G-CODE para correção de Z (arquivo C)", "gcodeFileSelected");
  }
  
  // Outros comandos manuais
  if (!streaming) {
    if (keyCode == LEFT) port.write("G1 X00.00\n");
    if (keyCode == RIGHT) port.write("G1 X900.00\n");
    if (keyCode == UP) port.write("G1 X00.00 Y900.00\n");
    if (keyCode == DOWN) port.write("G1 Y0.00\n");
    if (keyCode == KeyEvent.VK_PAGE_UP) port.write("G1 Z1.000\n");
    if (keyCode == KeyEvent.VK_PAGE_DOWN) port.write("G1 Z0.000\n");
    if (key == 'h') port.write("G1 Z1.000 X0.000 Y0.000 M18\n");
    if (key == '0') openSerialPort();
    if (key == 'p') selectSerialPort();
    if (key == '$') port.write("$$\n");
  }
  
  if (!streaming && key == 'g') {
    gcode = null;
    currentCmdIndex = 0;
    selectInput("Selecione um arquivo para processar:", "fileSelected");
  }
  
  if (key == 'e') {
    selectInput("Selecione o arquivo a ser editado:", "editFile");
  }
  
  if (key == 'x') {
    streaming = false;
  }
}

// Callback: seleção do diretório para salvar o arquivo de nivelamento
void folderSelected(File selection) {
  if (selection == null) {
    println("Nenhum diretório selecionado.");
    return;
  } else {
    savePath = selection.getAbsolutePath() + File.separator;
    try {
      levelingFile = new PrintWriter(new File(savePath + "Nivelamento.txt"));
    } catch (Exception e) {
      println("Erro ao criar arquivo de nivelamento: " + e.getMessage());
    }
    String inputSpacing = JOptionPane.showInputDialog("Espaçamento entre pontos (ex.: 5):");
    String inputAreaX = JOptionPane.showInputDialog("Tamanho da área em X (ex.: 50):");
    String inputAreaY = JOptionPane.showInputDialog("Tamanho da área em Y (ex.: 60):");
    if (inputSpacing != null && inputAreaX != null && inputAreaY != null) {
      levelingSpacing = float(inputSpacing);
      levelingAreaX = float(inputAreaX);
      levelingAreaY = float(inputAreaY);
      String command = "N1 S" + inputSpacing + " N2 X" + inputAreaX + " Y" + inputAreaY;
      println("Enviando comando de nivelamento:");
      println(command);
      port.write(command + "\n");
      if (levelingFile != null) {
        levelingFile.println("(Espaçamento) S" + inputSpacing);
        levelingFile.println("(Área) X" + inputAreaX);
        levelingFile.println("(Área) Y" + inputAreaY);
        levelingFile.println();
        levelingFile.println("=========================================");
        levelingFile.println();
        levelingFile.flush();
      }
    } else {
      println("Dados de nivelamento não informados corretamente.");
    }
  }
}

// Callback: seleção do arquivo G‑CODE (comando F)
void gcodeFileSelected(File selection) {
  if (selection == null) {
    println("Nenhum arquivo G‑CODE selecionado.");
    return;
  }
  gcodeFilePath = selection.getAbsolutePath();
  println("Arquivo G‑CODE selecionado: " + gcodeFilePath);
  // Após selecionar o arquivo G‑CODE, solicita a seleção do arquivo de nivelamento
  selectInput("Selecione o arquivo de nivelamento", "levelingFileSelected");
}

// Callback: seleção do arquivo de nivelamento para correção de Z
void levelingFileSelected(File selection) {
  if (selection == null) {
    println("Nenhum arquivo de nivelamento selecionado.");
    return;
  }
  String levelingFilePath = selection.getAbsolutePath();
  println("Arquivo de nivelamento selecionado: " + levelingFilePath);
  // Lê os parâmetros e pontos do arquivo de nivelamento
  readLevelingFile(levelingFilePath);
  processGcodeFile(gcodeFilePath);
}

// Lê o arquivo de nivelamento e carrega os parâmetros e pontos (após o cabeçalho)
void readLevelingFile(String filename) {
  String[] lines = loadStrings(filename);
  if (lines == null) return;
  
  levelingPoints.clear();
  boolean headerEnded = false;
  for (int i = 0; i < lines.length; i++) {
    String line = trim(lines[i]);
    if (line.length() == 0) continue;
    if (line.startsWith("(Espaçamento) S")) {
      levelingSpacing = float(trim(line.substring("(Espaçamento) S".length())));
      continue;
    } else if (line.startsWith("(Área) X")) {
      levelingAreaX = float(trim(line.substring("(Área) X".length())));
      continue;
    } else if (line.startsWith("(Área) Y")) {
      levelingAreaY = float(trim(line.substring("(Área) Y".length())));
      continue;
    }
    if (line.startsWith("===")) {
      headerEnded = true;
      continue;
    }
    if (headerEnded && line.startsWith("X")) {
      // Exemplo: "X20.00 Y0.00 C-36"
      String[] tokens = splitTokens(line);
      float xVal = 0, yVal = 0, cVal = 0;
      for (String token : tokens) {
        token = token.trim();
        if (token.startsWith("X")) {
          xVal = float(token.substring(1));
        } else if (token.startsWith("Y")) {
          yVal = float(token.substring(1));
        } else if (token.startsWith("C")) {
          cVal = float(token.substring(1));
        }
      }
      levelingPoints.add(new LevelPoint(xVal, yVal, cVal));
    }
  }
  println("Parâmetros do leveling: S=" + levelingSpacing + ", Área X=" + levelingAreaX + ", Área Y=" + levelingAreaY);
  println("Pontos de nivelamento carregados: " + levelingPoints.size());
}

// Retorna o valor de C para o ponto (x, y) – ponto de nivelamento mais próximo (dentro do limite S)
float getCForPoint(float x, float y) {
  float bestDistance = Float.MAX_VALUE;
  float bestC = lastC; // Se não achar, mantém o último
  for (LevelPoint lp : levelingPoints) {
    if (abs(lp.x - x) <= levelingSpacing && abs(lp.y - y) <= levelingSpacing) {
      float d = dist(x, y, lp.x, lp.y);
      if (d < bestDistance) {
        bestDistance = d;
        bestC = lp.c;
      }
    }
  }
  return bestC;
}

// Função para extrair coordenadas de uma linha usando regex
// Retorna um float[] com: [X, Y, Z] (mantendo os valores anteriores se não encontrados)
float[] extractCoordinates(String line, float prevX, float prevY, float prevZ) {
  float x = prevX, y = prevY, z = prevZ;
  Pattern pattern = Pattern.compile("([XYZ])([-+]?\\d*\\.?\\d+)");
  Matcher matcher = pattern.matcher(line);
  while (matcher.find()){
    String axis = matcher.group(1);
    float value = float(matcher.group(2));
    if (axis.equals("X")) x = value;
    if (axis.equals("Y")) y = value;
    if (axis.equals("Z")) z = value;
  }
  return new float[]{x, y, z};
}

// Processa o arquivo G‑CODE, interpolando movimentos de X e Y.
// Se o movimento for grande (quando pelo menos um dos eixos varia mais que S), percorre o movimento
// em busca do primeiro ponto onde a diferença de C atinge ou ultrapassa 5 e divide o movimento nesse ponto.
// Se o movimento grande for subdividido (nSeg > 1), o primeiro segmento recebe N3, o último N5 e os intermediários N4.
// Se o movimento grande não for subdividido (nSeg == 1), nenhum prefixo é adicionado.
// Se tanto dx quanto dy forem menores que 3mm, os movimentos são agrupados e processados com prefixos: primeiro N3, intermediários N4 e último N5.
// Em cada movimento, o comando C é adicionado se a diferença for maior ou igual a 5.
void processGcodeFile(String filename) {
  String[] lines = loadStrings(filename);
  if (lines == null) return;
  
  float prevX = 0, prevY = 0, prevZ = 0;
  processedCommands.clear();
  
  ArrayList<SmallMove> smallGroup = new ArrayList<SmallMove>();
  
  for (int j = 0; j < lines.length; j++) {
    String line = trim(lines[j]);
    if (line.length() == 0) continue;
    
    boolean hasZ = (line.indexOf("Z") != -1);
    float[] coords = extractCoordinates(line, prevX, prevY, prevZ);
    float newX = coords[0], newY = coords[1], newZ = coords[2];
    
    float dx = newX - prevX;
    float dy = newY - prevY;
    float dz = newZ - prevZ;
    
    // Se ambos os eixos variarem menos que 3mm, agrupa como movimento pequeno.
    if (abs(dx) < 3 && abs(dy) < 3) {
      float currentC = getCForPoint(newX, newY);
      SmallMove sm = new SmallMove(newX, newY, newZ, hasZ, currentC);
      smallGroup.add(sm);
      prevX = newX;
      prevY = newY;
      prevZ = newZ;
    } else {
      // Processa grupo de movimentos pequenos, se existir.
      if (!smallGroup.isEmpty()) {
        processSmallMoves(smallGroup);
        smallGroup.clear();
      }
      // Para movimentos grandes: se pelo menos um dos eixos varia mais que S, subdivide o movimento
      // em função do primeiro ponto onde a diferença de C atingir ou ultrapassar 5.
      ArrayList<Segment> segList = new ArrayList<Segment>();
      float t0 = 0;
      // Usa passo t = 0.5 conforme solicitado
      float step = 0.5;
      while (t0 < 1) {
        float tCandidate = t0 + step;
        if (tCandidate > 1) tCandidate = 1;
        float tFound = tCandidate;
        for (float t = t0; t <= tCandidate; t += step) {
          float xi = prevX + dx * t;
          float yi = prevY + dy * t;
          float cVal = getCForPoint(xi, yi);
          if (abs(cVal - lastC) >= 5) {
            tFound = t;
            break;
          }
        }
        segList.add(new Segment(t0, tFound));
        if (abs(getCForPoint(prevX + dx * tFound, prevY + dy * tFound) - lastC) >= 5) {
          lastC = getCForPoint(prevX + dx * tFound, prevY + dy * tFound);
        }
        t0 = tFound;
      }
      
      int nSeg = segList.size();
      for (int i = 0; i < nSeg; i++) {
        Segment seg = segList.get(i);
        float tEnd = seg.t1;
        float targetX = prevX + dx * tEnd;
        float targetY = prevY + dy * tEnd;
        float targetZ = (hasZ && i == 0) ? (prevZ + dz * tEnd) : prevZ;
        String prefix = "";
        if (nSeg > 1) {
          if (i == 0) prefix = "N3 ";
          else if (i == nSeg - 1) prefix = "N5 ";
          else prefix = "N4 ";
        }
        float currentC = getCForPoint(targetX, targetY);
        float diff = currentC - lastC;
        String extra = "";
        if (abs(diff) >= 5) {
          extra = " C" + nf(diff, 0, 0);
          lastC = currentC;
        }
        String cmd = prefix
          + "X" + nf(targetX, 2, 2).replace(",", ".")
          + " Y" + nf(targetY, 2, 2).replace(",", ".");
        if (hasZ && i == 0) {
          cmd += " Z" + nf(targetZ, 2, 2).replace(",", ".");
        }
        processedCommands.add(cmd);
        println("Comando gerado: " + cmd);
      }
      prevX = newX;
      prevY = newY;
      prevZ = newZ;
    }
  }
  
  // Se sobrar grupo de movimentos pequenos, processa-o.
  if (!smallGroup.isEmpty()) {
    processSmallMoves(smallGroup);
    smallGroup.clear();
  }
  
  streaming = true;
  currentCmdIndex = 0;
  canSend = true;
  streamNextCommand();
}

// Processa um grupo de movimentos pequenos (agrupados em SmallMove)
// Atribui: primeiro movimento do grupo recebe N3, os intermediários N4 e o último N5.
void processSmallMoves(ArrayList<SmallMove> group) {
  int n = group.size();
  for (int i = 0; i < n; i++) {
    SmallMove sm = group.get(i);
    String prefix;
    if (i == 0) prefix = "N3 ";
    else if (i == n - 1) prefix = "N5 ";
    else prefix = "N4 ";
    
    float currentC = sm.c;
    float diff = currentC - lastC;
    String extra = "";
    if (abs(diff) >= 5) {
      extra = " C" + nf(diff, 0, 0);
      lastC = currentC;
    }
    String cmd = prefix + sm.toStringWithoutPrefix() + extra;
    processedCommands.add(cmd);
    println("Comando gerado (small move): " + cmd);
  }
}

// Envia o próximo comando se permitido (aguarda "ok" do Arduino)
void streamNextCommand() {
  if (!streaming) return;
  if (!canSend) return;
  if (currentCmdIndex >= processedCommands.size()) {
    streaming = false;
    println("Fim dos comandos.");
    return;
  }
  String cmd = processedCommands.get(currentCmdIndex);
  port.write(cmd + "\n");
  println("Enviado: " + cmd);
  currentCmdIndex++;
  canSend = false;
}

// Função para editar arquivo (sem alterações relativas à correção)
void editFile(File selection) {
  if (selection == null) {
    println("Nenhum arquivo selecionado.");
    return;
  }
  File saveDirectory = new File(savePath);
  if (!saveDirectory.exists() || !saveDirectory.isDirectory() || !saveDirectory.canWrite()) {
    println("Caminho de salvamento inválido: " + savePath);
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
          float value = float(coord);
          if (axis == 'Z') {
            value = (value >= 0) ? 1 : -1;
            currentZ = "Z" + (int)value;
          } else {
            String formattedValue = nf(value, 2, 2).replace(",", ".");
            if (axis == 'X') currentX = "X" + formattedValue;
            if (axis == 'Y') currentY = "Y" + formattedValue;
          }
        } catch (NumberFormatException e) { }
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
  output[index++] = "M18";
  output[index++] = "M5";
  output = subset(output, 0, index);
  File outputFile = new File(savePath + "Arquivo_editado.txt");
  saveStrings(outputFile.getAbsolutePath(), output);
  println("Arquivo salvo em: " + outputFile.getAbsolutePath());
}

void fileSelected(File selection) {
  if (selection == null) {
    println("Nenhum arquivo selecionado.");
  } else {
    println("Arquivo selecionado: " + selection.getAbsolutePath());
    gcode = loadStrings(selection.getAbsolutePath());
    if (gcode == null) return;
    streaming = true;
    startTime = millis();
    streamNextCommand();
  }
}

// Envio controlado: envia um comando por vez, aguardando "ok" do Arduino
void serialEvent(Serial p) {
  String s = p.readStringUntil('\n');
  if (s == null) return;
  s = s.trim();
  println("Recebido: " + s);
  
  // Se a resposta contiver dados de nivelamento, grava no arquivo
  if (s.startsWith("X") && s.indexOf("Y") != -1 && s.indexOf("C") != -1) {
    if (levelingFile != null) {
      levelingFile.println(s);
      levelingFile.flush();
      println("Nivelamento salvo: " + s);
    }
  }
  
  // Se o Arduino enviar "ok", libera o envio do próximo comando
  if (s.equals("ok")) {
    if (levelingFile != null) {
      levelingFile.println("Finalizado");
      levelingFile.flush();
      levelingFile.close();
      levelingFile = null;
      println("Arquivo de nivelamento finalizado.");
    }
    canSend = true;
    streamNextCommand();
  }
  
  if (s.startsWith("error")) {
    canSend = true;
    streamNextCommand();
  }
}
