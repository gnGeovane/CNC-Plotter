# CNC-Plotter

Aqui você encontrará os arquivos necessários para fazer uma CNC Plotter e controlá-la.

# Código:

Todos os códigos e a biblioteca para motores de passo foram feitos do zero, linha-a-linha, especificamente para este projeto.
  
As versões mais recentes do código para o arduino incluem suporte para:

- Drivers de motores de passo L293D, ULN2803 e ULN2003;
- Sensores de fim de curso;
- Botão stop;
- Controle de motores de passo com ShiftRegister 74595 usando FullSteps e HalfSteps;
- Motores de passo bipolares ou unipolares.

# G-CODE Sender

<img src="https://github.com/user-attachments/assets/b16e0854-48ce-461f-8a52-be4861c618d7" alt="Captura de tela 2025-06-23 110918" width="480">

Para a comunicação do computador com o arduino para o envio do G-CODE do desenho, foi feito um G-CODE Sender do zero, programado no processing 4.
Este arquivo, quando executado no compilador processing, cria uma interface gráfica simples mostrando os comandos de controle da CNC, como:

- Movimentar algum eixo;
- Subir e descer a caneta;
- Ir para a posição Home;
- Buscar um arquivo G-CODE no Windows Explorer e iniciar o envio serial.

# APP para android

Também foi criado um app em Kotlin no Android Studio que funciona de forma similar ao programa G-CODE Sender do item anterior. Porém, o diferencial do app para celular é a possibilidade de controlar a máquina à distância via Bluetooth.

<img width="400" height="800" alt="Adobe Express - file" src="https://github.com/user-attachments/assets/f6d6319e-5708-424f-b879-4bf21e9cae9d" />
<br> Demonstração do app aqui: https://youtu.be/cIXA6mVwmhk

# Criação de G-CODE

Para criar o arquivo de coordenadas (G-CODE) de uma imagem para desenhá-la, é necessário utilizar o software InksCape (gratuito) ou Autodesk ArtCAM (Pago). Caso queira plotar um circuito em uma PCB, poderá converter arquivos Gerber(.gbr) e Excellon(.drl) em G-CODE utilizando o software de código aberto FlatCAM.
  
# Veja os resultados atuais do projeto:

 <img src= "https://github.com/user-attachments/assets/7f2f3f2b-0f02-432c-927f-b8280d44c11f" alt="CNC" width="600">

<img src="https://github.com/user-attachments/assets/37227357-bcdf-41d3-a32f-c4ceca0404bb" alt="IMG_20251009_102834619_HDR" width="600">

<br> <img src="https://github.com/user-attachments/assets/59b7aeef-37a0-487b-9d7a-38e931c0402e" alt="Desenhos" width="600">

<img src="https://github.com/user-attachments/assets/fdfeb391-7295-4330-b843-7dff32849373" alt="Desenhos" width="600">

<img src="https://github.com/user-attachments/assets/3780d1a4-7bba-4226-90e6-3467ad941881" alt="Captura de tela 2025-06-23 113415" width="600">

# Assista a CNC em ação no YouTube!

[Desenhando ícone do app (controle por Bluetooth com app)](https://youtu.be/TSL-Wnw5sp4)

<a href="https://youtu.be/TSL-Wnw5sp4">
  <img src="https://img.youtube.com/vi/TSL-Wnw5sp4/hqdefault.jpg" alt="Desenhando ícone do app" width="480">
</a>

<br> [Desenhando um avião (controle via porta serial)](https://www.youtube.com/watch?v=H5-JilG_zv4)

<a href="https://www.youtube.com/watch?v=H5-JilG_zv4">
  <img src="https://img.youtube.com/vi/H5-JilG_zv4/hqdefault.jpg" alt="Desenhando um avião" width="480">
</a>


<br> [short - Desenhando um touro (Controle via porta serial)](https://www.youtube.com/shorts/jxCdN6tVYik)

<a href="https://www.youtube.com/shorts/jxCdN6tVYik">
  <img src="https://img.youtube.com/vi/jxCdN6tVYik/hqdefault.jpg" alt="Desenhando um touro" width="480">
</a>
