/********************************************************************************
 * Autor: Felipe Rodrigues Moreira Dos Santos
 * Contato: axfeliperodrigues@gmail.com
 * Data: 20/10/2024
 * Projeto: Monitor de Sinais (Osciloscópio)
 * Versão: 2.0
 * GitHub: https://github.com/TheDarkTD
 ********************************************************************************/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

// Configurações de display e buffer de sinal
#define LARGURA_DISPLAY 128         // Largura do display OLED
#define ALTURA_DISPLAY 64           // Altura do display OLED
#define TAMANHO_BUFFER 200          // Tamanho do buffer de sinal
#define LIMIAR_SINCRONIA 5          // Limiar mínimo para detecção de trigger

// Definição do pino de reset para o display OLED
#define RESET_OLED -1            
Adafruit_SSD1306 tela(LARGURA_DISPLAY, ALTURA_DISPLAY, &Wire, RESET_OLED);

// Tabelas de escalas de tensão e tempo, com rótulos para exibição
const char rotulosEscalaTensao[10][5] PROGMEM = {"A50V", "A5V", "50V", "20V", "10V", "5V", "2V", "1V", "0.5V", "0.2V"};
const char * const tabelaTensao[] PROGMEM = {rotulosEscalaTensao[0], rotulosEscalaTensao[1], rotulosEscalaTensao[2], rotulosEscalaTensao[3], rotulosEscalaTensao[4], rotulosEscalaTensao[5], rotulosEscalaTensao[6], rotulosEscalaTensao[7], rotulosEscalaTensao[8], rotulosEscalaTensao[9]};
const char rotulosEscalaTempo[10][6] PROGMEM = {"200ms", "100ms", "50ms", "20ms", "10ms", "5ms", "2ms", "1ms", "500us", "200us"};
const char * const tabelaTempo[] PROGMEM = {rotulosEscalaTempo[0], rotulosEscalaTempo[1], rotulosEscalaTempo[2], rotulosEscalaTempo[3], rotulosEscalaTempo[4], rotulosEscalaTempo[5], rotulosEscalaTempo[6], rotulosEscalaTempo[7], rotulosEscalaTempo[8], rotulosEscalaTempo[9]};
const PROGMEM float valoresEscalaTempo[] = {0.2, 0.1, 0.05, 0.02, 0.01, 0.005, 0.002, 0.001, 0.0005, 0.0002};

int bufferSinal[TAMANHO_BUFFER];    // Buffer de sinal para armazenar leituras
char bufferTexto[8];                // Buffer de texto para exibição de informações
char escalaHorizontal[] = "xxxAs";  // Rótulo da escala de tempo
char escalaVertical[] = "xxxx";     // Rótulo da escala de tensão

float coefTensao5V = 0.00566826;    // Coeficiente de sensibilidade para a escala de 5V
float coefTensao50V = 0.05243212;   // Coeficiente de sensibilidade para a escala de 50V

// Variáveis de controle e flags
volatile int escalaTensao;
volatile int escalaTempo;
volatile int polaridadeTrigger;
volatile int seletorParametro;
volatile bool sinalPausado = false;
volatile bool botaoPressionado = false;
volatile int contadorEEPROM;
int tempoExecucaoCiclo;

int valorMinimoSinal, valorMaximoSinal, mediaSinal;
int maxEscalaGrafico, minEscalaGrafico, valorMaxExibido, valorMinExibido;
int pontoTrigger;
bool triggerEncontrado;

float frequenciaSinal, cicloAtivo;

// Configuração do display e exibição de uma mensagem inicial
void configurarDisplay() {
    if (!tela.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        while (true);
    }
    exibirMensagemInicial();
    carregarConfiguracoes();
    analogReference(INTERNAL);  // Define a referência de tensão do ADC
    configurarInterrupcoes();   // Configura interrupções
}

// Configura os pinos utilizados para botões e LED
void configurarPinos() {
    pinMode(2, INPUT_PULLUP);       // Botão de trigger
    pinMode(8, INPUT_PULLUP);       // Botão de seleção de parâmetro
    pinMode(9, INPUT_PULLUP);       // Botão para aumentar escala
    pinMode(10, INPUT_PULLUP);      // Botão para diminuir escala
    pinMode(11, INPUT_PULLUP);      // Botão de pausa
    pinMode(12, INPUT);             // Controle de atenuador (Hi-z quando entrada)
    pinMode(13, OUTPUT);            // LED indicador
}

// Configura interrupções no botão de trigger (pino 2)
void configurarInterrupcoes() {
    attachInterrupt(digitalPinToInterrupt(2), aoPressionarBotao, FALLING);  
}

// Função chamada ao pressionar qualquer botão
void aoPressionarBotao() {
    int estadoPorta = PINB;
    if ((estadoPorta & 0x07) != 0x07) {
        contadorEEPROM = 5000;          // Reinicia o contador EEPROM ao pressionar um botão
        botaoPressionado = true;
    }
    if ((estadoPorta & 0x01) == 0) {
        seletorParametro = (seletorParametro + 1) % 3;  // Alterna entre escala de tensão, tempo e trigger
    }
    if ((estadoPorta & 0x04) == 0) {
        ajustarConfiguracao(1);         // Aumenta o valor do parâmetro selecionado
    }
    if ((estadoPorta & 0x02) == 0) {
        ajustarConfiguracao(-1);        // Diminui o valor do parâmetro selecionado
    }
    if ((estadoPorta & 0x08) == 0) {
        sinalPausado = !sinalPausado;   // Alterna o estado de pausa
    }
}

// Ajusta o parâmetro selecionado (tensão, tempo ou trigger)
void ajustarConfiguracao(int direcao) {
    if (seletorParametro == 0) {
        escalaTensao = constrain(escalaTensao + direcao, 0, 9);
    } else if (seletorParametro == 1) {
        escalaTempo = constrain(escalaTempo + direcao, 0, 9);
    } else if (seletorParametro == 2) {
        polaridadeTrigger = direcao > 0 ? 0 : 1;
    }
}

// Salva as configurações atuais na EEPROM após um certo tempo de inatividade
void salvarParaEEPROM() {
    if (contadorEEPROM > 0) {
        contadorEEPROM -= tempoExecucaoCiclo;
        if (contadorEEPROM <= 0) {
            EEPROM.write(0, escalaTensao);
            EEPROM.write(1, escalaTempo);
            EEPROM.write(2, polaridadeTrigger);
            EEPROM.write(3, seletorParametro);
        }
    }
}

// Carrega as configurações salvas na EEPROM ao iniciar o dispositivo
void carregarConfiguracoes() {
    int valorSalvo;
    valorSalvo = EEPROM.read(0);
    escalaTensao = (valorSalvo < 0 || valorSalvo > 9) ? 3 : valorSalvo;

    valorSalvo = EEPROM.read(1);
    escalaTempo = (valorSalvo < 0 || valorSalvo > 9) ? 3 : valorSalvo;

    valorSalvo = EEPROM.read(2);
    polaridadeTrigger = (valorSalvo < 0 || valorSalvo > 1) ? 1 : valorSalvo;

    valorSalvo = EEPROM.read(3);
    seletorParametro = (valorSalvo < 0 || valorSalvo > 2) ? 0 : valorSalvo;
}

// Exibe uma mensagem inicial no display ao ligar o dispositivo
void exibirMensagemInicial() {
    tela.clearDisplay();
    tela.setTextSize(2);
    tela.setCursor(10, 15);
    tela.println(F("Monitor"));
    tela.setCursor(10, 35);
    tela.println(F("Sinais v2.0"));
    tela.display();
    delay(1500);
    tela.clearDisplay();
}

// Captura dados do sinal analógico e armazena no buffer
void capturarSinal() {
    for (int i = 0; i < TAMANHO_BUFFER; i++) {
        bufferSinal[i] = analogRead(A0);  
        delayMicroseconds(calcularAtraso(escalaTempo));  
    }
    calcularEstatisticas();
}

// Calcula o atraso de captura com base na escala de tempo selecionada
int calcularAtraso(int tempoEscala) {
    const int ajustesTempo[] = {8000, 4000, 2000, 1000, 500, 250, 125, 62, 31, 15};
    return ajustesTempo[tempoEscala];
}

// Calcula o valor mínimo, máximo e a média do sinal capturado
void calcularEstatisticas() {
    int soma = 0;
    valorMinimoSinal = 1023;
    valorMaximoSinal = 0;

    for (int i = 0; i < TAMANHO_BUFFER; i++) {
        int valorAtual = bufferSinal[i];
        soma += valorAtual;
        if (valorAtual < valorMinimoSinal) valorMinimoSinal = valorAtual;
        if (valorAtual > valorMaximoSinal) valorMaximoSinal = valorAtual;
    }
    mediaSinal = soma / TAMANHO_BUFFER;
}

// Define o ponto de trigger para exibição sincronizada do sinal
void definirPontoTrigger() {
    int centroAmplitude = (valorMinimoSinal + valorMaximoSinal) / 2;
    triggerEncontrado = false;

    for (int i = TAMANHO_BUFFER / 2 - 50; i < TAMANHO_BUFFER / 2 + 50; i++) {
        if ((polaridadeTrigger == 0 && bufferSinal[i - 1] < centroAmplitude && bufferSinal[i] >= centroAmplitude) ||
            (polaridadeTrigger == 1 && bufferSinal[i - 1] > centroAmplitude && bufferSinal[i] <= centroAmplitude)) {
            pontoTrigger = i;
            triggerEncontrado = true;
            break;
        }
    }

    if (!triggerEncontrado) {
        pontoTrigger = TAMANHO_BUFFER / 2;  
    }
}

// Exibe informações do sinal, incluindo escala, frequência e ciclo ativo
void exibirDados() {
    tela.clearDisplay();
    tela.setTextSize(1);
    
    strcpy_P(escalaVertical, (char*)pgm_read_word(&(tabelaTensao[escalaTensao])));
    tela.setCursor(0, 0);
    tela.print("Escala T: ");
    tela.print(escalaVertical);

    strcpy_P(escalaHorizontal, (char*)pgm_read_word(&(tabelaTempo[escalaTempo])));
    tela.setCursor(64, 0);
    tela.print("Tempo: ");
    tela.print(escalaHorizontal);

    tela.setCursor(0, 20);
    if (triggerEncontrado) {
        calcularFrequencia();
        tela.print("Freq: ");
        tela.print(frequenciaSinal);
        tela.print(" Hz");

        tela.setCursor(0, 30);
        tela.print("Ciclo: ");
        tela.print(cicloAtivo);
        tela.print(" %");
    } else {
        tela.print("Sem sincronia");
    }

    tela.display();
}

// Calcula a frequência e o ciclo ativo do sinal
void calcularFrequencia() {
    int periodo = 0, duracaoAlta = 0;
    int centro = (valorMinimoSinal + valorMaximoSinal) / 2;

    for (int i = pontoTrigger; i < TAMANHO_BUFFER - 1; i++) {
        if (bufferSinal[i] >= centro && bufferSinal[i + 1] < centro) {
            periodo++;
        }
        if (bufferSinal[i] >= centro) {
            duracaoAlta++;
        }
    }

    float tempoCiclo = valoresEscalaTempo[escalaTempo] * periodo / 25.0;
    frequenciaSinal = (tempoCiclo > 0) ? (1.0 / tempoCiclo) : 0;
    cicloAtivo = (periodo > 0) ? (100.0 * duracaoAlta / periodo) : 0;
}

// Função principal de configuração (setup) do Arduino
void setup() {
    configurarDisplay();    
    configurarPinos();      
    configurarInterrupcoes();
}

// Função principal de execução (loop) do Arduino
void loop() {
    digitalWrite(13, HIGH);          // Ativa o LED indicador para início da captura
    capturarSinal();                 // Captura o sinal analógico
    definirPontoTrigger();           // Define o ponto de trigger do sinal
    digitalWrite(13, LOW);           // Desativa o LED indicador

    exibirDados();                   // Exibe os dados processados no display
    salvarParaEEPROM();              // Salva as configurações na EEPROM, se necessário

    // Exibe "PAUSADO" no display enquanto o sinal estiver pausado
    while (sinalPausado) {
        tela.setCursor(0, 40);
        tela.print("PAUSADO");
        tela.display();
        delay(100);
    }
}
