#include <Arduino.h>
#include <ESP32Servo.h>
#include <DHT.h>

// Definicao dos pinos de conexao dos componentes
#define PINO_BOTAO 18
#define PINO_SERVO 13
#define PINO_DHT 4
#define TIPO_SENSOR_DHT DHT11

// Configuracao dos limites e velocidade de movimento da palheta
const int ANGULO_ESQUERDA_REPOUSO = 20;
const int ANGULO_DIREITA_MAXIMO = 160;
const unsigned long TEMPO_CURSO_PALHETA_MS = 600;

// Tempo de filtro (debounce) para ignorar ruidos eletricos no clique do botao
const unsigned long TEMPO_DEBOUNCE_BOTAO_MS = 50;

// Limites de umidade e intervalo entre leituras do sensor DHT11
const float LIMIAR_UMIDADE_LIGAR = 20.0;
const float LIMIAR_UMIDADE_DESLIGAR = 0.0;
const unsigned long INTERVALO_LEITURA_DHT_MS = 2000;

// Criacao dos objetos de controle do servo motor e do sensor DHT
Servo servoLimpador;
DHT sensorDht(PINO_DHT, TIPO_SENSOR_DHT);

// Variaveis de estado do sistema
bool limpadorAtivo = false;
bool ativadoManualmente = false;
bool ativadoPorSensor = false;
bool sensorAutomaticoBloqueado = false;

// Variaveis de controle do movimento do servo
int anguloDestinoServo = ANGULO_ESQUERDA_REPOUSO;
unsigned long tempoInicioMovimentoMs = 0;

// Variaveis de leitura e filtro do botao
int ultimaLeituraPinoBotao = HIGH;
int estadoConfirmadoBotao = HIGH;
unsigned long tempoInicioDebounceMs = 0;

// Variavel de temporizacao da leitura do sensor DHT
unsigned long tempoUltimaLeituraDhtMs = 0;


// Liga ou desliga o limpador e posiciona o servo motor
void definirEstadoLimpador(bool novoEstado, const char *origem) {
    if (novoEstado == limpadorAtivo) {
        return;
    }

    limpadorAtivo = novoEstado;

    Serial.print("Limpador: ");
    Serial.print(limpadorAtivo ? "LIGADO" : "DESLIGADO");
    Serial.print(" (");
    Serial.print(origem);
    Serial.println(")");

    if (limpadorAtivo) {
        anguloDestinoServo = ANGULO_DIREITA_MAXIMO;
        servoLimpador.write(anguloDestinoServo);
        tempoInicioMovimentoMs = millis();
    } else {
        anguloDestinoServo = ANGULO_ESQUERDA_REPOUSO;
        servoLimpador.write(anguloDestinoServo);
    }
}


// Verifica se o limpador deve ficar ligado pelo botao OU pelo sensor
void atualizarEstadoLimpador(const char *origem) {
    definirEstadoLimpador(
        ativadoManualmente || ativadoPorSensor,
        origem
    );
}


// Alterna entre ligar ou desligar quando o botao fisico e pressionado
void alternarLimpadorPeloBotao() {
    if (limpadorAtivo) {
        ativadoManualmente = false;
        ativadoPorSensor = false;
        sensorAutomaticoBloqueado = true; // Bloqueia religamento automatico imediato
    } else {
        ativadoManualmente = true;
    }

    atualizarEstadoLimpador("botao");
}


// Le o pino do botao e filtra ruidos (debounce) para detectar um clique valido
void atualizarBotao() {
    int leituraAtual = digitalRead(PINO_BOTAO);

    if (leituraAtual != ultimaLeituraPinoBotao) {
        tempoInicioDebounceMs = millis();
        ultimaLeituraPinoBotao = leituraAtual;
    }

    if (
        millis() - tempoInicioDebounceMs >= TEMPO_DEBOUNCE_BOTAO_MS &&
        leituraAtual != estadoConfirmadoBotao
    ) {
        estadoConfirmadoBotao = leituraAtual;

        // Como usamos INPUT_PULLUP, LOW significa que o botao foi pressionado
        if (estadoConfirmadoBotao == LOW) {
            alternarLimpadorPeloBotao();
        }
    }
}


// Le a umidade do ar e aciona o limpador automaticamente se necessario
void atualizarSensor() {
    if (millis() - tempoUltimaLeituraDhtMs < INTERVALO_LEITURA_DHT_MS) {
        return;
    }

    tempoUltimaLeituraDhtMs = millis();

    float umidadeAtual = sensorDht.readHumidity();

    if (isnan(umidadeAtual)) {
        Serial.println("Falha ao ler o sensor DHT11");
        return;
    }

    Serial.print("Umidade: ");
    Serial.print(umidadeAtual, 1);
    Serial.println(" %");

    if (umidadeAtual <= LIMIAR_UMIDADE_DESLIGAR) {
        ativadoPorSensor = false;
        sensorAutomaticoBloqueado = false;
    } 
    else if (
        umidadeAtual >= LIMIAR_UMIDADE_LIGAR &&
        !sensorAutomaticoBloqueado
    ) {
        ativadoPorSensor = true;
    }

    atualizarEstadoLimpador("sensor");
}


// Faz a palheta oscilar continuamente de um lado para o outro
void atualizarServo() {
    if (!limpadorAtivo) {
        return;
    }

    if (millis() - tempoInicioMovimentoMs >= TEMPO_CURSO_PALHETA_MS) {

        anguloDestinoServo =
            (anguloDestinoServo == ANGULO_DIREITA_MAXIMO)
                ? ANGULO_ESQUERDA_REPOUSO
                : ANGULO_DIREITA_MAXIMO;

        servoLimpador.write(anguloDestinoServo);

        tempoInicioMovimentoMs = millis();
    }
}


// Executado uma unica vez ao ligar o ESP32 para inicializar os perifericos
void setup() {
    Serial.begin(115200);

    pinMode(PINO_BOTAO, INPUT_PULLUP);

    servoLimpador.setPeriodHertz(50);
    servoLimpador.attach(PINO_SERVO, 500, 2400);
    servoLimpador.write(ANGULO_ESQUERDA_REPOUSO);

    sensorDht.begin();
}


// Loop principal que executa repetidamente as rotinas do sistema
void loop() {
    atualizarBotao();
    atualizarSensor();
    atualizarServo();
}