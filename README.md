# CNC-Plotter

Aqui você encontrará os arquivos necessários para fazer uma CNC Plotter e controlá-la.

# Código:

  todos os códigos e a biblioteca para motores de passo foram feitos do zero, linha-a-linha, especificamente para este projeto.
  
As versões mais recentes do código para o arduino incluem suporte para:

Sensores de fim de curso;
Botão stop;
Controle de motores de passo com ShiftRegister 74595 usando FullSteps e HalfSteps;
Motores de passo bipolares ou unipolares.

# G-CODE Sender

![Captura de tela 2025-06-23 110918](https://github.com/user-attachments/assets/b16e0854-48ce-461f-8a52-be4861c618d7)

  Para a comunicação do computador com o arduino para o envio do G-CODE do desenho, foi feito um G-CODE Sender do zero, programado no processing 4.
Este arquivo, quando executado no compilador processing, cria uma interface gráfica simples mostrando os comandos de controle da CNC, como:

  Movimentar algum eixo;
  Suir e descer a caneta;
  Ir para a posição Home;
  Buscar um arquivo G-CODE no windows explorer e iniciar o envio serial.

# Criação de G-CODE

  Para criar o arquivo de cooredenadas de uma imagem para deselhá-la, é necessário utilizar o software InksCape (gratuito) ou Autodesk ArtCAM (Pago)
  
# Veja os resultados atuais do projeto:

![CNC](https://github.com/user-attachments/assets/07bf675e-016e-4d0d-b267-4f841a70f2a5)

![Base eixo X e Z](https://github.com/user-attachments/assets/c5f858aa-a01c-451d-be9a-643e2bd71321)

[![Assista no YouTube](https://img.youtube.com/vi/H5-JilG_zv4/hqdefault.jpg)](https://www.youtube.com/watch?v=H5-JilG_zv4)

![Desenhos](https://github.com/user-attachments/assets/fdfeb391-7295-4330-b843-7dff32849373)

![Captura de tela 2025-06-23 113415](https://github.com/user-attachments/assets/3780d1a4-7bba-4226-90e6-3467ad941881)



  
