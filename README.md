# Limpador de Para-brisa Automático com ESP32

Projeto desenvolvido com **ESP32**, **servo motor SG90**, **botão** e **sensor DHT11** para simular o funcionamento de um limpador de para-brisa automático.

O sistema permite controlar o limpador manualmente pelo botão e também realizar o acionamento automático com base na umidade detectada pelo DHT11.

## Funcionalidades

* Acionamento manual por botão.
* Um clique liga o limpador.
* Outro clique desliga o limpador.
* Movimento contínuo do servo entre dois ângulos.
* Monitoramento da umidade com DHT11.
* Acionamento automático conforme o nível de umidade.
* Exibição das informações pelo Serial Monitor.
* Tratamento de debounce do botão.

## Componentes

* ESP32 DevKit
* Protoboard
* Servo motor SG90
* Sensor DHT11 com módulo
* Botão push button
* Jumpers
* Cabo USB

## Ligações

### Botão

| Botão  | ESP32   |
| ------ | ------- |
| Pino 1 | GPIO 18 |
| Pino 2 | GND     |

O botão utiliza `INPUT_PULLUP`, portanto não é necessário resistor externo.

### Servo SG90

| SG90               | ESP32    |
| ------------------ | -------- |
| Vermelho           | VIN / 5V |
| Marrom ou preto    | GND      |
| Laranja ou amarelo | GPIO 13  |

### DHT11

| DHT11    | ESP32  |
| -------- | ------ |
| VCC / +  | 3.3V   |
| GND / -  | GND    |
| DATA / S | GPIO 4 |

## Funcionamento

O limpador possui dois modos de acionamento.

### Modo manual

Ao pressionar o botão uma vez:

```text
DESLIGADO → LIGADO
```

O servo passa a movimentar o limpador continuamente:

```text
20° → 160° → 20° → 160° → ...
```

Ao pressionar novamente:

```text
LIGADO → DESLIGADO
```

O servo retorna para a posição inicial.

### Modo automático

O DHT11 realiza leituras periódicas da umidade.

Quando o valor configurado para acionamento é atingido, o sistema pode ligar automaticamente o limpador.

Os valores podem ser alterados no código:

```cpp
const float UMIDADE_PARA_LIGAR = 20.0;
const float UMIDADE_PARA_DESLIGAR = 0;
```

## Tecnologias utilizadas

* C++
* Arduino Framework
* PlatformIO
* VS Code
* ESP32

## Bibliotecas

O projeto utiliza:

```text
ESP32Servo
DHT sensor library
Adafruit Unified Sensor
```

No `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

monitor_speed = 115200

lib_deps =
    madhephaestus/ESP32Servo
    adafruit/DHT sensor library
    adafruit/Adafruit Unified Sensor
```

## Executando o projeto

Clone o repositório:

```bash
git clone URL_DO_REPOSITORIO
```

Entre na pasta:

```bash
cd NOME_DO_REPOSITORIO
```

Abra no VS Code:

```bash
code .
```

Com a extensão PlatformIO instalada, conecte o ESP32 ao computador e execute:

```text
PlatformIO → Upload
```

Depois abra:

```text
PlatformIO → Monitor
```

O Serial Monitor deve estar configurado para:

```text
115200 baud
```

## Estrutura básica

```text
projeto/
├── src/
│   └── main.cpp
├── include/
├── lib/
├── test/
├── platformio.ini
└── README.md
```

## Observações

O DHT11 mede **temperatura e umidade do ar**, portanto não detecta chuva diretamente.

Para uma versão mais próxima de um limpador de para-brisa real, o projeto pode futuramente utilizar um **sensor de chuva** dedicado.

Também é recomendado utilizar uma fonte externa de 5 V para o SG90 caso o servo apresente travamentos, tremores ou reinicializações do ESP32 devido ao consumo de corrente.
