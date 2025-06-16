## 📌 IV - O que é um conversor de nível lógico?

Um **conversor de nível lógico** é um circuito que adapta sinais elétricos entre dispositivos que operam em diferentes tensões, como **3,3V** e **5V**.

---

## ⚠️ Por que é necessário?

Dispositivos como o **ESP32** ou **Raspberry Pi** operam em **3,3V**, enquanto a fita **WS2812B** geralmente requer sinais de **~5V** para funcionar de forma confiável.

Sem esse ajuste de nível:

- O sinal de dados de 3,3V pode não ser reconhecido corretamente pela fita.
- Isso pode causar efeitos como:
  - LEDs piscando incorretamente
  - Falhas intermitentes
  - LEDs travados

---

## ✅ Formas de conexão 

### 1. Módulo Conversor de Nível Lógico Bidirecional (MOSFET BSS138)

- Compatível com sinais de alta velocidade como o da WS2812B.
- Terminais:
  - **HV**: 5V da fonte
  - **LV**: 3,3V do ESP32
  - **GND**: comum entre ESP32 e fita
  - **LV1 → GPIO do ESP32**, **HV1 → DIN da fita WS2812B**

### 2. CI TTL Compatível: 74HCT245 ou 74HCT125

- Recomendado para buffers compatíveis com 3,3V.
- Usado para garantir a integridade do sinal em sistemas mais robustos.

### 3. Divisor Resistivo (não recomendado para WS2812B)

- Exemplo: Resistores de 1kΩ e 2kΩ.
- Reduz tensão de 5V para 3,3V, mas **não funciona bem para sinais rápidos** como os da WS2812B.

### 4. 🔌 Usando o Conversor de Nível Lógico IIC/I2C 

O módulo conhecido como **Conversor de Nível Lógico IIC/I2C bidirecional 5V ↔ 3.3V** (geralmente baseado no **MOSFET BSS138**) pode ser usado com segurança para **controlar fitas WS2812B** usando microcontroladores de 3,3V como o **ESP32**.

<p align="center">
  <img src="conversor.jpg" alt="conversor lógico" style="width:20%;">
</p>

Apesar de rotulado como “I2C”, esse módulo é um **conversor de nível lógico genérico**, ideal para sinais digitais de entrada/saída. O sinal da WS2812B:

- É **digital**
- É **unidirecional** (do ESP32 para a fita)
- Opera em **alta frequência (~800kHz)**

---

## 🔌 Exemplo de ligação

Use apenas **um canal** do conversor. Exemplo usando o canal **LV1/HV1**:

| Pino no Conversor | Conectar a                  |
|-------------------|-----------------------------|
| **HV**            | 5V da fonte da fita         |
| **LV**            | 3,3V do ESP32               |
| **GND**           | GND comum (ESP32 e fonte)   |
| **LV1**           | GPIO do ESP32 (ex: GPIO5)   |
| **HV1**           | DIN da fita WS2812B         |

> Os outros canais (LV2/HV2, etc.) não precisam ser usados.

---
