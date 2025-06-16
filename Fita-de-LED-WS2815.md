## VIII. Fita de LED WS2815

A **fita WS2815** é um tipo de fita de LED digital endereçável que oferece vantagens significativas sobre modelos anteriores como a WS2812B, sendo ideal para projetos de iluminação programável, decoração, automação e efeitos visuais complexos.

---

## 🔧 Características Técnicas

| Característica         | Detalhes                                                      |
|------------------------|---------------------------------------------------------------|
| **Modelo**             | WS2815                                                         |
| **Tipo de LED**        | SMD 5050 RGB com controle individual                           |
| **Tensão de operação** | **12V DC**                                                     |
| **Dados de controle**  | Protocolo serial com único fio de dados (mais fio de backup)  |
| **Nível lógico**       | 5V (entrada de dados)                                          |
| **PWM**                | ~2kHz (sem cintilação visível a olho nu)                      |
| **Redundância**        | Linha de dados dupla (continua operando mesmo se um LED falhar)|
| **Densidade típica**   | 30, 60, 96 ou 144 LEDs por metro                               |

---

## ✅ Vantagens da WS2815

- **Alimentação em 12V**: reduz quedas de tensão em fitas longas.
- **Resiliência a falhas**: com dois fios de dados, continua operando mesmo que um LED pare.
- **Controle independente**: cada LED pode ser configurado com cor e brilho únicos.
- **Mais robusta para instalações grandes**: menos perda por cabo, ideal para grandes projetos.

---

## ⚠️ Cuidados ao Usar

- **Alimentação adequada**:
  - Cada LED pode consumir até **0,3W** em branco total.
  - Exemplo: 60 LEDs consomem até **18W (~1,5A a 12V)**.
  - Fonte de qualidade é essencial para evitar instabilidade.

- **Capacitor recomendado**:
  - Coloque um **capacitor de 1000 µF / 16V** entre +12V e GND perto da fita.

- **Resistor no pino de dados**:
  - Use **330Ω** entre o microcontrolador e a fita para proteger o primeiro LED.

- **Nível lógico do sinal**:
  - A fita **espera 5V no sinal de dados**, mas ESP8266 e ESP32 usam 3,3V.
  - **Pode funcionar com 3,3V**, mas o ideal é usar um **conversor de nível lógico** (3,3V → 5V).

---

## 🔌 Alimentação e Conversão de Nível Lógico

Apesar da fita usar **12V para os LEDs**, o **sinal de dados é de 5V**. Para compatibilidade com microcontroladores modernos (como o ESP32 ou ESP8266, que operam com 3,3V), recomenda-se:

### ✅ Esquema de alimentação com uma única fonte:

- Use **uma fonte 12V** para:
  - Alimentar a fita diretamente.
  - Alimentar um **regulador step-down (buck)** que forneça 5V (para o lado "alto" do conversor de nível lógico).
  - Alimentar o ESP, se necessário.

### 🧠 Esquema de ligação resumido:

```
[Fonte 12V] ─┬─> Fita WS2815 (+12V / GND)
             ├─> Step-down 12V → 5V ──> HV do conversor
             └─> ESP32 / ESP8266 (via regulador ou USB)

ESP 3,3V ─────> LV do conversor
GND compartilhado entre todos os módulos
Saída do conversor ──> Pino de dados da WS2815
```

- **GNDs devem estar todos interligados.**
- O conversor usa 3,3V do ESP (LV) e 5V do step-down (HV).

---

## 📡 Compatibilidade com Tasmota

A fita  WS2815 pode ser usada com **Tasmota**, desde que:

### ✅ Requisitos:

- Microcontrolador com Tasmota instalado (ESP8266/ESP32)
- Tasmota com suporte a **FastLED / WS2812**
- Alimentação 12V para a fita
- Conversor de nível lógico (opcional, mas recomendado)

### ⚙️ Configuração no Tasmota:

1. **Acesse o Web UI do Tasmota**
2. Vá em **Configuration → Configure Module**
3. Selecione `Generic (18)`
4. Configure o GPIO como `WS2812 (4)`
5. Salve e reinicie

### 📜 Comandos no console:

```
LedTable 0            # Desativa mapeamento de brilho
SetOption15 1         # Ativa controle por cor direta
LedMode 1             # Modo de controle
PixelCount 60         # Número de LEDs na fita
```

### 📡 Exemplo de controle via MQTT:

```
cmnd/tasmota_WS2815/Color FF0000   # Todos os LEDs em vermelho
cmnd/tasmota_WS2815/Dimmer 50      # Brilho em 50%
```

---

## 💡 Aplicações Típicas

- Ambientes decorativos RGB
- Sinalização dinâmica
- Ambientes gamers e estúdios de vídeo
- Painéis de LED e obras de arte interativa
- Efeitos luminosos sincronizados com som
- Fachadas iluminadas e shows

---

## 🧾 Tabela Comparativa

| Característica              | WS2812B                         | WS2815                            |
|----------------------------|----------------------------------|-----------------------------------|
| **Tensão de Alimentação**  | 5V                               | **12V**                           |
| **Consumo por LED (RGB máx)** | ~60 mA                        | ~12 mA                            |
| **Número de Canais**       | 1 linha de dados (Data In/Out)  | **2 linhas** (Data + Backup)      |
| **Tolerância a Falhas**    | Um LED queimado interrompe tudo | **Linha de backup mantém funcionamento** |
| **Distância de Alimentação** | Baixa (perda de tensão rápida) | **Alta (perda menor com 12V)**    |
| **Compatibilidade**        | Alta (muito usada)              | Alta (usa o mesmo protocolo WS281x) |
| **Resistência a Ruído**    | Média                           | **Maior robustez**                |
| **Uso Recomendado**        | Projetos curtos (<2m), baratos  | **Instalações maiores e profissionais** |

---

## ⚠️ Considerações de Instalação

### WS2812B:
- Ideal para **curtas distâncias**
- Alimentação com **5V**
- Se um LED falhar, a fita inteira após ele para de funcionar
- Simples de usar com ESP32 (pode usar direto GPIO)

### WS2815:
- **Alimentação 12V** → permite **menos injeções de energia**
- Requer **conversor de nível lógico (3.3V → 5V)** para uso com ESP32
- Linha de **backup (BI)** evita falhas completas
- Perfeita para **projetos longos e resistentes a falhas**

---

## ✅ Recomendação

| Tipo de Projeto                           | Melhor escolha |
|------------------------------------------|----------------|
| Protótipo simples, portátil               | WS2812B        |
| Instalação longa (3m+), externa ou robusta| **WS2815**     |
| Ambientes industriais ou com ruído        | **WS2815**     |

---

## 📎 Observações

- Sempre conecte o **GND da fonte ao GND do ESP32**
- Use capacitor de 1000 µF entre V+ e GND da fita
- Use resistor de 330Ω no fio de dados para proteção
- Para longas distâncias, injete alimentação a cada 2-3 metros

---
