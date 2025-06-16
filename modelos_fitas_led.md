# X- Comparativo de Fitas de LED Endereçáveis

Fitas de LED endereçáveis permitem o controle individual de cada LED, criando efeitos dinâmicos como arco-íris, animações, transições e muito mais. Abaixo está um comparativo entre os modelos mais populares.

---

## 📋 Principais Modelos

| Modelo      | IC (Driver)    | Tensão | Comunicação | Canais por LED | RGB ou RGBW | Popularidade |
|-------------|----------------|--------|-------------|----------------|-------------|--------------|
| WS2812B     | WS2812         | 5V     | 1 fio (PWM) | 3              | RGB         | ⭐⭐⭐⭐⭐       |
| WS2811      | WS2811         | 12V    | 1 fio (PWM) | 3 (grupo de 3) | RGB         | ⭐⭐⭐⭐        |
| SK6812      | SK6812         | 5V     | 1 fio (PWM) | 4              | RGBW        | ⭐⭐⭐⭐        |
| APA102      | APA102         | 5V     | 2 fios (SPI)| 3              | RGB         | ⭐⭐⭐⭐        |
| TM1814      | TM1814         | 12/24V | 1 fio (PWM) | 4              | RGBW        | ⭐⭐⭐         |
| WS2815      | WS2811 derivado| 12V    | 1 fio (PWM) | 3              | RGB         | ⭐⭐⭐⭐        |

---

## 🔍 Comparativo Técnico

| Modelo    | Facilidade | Tolerância à Tensão | Comunicação | Recursos Especiais                        | Ideal para                     |
|-----------|------------|---------------------|-------------|-------------------------------------------|--------------------------------|
| **WS2812B** | ⭐⭐⭐⭐     | Baixa               | 1 fio       | Compatível com Adafruit NeoPixel / FastLED | Projetos DIY com ESP32/Arduino |
| **WS2811**  | ⭐⭐⭐      | Alta                | 1 fio       | LEDs controlados em grupos de 3           | Ambientes com longas distâncias |
| **SK6812**  | ⭐⭐⭐⭐     | Média               | 1 fio       | RGBW (branco adicional)                   | Iluminação com branco real      |
| **APA102**  | ⭐⭐⭐⭐     | Alta                | 2 fios (SPI)| Alta velocidade, resistente a interferências | Efeitos rápidos (POV, etc.)     |
| **TM1814**  | ⭐⭐       | Alta                | 1 fio       | RGBW, tensão 12/24V                       | Instalações profissionais       |
| **WS2815**  | ⭐⭐⭐⭐     | Muito Alta          | 1 fio       | Redundância de dados, 12V, resistente a falhas | Ambientes exigentes             |

---

## ✅ Recomendação por Aplicação

| Aplicação                                      | Modelo recomendado |
|-----------------------------------------------|--------------------|
| Projetos básicos com ESP32/Arduino            | WS2812B            |
| Longas distâncias com menor queda de tensão   | WS2811 ou WS2815   |
| Efeitos rápidos e precisos (ex: POV displays) | APA102             |
| Iluminação com branco real                    | SK6812 ou TM1814   |
| Ambientes sujeitos a falhas ou tensão instável| WS2815             |

---

## 📚 Bibliotecas compatíveis

- `Adafruit_NeoPixel` – WS2812, SK6812, WS2811
- `FastLED` – WS2812, WS2811, APA102, SK6812
- `DotStar` – APA102

---

> ⚠️ **Nota:** Sempre verifique a necessidade de resistores, capacitores e fonte de alimentação adequada conforme o modelo utilizado.
