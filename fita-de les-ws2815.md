# Fita de LED WS2815 

## Visão Geral

A fita LED digital endereçável WS2815 é uma solução RGB inteligente baseada em alimentação de 12 VDC, muito utilizada em projetos de automação, iluminação decorativa, painéis LED, efeitos visuais e integração com microcontroladores como ESP32, ESP8266 e Arduino.

A WS2815 é considerada uma evolução prática da WS2812B por oferecer:

* Alimentação em 12 V
* Menor queda de tensão
* Maior estabilidade em longas distâncias
* Sistema redundante de sinal (Backup Data)
* Maior robustez para instalações profissionais

---

# Características Técnicas

| Característica     | Valor                  |
| ------------------ | ---------------------- |
| Alimentação        | 12 VDC                 |
| Comunicação        | Serial digital 800 kHz |
| Controle           | Individual por LED     |
| Tipo               | RGB inteligente        |
| Backup de sinal    | Sim                    |
| Protocolo          | Compatível WS2812      |
| LEDs por metro     | 30 / 60 / 144          |
| Tensão lógica      | 3,3 V a 5 V            |
| Consumo aproximado | 0,3 W por LED          |

---

# Vantagens da WS2815

## Alimentação em 12 V

A principal vantagem da WS2815 é trabalhar com 12 V em vez de 5 V.

Isso proporciona:

* Menor queda de tensão
* Maior distância entre pontos de injeção
* Menor aquecimento dos cabos
* Melhor estabilidade
* Instalações maiores com menos problemas

---

## Backup de Sinal

A WS2815 possui entrada redundante de dados.

Mesmo se um LED falhar parcialmente, os LEDs seguintes continuam funcionando.

Isso é extremamente importante em:

* Eventos
* Instalações arquitetônicas
* Fachadas
* Painéis profissionais
* Projetos longos

---

# Pinagem da WS2815

```text
      _________
     |         |
 VDD |1     6| DO
 DIN |2     5| BO
 GND |3     4| BI
     |_______|
```

## Descrição dos Pinos

| Pino | Função                     |
| ---- | -------------------------- |
| VDD  | Alimentação 12 V           |
| GND  | Terra                      |
| DIN  | Entrada principal de dados |
| BI   | Entrada backup             |
| DO   | Saída principal de dados   |
| BO   | Saída backup               |

---

# Alimentação para 10 Metros de Fita

## Exemplo

* 10 metros
* 60 LEDs por metro
* Total: 600 LEDs

## Consumo aproximado

```text
600 LEDs × 0,3 W ≈ 180 W
```

## Fonte recomendada

| Item            | Recomendação |
| --------------- | ------------ |
| Tensão          | 12 VDC       |
| Corrente mínima | 15 A         |
| Corrente ideal  | 20 A a 25 A  |
| Potência ideal  | 240 W        |

---

# Esquema de Alimentação

```text
                FONTE 12 VDC
          +12V  ┌───────────────┐  GND
                │ 12 V / 20 A   │
                └──────┬────────┘
                       │
        ┌──────────────┼───────────────────── +12V
        │              │
        │              └───────────────────── GND
        │
        │
ESP32   │
┌────────────┐
│ GPIO 5     ├───[330 Ω]──── DI  fita WS2815
│ GND        ├────────────── GND comum
└────────────┘
                           início
                            │
                            ▼
       +12V ────────────────┬──────── fita 5 m ──────── fita 10 m
       GND  ────────────────┴───────────────────────────────
                             ▲                ▲
                             │                │
                     injeção no início   injeção no meio/final
```

---

# Injeção de Alimentação

Nunca alimente 10 metros de fita apenas por uma ponta.

## Recomendação

Injetar alimentação em:

* Início
* Meio
* Final

## Ligação correta

```text
Fonte 12 V +  ────────┬──────── +12V no início da fita
                      ├──────── +12V no meio da fita
                      └──────── +12V no final da fita

Fonte 12 V -  ────────┬──────── GND no início da fita
                      ├──────── GND no meio da fita
                      ├──────── GND no final da fita
                      └──────── GND do ESP32
```

---

# Componentes Recomendados

| Componente       | Recomendação             |
| ---------------- | ------------------------ |
| Fonte            | 12 V 20 A                |
| Capacitor        | 1000 µF a 2200 µF / 25 V |
| Resistor DATA    | 220 Ω a 470 Ω            |
| Conversor lógico | 74HCT125 ou 74HCT245     |
| Cabo alimentação | 2,5 mm² ou superior      |
| Fusível          | Recomendado              |

---

# Conversor Lógico

O ESP32 trabalha com 3,3 V.

Embora algumas fitas aceitem diretamente, o correto é usar conversor lógico.

## Circuito recomendado

```text
ESP32 GPIO 5 ───── entrada 74HCT125
74HCT125 saída ───[330 Ω]──── DI WS2815

ESP32 GND ─────────────┐
Fonte GND ─────────────┼──── GND fita
74HCT125 GND ──────────┘

Fonte 5 V ou regulador buck 5 V ─── VCC do 74HCT125
Fonte 12 V ─────────────────────── +12V da fita
```

---

# Fonte Correta para WS2815

A WS2815 utiliza fonte de tensão constante.

## Correto

* Fonte chaveada 12 VDC
* Fonte industrial
* Fonte metálica tipo colmeia

## Não utilizar

* Driver de corrente constante
* Fonte 24 V
* Carregador USB
* Fonte subdimensionada

---

# Exemplo para 480 LEDs

## Cálculo

```text
480 LEDs × 0,3 W ≈ 144 W
144 W ÷ 12 V ≈ 12 A
```

## Fonte recomendada

| Configuração   | Valor       |
| -------------- | ----------- |
| Mínimo         | 12 V / 15 A |
| Ideal          | 12 V / 20 A |
| Potência ideal | 240 W       |

---

# Bibliotecas Compatíveis

## FastLED

[https://fastled.io/](https://fastled.io/)

## Adafruit NeoPixel

[https://github.com/adafruit/Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)

## WLED

[https://kno.wled.ge/](https://kno.wled.ge/)

---

# Exemplo com FastLED

```cpp
#include <FastLED.h>

#define LED_PIN     5
#define NUM_LEDS    60

CRGB leds[NUM_LEDS];

void setup() {
    FastLED.addLeds<WS2815, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {

    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;
    }

    FastLED.show();
    delay(1000);

    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
    }

    FastLED.show();
    delay(1000);
}
```

---

# Diferença entre WS2812B e WS2815

| Característica     | WS2812B   | WS2815      |
| ------------------ | --------- | ----------- |
| Alimentação        | 5 V       | 12 V        |
| Queda de tensão    | Alta      | Menor       |
| Backup de sinal    | Não       | Sim         |
| Robustez           | Média     | Alta        |
| Instalações longas | Limitadas | Excelente   |
| Uso profissional   | Médio     | Muito comum |

---

# Boas Práticas

* Utilizar GND comum
* Não alimentar pelo ESP32
* Usar cabos grossos
* Fazer injeção de alimentação
* Utilizar fusível
* Utilizar capacitor na entrada
* Utilizar resistor no DATA
* Utilizar fonte com margem de potência
* Evitar conexões frouxas
* Utilizar dissipação adequada na fonte

---

# Datasheets e Manuais

## Datasheet Oficial

[https://www.alldatasheet.com/datasheet-pdf/pdf/1667393/WORLDSEMI/WS2815.html](https://www.alldatasheet.com/datasheet-pdf/pdf/1667393/WORLDSEMI/WS2815.html)

## Manual Técnico

[https://www.superlightingled.com/PDF/WS2815-12v-addressable-led-chip-specification-.pdf](https://www.superlightingled.com/PDF/WS2815-12v-addressable-led-chip-specification-.pdf)

## Manual BTF-Lighting

[https://device.report/manual/18263308](https://device.report/manual/18263308)

---

# Aplicações Comuns

* Automação residencial
* Iluminação decorativa
* Painéis LED
* Fachadas
* Ambilight
* Eventos
* Cenografia
* Integração com MQTT
* Home Assistant
* WLED
* ESP32
* Tasmota
* Efeitos musicais
* DMX512

---

# Conclusão

A WS2815 é atualmente uma das melhores opções para projetos RGB endereçáveis de médio e grande porte.

A alimentação em 12 V e o sistema de backup de sinal tornam a fita muito mais robusta e confiável para aplicações profissionais e instalações longas.

Para projetos com ESP32, recomenda-se:

* Fonte 12 V bem dimensionada
* Conversor lógico
* Injeção de alimentação
* Cabos adequados
* Proteção elétrica básica

Com isso, é possível construir sistemas extremamente estáveis e com excelente qualidade visual.
