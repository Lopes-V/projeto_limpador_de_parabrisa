#include <Arduino.h>
#include <ESP32Servo.h>
#include <DHT.h>

#define BOTAO 18
#define SERVO_PIN 13
#define DHT_PIN 4
#define DHT_TYPE DHT11

// Configuracao dos limites e velocidade do movimento do servo
const int ANGULO_ESQUERDA = 20;
const int ANGULO_DIREITA = 160;
const unsigned long TEMPO_DO_CURSO = 600;

// Tempo usado para evitar que um clique seja detectado varias vezes
const unsigned long TEMPO_DEBOUNCE = 50;

// Configuracao dos limites de umidade e intervalo de leitura do DHT11
const float UMIDADE_PARA_LIGAR = 20.0;
const float UMIDADE_PARA_DESLIGAR = 0;
const unsigned long INTERVALO_LEITURA_DHT = 2000;

// Criacao dos objetos responsaveis pelo servo e pelo DHT11
Servo servo;
DHT dht(DHT_PIN, DHT_TYPE);

// Variaveis que guardam o estado atual do sistema
bool ligado = false;
bool ligadoPeloBotao = false;
bool ligadoPeloSensor = false;
bool sensorAutomaticoBloqueado = false;

// Guarda o destino atual do servo e quando o movimento começou
int destinoServo = ANGULO_ESQUERDA;
unsigned long inicioDoCurso = 0;

// Variaveis usadas para controlar corretamente o clique do botao
int ultimaLeituraBotao = HIGH;
int estadoEstavelBotao = HIGH;
unsigned long inicioDoDebounce = 0;

// Guarda quando foi feita a ultima leitura do DHT11
unsigned long ultimaLeituraDht = 0;


// Liga ou desliga o limpador e movimenta o servo para a posicao correta
void definirEstadoLimpador(bool novoEstado, const char *origem) {
    if (novoEstado == ligado) {
        return;
    }

    ligado = novoEstado;

    Serial.print("Limpador: ");
    Serial.print(ligado ? "LIGADO" : "DESLIGADO");
    Serial.print(" (");
    Serial.print(origem);
    Serial.println(")");

    if (ligado) {
        destinoServo = ANGULO_DIREITA;
        servo.write(destinoServo);
        inicioDoCurso = millis();
    } else {
        destinoServo = ANGULO_ESQUERDA;
        servo.write(destinoServo);
    }
}


// Verifica se o limpador deve ficar ligado pelo botao ou pelo sensor
void atualizarEstadoLimpador(const char *origem) {
    definirEstadoLimpador(
        ligadoPeloBotao || ligadoPeloSensor,
        origem
    );
}


// Alterna o limpador entre ligado e desligado quando o botao e pressionado
void alternarLimpadorPeloBotao() {
    if (ligado) {
        ligadoPeloBotao = false;
        ligadoPeloSensor = false;
        sensorAutomaticoBloqueado = true;
    } else {
        ligadoPeloBotao = true;
    }

    atualizarEstadoLimpador("botao");
}


// Le o botao e aplica debounce para considerar apenas um clique por toque
void atualizarBotao() {
    int leitura = digitalRead(BOTAO);

    if (leitura != ultimaLeituraBotao) {
        inicioDoDebounce = millis();
        ultimaLeituraBotao = leitura;
    }

    if (
        millis() - inicioDoDebounce >= TEMPO_DEBOUNCE &&
        leitura != estadoEstavelBotao
    ) {
        estadoEstavelBotao = leitura;

        // Como usamos INPUT_PULLUP, LOW significa botao pressionado
        if (estadoEstavelBotao == LOW) {
            alternarLimpadorPeloBotao();
        }
    }
}


// Le a umidade do DHT11 e decide se o modo automatico deve ligar ou desligar
void atualizarSensor() {
    if (millis() - ultimaLeituraDht < INTERVALO_LEITURA_DHT) {
        return;
    }

    ultimaLeituraDht = millis();

    float umidade = dht.readHumidity();

    if (isnan(umidade)) {
        Serial.println("Falha ao ler o DHT11");
        return;
    }

    Serial.print("Umidade: ");
    Serial.print(umidade, 1);
    Serial.println(" %");

    if (umidade <= UMIDADE_PARA_DESLIGAR) {
        ligadoPeloSensor = false;
        sensorAutomaticoBloqueado = false;
    } 
    else if (
        umidade >= UMIDADE_PARA_LIGAR &&
        !sensorAutomaticoBloqueado
    ) {
        ligadoPeloSensor = true;
    }

    atualizarEstadoLimpador("sensor");
}


// Faz o servo alternar continuamente entre esquerda e direita
void atualizarServo() {
    if (!ligado) {
        return;
    }

    if (millis() - inicioDoCurso >= TEMPO_DO_CURSO) {

        destinoServo =
            (destinoServo == ANGULO_DIREITA)
                ? ANGULO_ESQUERDA
                : ANGULO_DIREITA;

        servo.write(destinoServo);

        inicioDoCurso = millis();
    }
}


// Executado uma vez ao ligar o ESP32 e configura todos os componentes
void setup() {
    Serial.begin(115200);

    pinMode(BOTAO, INPUT_PULLUP);

    servo.setPeriodHertz(50);
    servo.attach(SERVO_PIN, 500, 2400);
    servo.write(ANGULO_ESQUERDA);

    dht.begin();
}


// Executado continuamente e atualiza botao, sensor e servo
void loop() {
    atualizarBotao();
    atualizarSensor();
    atualizarServo();
}