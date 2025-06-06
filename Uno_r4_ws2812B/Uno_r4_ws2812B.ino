// Projeto: Arduino UNO R4 WiFi + Fita WS2812B + Matriz de LED + Página AJAX

#include <WiFiS3.h>
#include <Adafruit_NeoPixel.h>
#include "Arduino_LED_Matrix.h"

#define LED_PIN     6
#define NUM_LEDS    30
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
ArduinoLEDMatrix matrix;

char ssid[] = "PLT-DIR";
char pass[] = "epaminondas";
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


const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Controle de LEDs WS2812B</title>
  <style>
    body { font-family: sans-serif; text-align: center; margin-top: 40px; }
    button { padding: 10px; margin: 5px; font-size: 16px; }
    input[type=range] { width: 200px; }
  </style>
</head>
<body>
  <h1>Controle de Efeitos</h1>
  <div>
    <button onclick="enviar('efeito=0')">Arco-Íris</button>
    <button onclick="enviar('efeito=1')">Knight Rider</button>
    <button onclick="enviar('efeito=2')">Teclado</button><br>
    <button onclick="enviar('efeito=3')">Piscada Dupla</button>
    <button onclick="enviar('efeito=4')">Chuva</button>
    <button onclick="enviar('efeito=5')">Branco</button>
  </div>
  <h2>Brilho</h2>
  <input type="range" min="0" max="255" value="100" oninput="enviar('brilho=' + this.value)">
  <h2>Velocidade</h2>
  <input type="range" min="0" max="200" value="50" oninput="enviar('velocidade=' + this.value)">
  <script>
    function enviar(param) {
      fetch('/?' + param);
    }
  </script>
</body>
</html>
)rawliteral";

// Prototipação das funções de efeito
void exibirNomeEfeito();
void executarEfeito();
uint32_t Wheel(byte pos);

void arcoIris();
void knightRider();
void teclado();
void piscadaDupla();
void chuva();
void branco();



void setup() {
  Serial.begin(9600);
  matrix.begin();
  strip.begin();
  strip.setBrightness(brilho);
  strip.show();

  
  Serial.begin(9600);
  while (!Serial);  // Aguarda a Serial estar disponível
  Serial.println("Iniciando conexão Wi-Fi...");
  WiFi.begin(ssid, pass);
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Falha ao conectar no Wi-Fi.");
  }
  server.begin();
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
  // matrix.scrollText(nomesEfeitos[efeitoAtual], 100);
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

