# joytest

## Tarefa Aula Síncrona de 10/02/2025 - Objetivo: Compreender o ADC do RP204 (Pico-w). 

**A tarefa combina diferentes componentes da placa BitDogLab, são eles**:

• LED RGB;

• Botão do Joystick (GPIO22);

• Joystick conectado aos GPIOs 26 e 27;

• Botão A (GPIO5);

• Display SSD1306;

• Módulo PWM;

• Módulo ADC;

• Módulo I2C.


O Programa consiste da **main()** e de uma **função callback** para tratar as interrupções dos botões (joystick e botão A).

**A main()** engloba todas as inicializações e configurações dos componentes citados acima.

Em seu **looping infinito** é executado:

• Leitura dos canais ADC para os eixos do joystick;

• Mapeamento dos valores do ADC para a posição de um quadrado (8x8 pixels) no display SSD1306;

• Atualiza os níveis do PWM dos LEDs vermelho e azul (somente se não foram desabilitados pelo botão A);

• Processa o evento do botão do joystick (on/off led G e colocação de molduras na tela do display);

• Processa o evento do botão A para ligar/desligar os LEDs vermelho e azul;

• Atualiza o display: Limpar o display; Desenhar o quadrado que representa o joystick; Desenhar a borda.


A **função callback** realiza as seguintes tarefas:

• Desativa as interrupções de GPIO de evento de borda de descida;

• Debouncing baseado em ignorar eventos com intervalos de tempo inferior a 350ms (usando a função time_us_64());

• Testa qual botão (GPIO) que gerou a interrupção, setando a flag de botão_A pressionado, ou a flag de botão_J pressionado;

• Reativa as interrupções;

Os flags setados serão testados na main(), para executar as tarefas disparadas por pressionamento de botões.



## Vídeo demonstrativo


[![Assista no YouTube](https://img.youtube.com/vi/iUD0vo06on8/maxresdefault.jpg)](https://youtu.be/iUD0vo06on8)


