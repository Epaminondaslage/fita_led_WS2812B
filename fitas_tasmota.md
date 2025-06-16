
# 💡 Controle de Fitas de LED Endereçáveis com Tasmota e ESP32-WROOM

Uso do firmware **Tasmota** em um **ESP32-WROOM** para controlar fitas de LED endereçáveis (individuais), como **WS2812B**, **WS2815**, **SK6812**, entre outras, com controle via interface web, MQTT ou comandos diretos.

---

## 🧩 Requisitos de Hardware

- ✅ ESP32-WROOM com Tasmota (firmware Tasmota32)
- ✅ Fita de LED endereçável (WS2812B, WS2815, SK6812, etc.)
- ✅ Fonte de alimentação adequada:
  - WS2812B/SK6812 → 5V
  - WS2815/WS2811 → 12V
- ✅ Conexão de dados entre GPIO e a fita
- ⚠️ Recomendado:
  - Capacitor 1000 µF / 6.3V ou superior (protege contra picos de corrente)
  - Resistor de 330 Ω no fio de dados (protege contra ruído)
  - Conversor de nível lógico (3.3V para 5V) para sinal de dados, se necessário

---

## 📦 Modelos de Fita Compatíveis

| Modelo     | Tensão | RGB/RGBW | Controle | Compatível com Tasmota32 |
|------------|--------|----------|----------|---------------------------|
| WS2812B    | 5V     | RGB      | 1 fio    | ✅ Sim                    |
| WS2811     | 12V    | RGB      | 1 fio    | ✅ Sim (grupo de 3 LEDs) |
| WS2815     | 12V    | RGB      | 1 fio    | ✅ Sim                    |
| SK6812     | 5V     | RGBW     | 1 fio    | ✅ Sim                    |

---

## 🛠️ Passo a Passo de Configuração (ESP32-WROOM)

### 1. ✅ Grave o Tasmota no ESP32

Use o firmware **Tasmota32-light.bin** (leve e com suporte a luzes) ou **Tasmota32-display.bin** se quiser suporte a displays.

Instalação recomendada:
- Via Tasmotizer ou ESP Flasher
- Repositório oficial: https://github.com/arendst/Tasmota/releases

---

### 2. ⚙️ Configure o GPIO da fita LED

Acesse a interface web do ESP32-Tasmota (ex: `http://192.168.1.50`):

1. Vá em **Configuração → Configurar Hardware**
2. Escolha um GPIO disponível e defina como:

```
WS2812 (i.e. LED Strip)
```

Exemplo recomendado:
```
GPIO12 → WS2812
```

3. Salve e reinicie o dispositivo.

---

### 3. 💡 Configure o número de LEDs

No console da interface web, digite:

```text
Pixels 60
```

*(Substitua 60 pela quantidade real de LEDs na sua fita)*

---

### 4. 🎨 Controle de Cores

No **console** ou via **MQTT**, envie:

```text
Color 255000000        → Vermelho
Color 000255000        → Verde
Color 000000255        → Azul
Color 255255255        → Branco
Color 000              → Apagar LED
```

---

### 5. 🌈 Efeitos (Schemes)

```text
Scheme 0  → Sem efeito (manual)
Scheme 1  → Mudança suave de cor
Scheme 2  → Piscar
Scheme 3  → Fade automático
Scheme 4  → Cores aleatórias
```

Envie via console:

```
Scheme 3
```

---

### 6. ⚙️ Outras configurações úteis

```text
SetOption15 1         → Permite controle direto RGB
LedTable 0            → Remove correção de curva de cores
```

#### Para fitas RGBW (como SK6812):

```text
SetOption37 128       → Ativa canal branco
```

---

## 📡 Controle via MQTT

Tasmota publica/subscreve em tópicos MQTT.

### Envio de comando MQTT:

```text
Tópico: cmnd/nome_do_dispositivo/Color
Payload: 255255000
```

### Efeitos via MQTT:

```text
cmnd/nome_do_dispositivo/Scheme 4
```

---

## ⚠️ Cuidados com alimentação

| LEDs (exemplo) | Corrente estimada | Fonte sugerida |
|----------------|-------------------|----------------|
| 30 LEDs        | ~1.8A             | 5V 2A          |
| 60 LEDs        | ~3.6A             | 5V 4A          |
| 120 LEDs       | ~7.2A             | 5V 8A          |

> 💡 Use alimentação externa potente e sempre conecte **GND da fonte com GND do ESP32**.

---

## 🧰 Esquema de ligação (ESP32-WROOM + WS2812B)

```
ESP32 GPIO12 ---[330Ω]---> DIN da fita LED
ESP32 GND ----------------> GND da fonte
FITA WS2812B +5V --------> Fonte 5V
FITA WS2812B GND --------> GND da fonte
```

---

## 🔌 Exemplo com ESP32-WROOM + Fita WS2815 (12V)

A WS2815 é uma versão melhorada da WS2812, com **tensão de 12V** (ideal para grandes comprimentos) e **linha de dados redundante**. Cada LED continua sendo controlado individualmente via 1 fio de dados.

---

### ⚠️ Características da WS2815

| Propriedade        | Valor                        |
|--------------------|------------------------------|
| Tensão             | 12V                          |
| Tipo de controle   | 1 fio de dados (DIN)         |
| Corrente por LED   | ~60 mA (máximo brilho RGB)   |
| Redundância de sinal | Sim (continua se 1 LED falhar) |
| Compatível com Tasmota | ✅ Sim (via `WS2812` config) |

---

### 🧰 Ligações recomendadas

| Conexão           | Ligação                          |
|-------------------|----------------------------------|
| Fio de dados      | GPIO12 do ESP32 (com resistor 330Ω) |
| GND do ESP32      | GND da fonte 12V e GND da fita   |
| Alimentação fita  | 12V direto da fonte (não pelo ESP!) |

---

### 💡 Exemplo de configuração no Tasmota

Configure o GPIO como:

```
GPIO12 → WS2812 (i.e. LED Strip)
```

Mesmo sendo WS2815, o driver é o mesmo do WS2812.

---

### 📟 Defina o número de LEDs

```text
Pixels 60
```

---

### 🎨 Teste de cores e efeitos

No console:

```text
Color 255000000
Color 000255000
Color 000000255
Scheme 3
```

---

### 🧪 Alimentação da WS2815

- 60 LEDs podem consumir até **18W** (1.5A em 12V)
- Fonte recomendada: **12V 3A ou mais**
- Use capacitor de **1000µF / 25V** entre 12V e GND perto do início da fita

---

### 📷 Esquema resumido

```
ESP32 GPIO12 ---[330Ω]---> DIN da fita WS2815
ESP32 GND ----------------> GND da fonte
FITA WS2815 +12V --------> Fonte 12V
FITA WS2815 GND ---------> GND da fonte
```

---

## 📚 Links úteis

- Site oficial do Tasmota: https://tasmota.github.io
- Lista de comandos: https://tasmota.github.io/docs/Commands
- Documentação de LEDs: https://tasmota.github.io/docs/Lights/

---

## ✅ Conclusão

Com o ESP32-WROOM e Tasmota, você pode controlar fitas de LED endereçáveis com:

- Interface web integrada
- Controle MQTT local (sem nuvem)
- Efeitos, cores e animações
- Compatibilidade com automações (Home Assistant, Node-RED, etc.)

> 💡 Uma solução poderosa, sem programação adicional, ideal para automação residencial e projetos de iluminação inteligentes.
