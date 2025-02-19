Descrição do Projeto:
O joystick fornecerá valores analógicos correspondentes aos eixos X e Y, que serão utilizados para:
Controlar a intensidade luminosa dos LEDs RGB, onde:
• O LED Azul terá seu brilho ajustado conforme o valor do eixo Y. Quando o joystick estiver solto
(posição central - valor 2048), o LED permanecerá apagado. À medida que o joystick for movido para
cima (valores menores) ou para baixo (valores maiores), o LED aumentará seu brilho gradualmente,
atingindo a intensidade máxima nos extremos (0 e 4095).
• O LED Vermelho seguirá o mesmo princípio, mas de acordo com o eixo X. Quando o joystick estiver
solto (posição central - valor 2048), o LED estará apagado. Movendo o joystick para a esquerda
(valores menores) ou para a direita (valores maiores), o LED aumentará de brilho, sendo mais intenso
nos extremos (0 e 4095).
• Os LEDs serão controlados via PWM para permitir variação suave da intensidade luminosa.
Exibir no display SSD1306 um quadrado de 8x8 pixels, inicialmente centralizado, que se moverá
proporcionalmente aos valores capturados pelo joystick.
Adicionalmente, o botão do joystick terá as seguintes funcionalidades:
• Alternar o estado do LED Verde a cada acionamento.
• Modificar a borda do display para indicar quando foi pressionado, alternando entre diferentes estilos
de borda a cada novo acionamento.
Finalmente, o botão A terá a seguinte funcionalidade:
• Ativar ou desativar os LED PWM a cada acionamento.
Neste projeto, deverá ser utilizados os seguintes componentes já interconectados à placa BitDogLab:
• LED RGB, com os pinos conectados às GPIOs (11, 12 e 13).
• Botão do Joystick conectado à GPIO 22.
• Joystick conectado aos GPIOs 26 e 27.
• Botão A conectado à GPIO 5.
• Display SSD1306 conectado via I2C (GPIO 14 e GPIO15).
Entrega: Os seguintes itens devem ser entregues:
Código: O código-fonte do projeto deve ser entregue em um link do seu repositório, contendo todos os
arquivos necessários para sua execução.
Vídeo de demonstração: Deve ser produzido um vídeo de até 2 minutos, contendo:
• O aluno aparecendo pessoalmente para demonstrar o funcionamento do projeto.
• Uma explicação breve das funcionalidades implementadas.
• A execução do projeto na placa BitDogLab.
• Link do vídeo no Youtube ou Google Drive.

Link do video de teste
https://github.com/tiagomoura28/adc
