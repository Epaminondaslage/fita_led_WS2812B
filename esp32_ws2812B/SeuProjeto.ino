#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

#define LED_PIN    5
#define NUM_LEDS   100

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "seu_ssid";
const char* password = "sua_senha";

WebServer server(80);
WebSocketsServer webSocket(81);

int efeitoAtual = 0;
int brilho = 100;
int velocidade = 50;

unsigned long ultimoTempo = 0;
int frameAtual = 0;
bool estadoPiscar = false;
int posCometa = 0;
uint8_t arcoIrisOffset = 0;

void handleWebSocketMessage(uint8_t num, uint8_t *payload, size_t length) {
  String msg = (char*)payload;
  if (msg.startsWith("efeito:")) efeitoAtual = msg.substring(7).toInt();
  if (msg.startsWith("brilho:")) {
    brilho = constrain(msg.substring(7).toInt(), 0, 255);
    strip.setBrightness(brilho);
  }
  if (msg.startsWith("vel:")) velocidade = constrain(msg.substring(4).toInt(), 1, 200);
}

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    handleWebSocketMessage(num, payload, length);
  }
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(brilho);
  strip.show();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());

  if (!SPIFFS.begin(true)) {
    Serial.println("Erro ao montar SPIFFS");
    return;
  }

  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();

  if (millis() - ultimoTempo >= velocidade) {
    ultimoTempo = millis();
    switch (efeitoAtual) {
      case 0: efeitoCoresAleatorias(); break;
      case 1: efeitoCometa(); break;
      case 2: efeitoPiscar(); break;
      case 3: efeitoArcoIris(); break;
      case 4: efeitoApagar(); break;
      case 5: efeitoCorFixa(255, 255, 255); break;
      case 6: efeitoCorFixa(255, 160, 60); break;
      case 7: efeitoCorFixa(0, 0, 255); break;
      case 8: efeitoCorFixa(0, 255, 0); break;
      case 9: efeitoCorFixa(255, 0, 0); break;
      case 10: efeitoArcoIrisRotativo(); break;
    }
  }
}

// ======= Efeitos =======

void efeitoCoresAleatorias() {
  strip.setPixelColor(frameAtual, strip.Color(random(255), random(255), random(255)));
  frameAtual++;
  if (frameAtual >= NUM_LEDS) {
    strip.show();
    frameAtual = 0;
  }
}

void efeitoCometa() {
  int cauda = 10;
  strip.clear();
  for (int j = 0; j < cauda; j++) {
    int p = posCometa - j;
    if (p >= 0 && p < NUM_LEDS) {
      float b = 1.0 - float(j) / cauda;
      strip.setPixelColor(p, strip.Color(255 * b, 100 * b, 0));
    }
  }
  strip.show();
  posCometa++;
  if (posCometa > NUM_LEDS + cauda) posCometa = 0;
}

void efeitoPiscar() {
  if (estadoPiscar) strip.fill(strip.Color(255, 255, 255));
  else strip.clear();
  strip.show();
  estadoPiscar = !estadoPiscar;
}

void efeitoArcoIris() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, wheel((i + arcoIrisOffset) & 255));
  }
  strip.show();
  arcoIrisOffset++;
}

void efeitoApagar() {
  strip.clear();
  strip.show();
}

void efeitoCorFixa(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void efeitoArcoIrisRotativo() {
  static uint16_t offset = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t pos = (i * 256 / NUM_LEDS + offset) & 255;
    strip.setPixelColor(i, wheel(pos));
  }
  strip.show();
  offset++;
}

uint32_t wheel(byte pos) {
  if (pos < 85) return strip.Color(pos * 3, 255 - pos * 3, 0);
  else if (pos < 170) {
    pos -= 85;
    return strip.Color(255 - pos * 3, 0, pos * 3);
  } else {
    pos -= 170;
    return strip.Color(0, pos * 3, 255 - pos * 3);
  }
}
