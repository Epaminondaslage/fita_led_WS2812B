# 🎛️ Controle de Fita LED WS2812B com ESP32 via Página Web

Este projeto utiliza um ESP32 para controlar uma fita de LED WS2812B (NeoPixel) através de uma página web responsiva, com botões para selecionar efeitos, ajustar a velocidade e o brilho em tempo real utilizando requisições AJAX.

[Fita LED WS2812B RGB 5V no AliExpress](https://pt.aliexpress.com/item/1005007989431712.html?srcSns=sns_Copy&spreadType=socialShare&bizType=ProductDetail&social_params=21851383776&aff_fcid=6abde431cf2c43d78abba54f8fe92092-1748561979491-00334-_mq1jYKF&tt=MG&aff_fsk=_mq1jYKF&aff_platform=default&sk=_mq1jYKF&aff_trace_key=6abde431cf2c43d78abba54f8fe92092-1748561979491-00334-_mq1jYKF&shareId=21851383776&businessType=ProductDetail&platform=AE&terminal_id=fb8fc465198c41748a4019f7189cdc36&afSmartRedirect=y)

---
## 📦 Requisitos

- ESP32-WROOM
- Fita LED WS2812B (5V)
- Fonte externa 5V (capaz de fornecer corrente suficiente)
- Arduino IDE com suporte à placa ESP32
- Biblioteca: [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)

--- 

## 🔧 Características principais

- **Tipo de LED:** WS2812B (com controlador embutido em cada LED)
- **Tensão de operação:** 5V DC
- **Comunicação:** 1 fio (protocolo serial)
- **Controle individual:** Cada LED pode ter cor e brilho próprios
- **Cores:** RGB (16 milhões de cores, controle PWM de 8 bits por canal)
- **Consumo:** ~60 mA por LED em brilho máximo (branco total)
- **Frequência de atualização:** Até 800 KHz
- **Endereçamento:** Dados em cascata (de um LED para o próximo)
- **Corte:** A cada LED ou grupo de LEDs (ver marcações na fita)

---

## 🛠️ Ligações básicas

| Fita WS2812B | Função                            |
|--------------|-----------------------------------|
| **+5V**       | Alimentação                       |
| **GND**       | Terra (referência)               |
| **DIN**       | Entrada de dados (microcontrolador → fita) |
| **DOUT**      | Saída de dados (para próxima fita) |

> ⚠️ **Atenção:** Para microcontroladores de 3,3V (ESP32, Raspberry Pi), use **conversor de nível lógico** para o sinal de dados.

--- 

## 🔧 Características da fita utilizada

**Escala Cinzenta:** 256  
**Bits/Cor:** 8 Bits/Cor  

## Largura do PCB:
- 30 LEDs/m — 10mm  
- 60 LEDs/m — 10mm  
- 74 LEDs/m — 10mm  
- 96 LEDs/m — 10mm  
- 144 LEDs/m — 12mm  

**Cor FPC:** Preto/Branco  

**Taxa de Proteção:** IP30 / IP65 / IP67  

**Cores:** RGB em cores, mudança de cor de sonho  

**Cortável:** Cada LED é cortável  
 
**Double-sided copper**  
  - Folha de cobre de 35μm usada, com boa condutividade e redução de tensão
 
## 🧠 Aplicações comuns

- Iluminação RGB de ambientes
- Decoração de festas e Natal
- Ambientes gamers (modding de PC)
- Painéis e letreiros visuais
- Robótica e cosplay com luzes dinâmicas
- Luzes reativas a som

## ✅ Vantagens

- Controle individual por LED
- Integração fácil com microcontroladores
- Flexível, cortável e expansível
- Suporte por bibliotecas (FastLED, NeoPixel)

## ❌ Desvantagens

- Precisa de alimentação 5V estável
- Sensível a ruídos em longas distâncias
- Pode exigir capacitor e resistor para proteção
---

# 🔄 Conversor de Nível Lógico para WS2812B com ESP32

## 📌 O que é um conversor de nível lógico?

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

## ✅ Soluções recomendadas

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

---

## 🔌 Exemplo de ligação

| Pino do Conversor | Conecta a                      |
|-------------------|--------------------------------|
| HV                | 5V da fonte externa            |
| LV                | 3,3V do ESP32                  |
| GND               | GND comum entre fonte e ESP32  |
| LV1               | GPIO do ESP32 (ex: GPIO5)      |
| HV1               | DIN da fita WS2812B            |

---

## 💡 Dica

Em alguns casos a fita WS2812B funciona com sinal de 3,3V diretamente, mas **isso não é garantido** e **pode falhar dependendo da fita, comprimento ou temperatura**.

**Sempre que possível, use um conversor.**

---

# Projeto para que o ESP32-WROOM controle a fita WS2812B por uma página web, com um botão HTML que alterna entre os efeitos (arco-íris, correr, teclado).

## 🔧 Conexões da fita ao ESP

| Fita WS2812B | ESP32                  |
|--------------|------------------------|
| DIN          | GPIO 5  (conversor)    |
| GND          | GND                    |
| +5V          | Fonte 5V               |

**Importante:** Conecte o GND da fonte e do ESP32 em comum.


## 🚀 Funcionalidades

- Servidor Web embutido no ESP32.
- Interface HTML moderna e responsiva (compatível com smartphones).
- Seis efeitos visuais:
  - Arco-íris contínuo
  - Correr (Knight Rider)
  - Teclado (preencher e apagar)
  - Piscada dupla
  - Chuva de LEDs
  - Branco estático
- Botões para:
  - Selecionar efeitos
  - Aumentar / diminuir **velocidade**
  - Aumentar / diminuir **brilho**
- Comunicação via AJAX (sem recarregar a página).

---

## 🌐 Interface Web

Acesse via navegador após o ESP32 se conectar à rede.  
Use o IP exibido no Serial Monitor, ex:

```plaintext
Conectado! IP: 192.168.0.105
```

## 📸 Controles disponíveis

- **Seção de efeitos:** Botões nomeados por efeito
- **Velocidade:** "Mais rápido" / "Mais lento"
- **Brilho:** "Aumentar" / "Diminuir"

---

## 🛠️ Como usar

1. Instale a biblioteca **Adafruit NeoPixel** na IDE do Arduino.
2. Substitua as credenciais de Wi-Fi no código:
   ```cpp
   const char* ssid = "SEU_SSID";
   const char* password = "SUA_SENHA";
   ```
3. Faça o upload do código para o ESP32.
4. Abra o navegador e acesse o IP mostrado no monitor serial.

---

## 📁 Estrutura do código

| Função                  | Descrição                                         |
|-------------------------|--------------------------------------------------|
| `setup()`               | Inicializa Wi-Fi, LEDs e servidor HTTP           |
| `loop()`                | Executa o efeito atual continuamente             |
| `/`                     | Rota principal: exibe a interface HTML           |
| `/efeito?id=N`          | Seleciona o efeito visual                        |
| `/velocidade?acao=...`  | Aumenta ou diminui a velocidade dos efeitos      |
| `/brilho?acao=...`      | Aumenta ou diminui o brilho da fita LED          |

---

## 📌 Dependências

- ESP32 core para Arduino
- Adafruit NeoPixel

Instale via: **Sketch > Incluir Biblioteca > Gerenciar Bibliotecas**


