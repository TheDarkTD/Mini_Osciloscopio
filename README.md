# Monitor de Sinais (Osciloscópio) - Projeto em Arduino

### Autor: Felipe Rodrigues Moreira Dos Santos
[![GitHub](https://img.shields.io/badge/GitHub-Perfil-informational)](https://github.com/TheDarkTD)

## Descrição
Este projeto implementa um osciloscópio básico utilizando Arduino e um display OLED. Ele permite a captura e exibição de sinais analógicos, com medição de frequência, ciclo ativo e escalas configuráveis de tensão e tempo.

A interface de controle permite ajustes de escala, polaridade de trigger e pausa na execução, com exibição das informações capturadas e processadas diretamente no display OLED.

## Funcionalidades
- **Exibição em Tela OLED**: Visualização dos sinais capturados, incluindo frequência, ciclo ativo e escalas de tensão e tempo.
- **Configuração de Escalas**: Ajustes de escala de tempo e tensão para melhor visualização dos sinais.
- **Trigger**: Controle de polaridade do trigger para melhor sincronização do sinal.
- **Pausa**: Opção de pausa na exibição para uma análise mais detalhada.
- **EEPROM**: Armazenamento das configurações de escala e trigger para manter os ajustes entre as reinicializações.

## Requisitos de Hardware
- **Arduino** (compatível com a biblioteca `Adafruit_SSD1306` para display OLED)
- **Display OLED** (SSD1306, resolução de 128x64)
- **Componentes adicionais**:
  - Resistores e botões para entrada
  - LEDs (para indicação de captura de sinal)

## Requisitos de Software
- **Arduino IDE**: Certifique-se de que o Arduino IDE está instalado.
- **Bibliotecas Necessárias**:
  - **Adafruit GFX Library**: Para gráficos no display.
  - **Adafruit SSD1306 Library**: Para controle do display OLED.

### Instalação de Bibliotecas
As bibliotecas podem ser instaladas diretamente pelo Arduino IDE:
1. No menu do Arduino IDE, vá até **Sketch > Include Library > Manage Libraries...**
2. Procure por **Adafruit GFX** e **Adafruit SSD1306** e instale ambas.

## Instalação e Configuração do Projeto
1. Clone o repositório ou baixe os arquivos diretamente:
   ```bash
   git clone https://github.com/TheDarkTD/MonitorDeSinaisOsciloscopio.git
   ```
2. Abra o arquivo `.ino` no Arduino IDE.
3. Conecte o Arduino ao computador e faça o upload do código para o dispositivo.

### Configuração dos Pinos
| Pino | Função                |
|------|------------------------|
| 2    | Acionar               |
| 8    | Seleção de Parâmetro  |
| 9    | Aumentar Escala       |
| 10   | Diminuir Escala       |
| 11   | Pausa                 |
| 12   | Controle de Atenuador |
| 13   | LED Indicador         |

## Uso do Projeto

### Controles
- **Seleção de Parâmetro**: Use o botão conectado ao pino 8 para alternar entre os modos de ajuste (escala de tensão, escala de tempo e polaridade do trigger).
- **Ajuste de Escalas**: Utilize os botões conectados aos pinos 9 e 10 para ajustar as escalas.
- **Gatilho e Pausa**: O botão de gatilho no pino 2 permite a detecção de gatilho, e o botão de pausa no pino 11 congela a exibição do sinal.

### Exibição no Display
- **Escala de Tensão e Tempo**: Exibidas no canto superior esquerdo e direito do display, respectivamente.
- **Frequência e Ciclo Ativo**: Exibidos abaixo das escalas, indicando as características do sinal capturado.
- **Mensagem "PAUSADO"**: Exibida quando a execução do sinal está em pausa.

## Estrutura de Código
- `setup()`: Configura o display, os pinos e inicializa as interrupções para os botões.
- `loop()`: Captura os sinais, processa as estatísticas, exibe as informações e salva as configurações na EEPROM.
- `capturarSinal()`: Captura o sinal analógico e armazena os dados no buffer.
- `definirPontoTrigger()`: Identifica o ponto de trigger do sinal para sincronização.
- `calcularFrequencia()`: Calcula a frequência e ciclo ativo do sinal.
- `exibirDados()`: Exibe as informações processadas na tela OLED.

## Referências
Este projeto foi inspirado e guiado por informações e recursos fornecidos por:
- **Site**: (http://radiopench.blog96.fc2.com/blog-entry-997.html)


## Licença
Este projeto é livre para uso e modificação. Sinta-se à vontade para contribuir e adaptar conforme necessário.

---

Este projeto é uma excelente ferramenta de aprendizado para quem deseja explorar conceitos de captura e exibição de sinais usando Arduino e displays OLED. Contribuições e melhorias são sempre bem-vindas!
