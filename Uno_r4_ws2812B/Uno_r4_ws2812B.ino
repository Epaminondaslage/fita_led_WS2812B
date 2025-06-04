// Projeto: Arduino UNO R4 WiFi + Fita WS2812B + Matriz de LED + Página AJAX

#include <WiFiS3.h>
#include <Adafruit_NeoPixel.h>
#include "Arduino_LED_Matrix.h"

#define LED_PIN     6
#define NUM_LEDS    30
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
ArduinoLEDMatrix matrix;

char ssid[] = "SEU_SSID";
char pass[] = "SUA_SENHA";
WiFiServer server(80);

int efeitoAtual = 0;
int brilho = 100;       // 0 a 255
int velocidade = 50;    // delay base

const char* nomesEfeitos[] = {
  "Arco-Iris",
  "Knight Rider",
  "Teclado",
  "Piscada Dupla",
  "Chuva",
  "Branco"
};
const int totalEfeitos = sizeof(nomesEfeitos) / sizeof(nomesEfeitos[0]);

void setup() {
  Serial.begin(115200);
  matrix.begin();
  strip.begin();
  strip.setBrightness(brilho);
  strip.show();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  server.begin();
  Serial.println(WiFi.localIP());
  exibirNomeEfeito();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\n');
    req.trim();
    if (req.startsWith("GET /")) {
      if (req.indexOf("/efeito=") > 0) efeitoAtual = req.substring(req.indexOf('=') + 1).toInt();
      else if (req.indexOf("/brilho=") > 0) brilho = constrain(req.substring(req.indexOf('=') + 1).toInt(), 0, 255);
      else if (req.indexOf("/velocidade=") > 0) velocidade = constrain(req.substring(req.indexOf('=') + 1).toInt(), 0, 200);

      strip.setBrightness(brilho);
      exibirNomeEfeito();

      // Envia resposta JSON
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println();
      client.print("{\"efeito\":\"");
      client.print(nomesEfeitos[efeitoAtual]);
      client.print("\",\"brilho\":");
      client.print(brilho);
      client.print(",\"velocidade\":");
      client.print(velocidade);
      client.println("}");
    } else {
      // Envia página HTML
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close\n");
      client.println(htmlPage); // conteúdo HTML injetado abaixo
    }
    client.stop();
  }
  executarEfeito();
}

void exibirNomeEfeito() {
  matrix.clear();
  matrix.scrollText(nomesEfeitos[efeitoAtual], 100);
}

void executarEfeito() {
  switch (efeitoAtual) {
    case 0: arcoIris(); break;
    case 1: knightRider(); break;
    case 2: teclado(); break;
    case 3: piscadaDupla(); break;
    case 4: chuva(); break;
    case 5: branco(); break;
  }
}

// ---------- EFEITOS ----------

void arcoIris() {
  static uint8_t j = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, Wheel((i + j) & 255));
  }
  strip.show();
  j++;
  delay(velocidade);
}

void knightRider() {
  static int pos = 0, dir = 1;
  strip.clear();
  strip.setPixelColor(pos, strip.Color(255, 0, 0));
  strip.show();
  pos += dir;
  if (pos <= 0 || pos >= NUM_LEDS - 1) dir = -dir;
  delay(velocidade);
}

void teclado() {
  static int i = 0, dir = 1;
  strip.setPixelColor(i, strip.Color(0, 255, 0));
  if (dir == -1) strip.setPixelColor(i, 0);
  strip.show();
  i += dir;
  if (i >= NUM_LEDS) { i = NUM_LEDS - 1; dir = -1; }
  if (i < 0) { i = 0; dir = 1; }
  delay(velocidade);
}

void piscadaDupla() {
  static bool on = false;
  strip.fill(on ? strip.Color(0, 0, 255) : 0);
  strip.show();
  on = !on;
  delay(velocidade * 2);
}

void chuva() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (random(10) > 7)
      strip.setPixelColor(i, strip.Color(0, 0, random(100, 255)));
    else
      strip.setPixelColor(i, 0);
  }
  strip.show();
  delay(velocidade);
}

void branco() {
  strip.fill(strip.Color(255, 255, 255));
  strip.show();
  delay(velocidade);
}

uint32_t Wheel(byte pos) {
  pos = 255 - pos;
  if (pos < 85) return strip.Color(255 - pos * 3, 0, pos * 3);
  if (pos < 170) {
    pos -= 85;
    return strip.Color(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return strip.Color(pos * 3, 255 - pos * 3, 0);
}

// ---------- HTML embutido ----------

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang='pt-br'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>LED FX Control</title>
  <style>
    body { font-family: sans-serif; text-align: center; background: #111; color: #eee; }
    button { margin: 0.5em; padding: 1em; font-size: 1em; border-radius: 8px; border: none; background: #00bcd4; color: white; cursor: pointer; }
    button:hover { background: #008c9e; }
  </style>
</head>
<body>
  <h1>Controle de Efeitos LED</h1>
  <div>
    <button onclick="enviar('efeito=0')">Arco-Íris</button>
    <button onclick="enviar('efeito=1')">Knight Rider</button>
    <button onclick="enviar('efeito=2')">Teclado</button>
    <button onclick="enviar('efeito=3')">Piscada Dupla</button>
    <button onclick="enviar('efeito=4')">Chuva</button>
    <button onclick="enviar('efeito=5')">Branco</button><br>
    <button onclick="enviar('velocidade=' + (velocidade - 10))">- Velocidade</button>
    <button onclick="enviar('velocidade=' + (velocidade + 10))">+ Velocidade</button><br>
    <button onclick="enviar('brilho=' + (brilho - 20))">- Brilho</button>
    <button onclick="enviar('brilho=' + (brilho + 20))">+ Brilho</button>
  </div>
  <p id='status'></p>
  <script>
    let brilho = 100;
    let velocidade = 50;
    function enviar(cmd) {
      fetch('/?' + cmd).then(r => r.json()).then(data => {
        brilho = data.brilho;
        velocidade = data.velocidade;
        document.getElementById('status').innerText = `Efeito: ${data.efeito} | Brilho: ${brilho} | Velocidade: ${velocidade}`;
      });
    }
  </script>
</body>
</html>
)rawliteral";
