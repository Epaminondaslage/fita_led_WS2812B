#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// ======= CONFIGURAÇÕES =======
#define PINO_LEDS     5
#define NUM_LEDS     60
#define BRILHO       100

const char* ssid     = "SEU_SSID";       // Substitua pelo seu Wi-Fi
const char* password = "SUA_SENHA";

WebServer server(80);
Adafruit_NeoPixel fita(NUM_LEDS, PINO_LEDS, NEO_GRB + NEO_KHZ800);

int efeitoAtual = 0;  // Índice do efeito ativo

// ======= CORES PADRÃO =======
const uint32_t COLOR_RED    = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t COLOR_GREEN  = Adafruit_NeoPixel::Color(0, 255, 0);
const uint32_t COLOR_BLUE   = Adafruit_NeoPixel::Color(0, 0, 255);
const uint32_t COLOR_WHITE  = Adafruit_NeoPixel::Color(255, 255, 255);
const uint32_t COLOR_YELLOW = Adafruit_NeoPixel::Color(255, 150, 0);

// ======= SETUP =======
void setup() {
  Serial.begin(115200);
  fita.begin();
  fita.setBrightness(BRILHO);
  fita.show();

  // Conexão Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());

  // Página HTML
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>LED WS2812B</title></head><body>";
    html += "<h2>Controle de Efeitos LED</h2>";
    html += "<button onclick=\"location.href='/mudar'\">Mudar Efeito</button>";
    html += "<p>Efeito atual: " + String(efeitoAtual + 1) + "</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/mudar", []() {
    efeitoAtual = (efeitoAtual + 1) % 6;  // Agora temos 6 efeitos
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.begin();
}

// ======= LOOP =======
void loop() {
  server.handleClient();
  switch (efeitoAtual) {
    case 0: efeitoArcoIris(5); break;
    case 1: efeitoCorrer(COLOR_RED, 30); break;
    case 2: efeitoTeclado(COLOR_BLUE, 40); break;
    case 3: efeitoPiscadaDupla(COLOR_YELLOW, 100); break;
    case 4: efeitoChuva(50); break;
    case 5: efeitoBrancoEstatico(); break;
  }
}

// ======= EFEITOS =======
void efeitoArcoIris(uint8_t wait) {
  for (long hue = 0; hue < 65536; hue += 512) {
    for (int i = 0; i < NUM_LEDS; i++) {
      int pixelHue = hue + (i * 65536L / NUM_LEDS);
      fita.setPixelColor(i, fita.gamma32(fita.ColorHSV(pixelHue)));
    }
    fita.show();
    delay(wait);
    server.handleClient();
  }
}

void efeitoCorrer(uint32_t cor, int wait) {
  for (int i = 0; i < NUM_LEDS; i++) {
    fita.clear();
    fita.setPixelColor(i, cor);
    fita.show();
    delay(wait);
    server.handleClient();
  }
}

void efeitoTeclado(uint32_t cor, int wait) {
  for (int i = 0; i < NUM_LEDS; i++) {
    fita.setPixelColor(i, cor);
    fita.show();
    delay(wait);
    server.handleClient();
  }
  delay(wait);
  for (int i = 0; i < NUM_LEDS; i++) {
    fita.setPixelColor(i, 0);
    fita.show();
    delay(wait / 2);
    server.handleClient();
  }
}

void efeitoPiscadaDupla(uint32_t cor, int wait) {
  fita.fill(cor, 0, NUM_LEDS);
  fita.show();
  delay(wait);
  fita.clear();
  fita.show();
  delay(wait / 2);
  fita.fill(cor, 0, NUM_LEDS);
  fita.show();
  delay(wait);
  fita.clear();
  fita.show();
  delay(800);
}

void efeitoChuva(int wait) {
  fita.clear();
  for (int i = 0; i < 10; i++) {
    int pixel = random(NUM_LEDS);
    fita.setPixelColor(pixel, COLOR_WHITE);
  }
  fita.show();
  delay(wait);
}

// ======= EFEITO BRANCO ESTÁTICO =======
void efeitoBrancoEstatico() {
  fita.fill(COLOR_WHITE, 0, NUM_LEDS);
  fita.show();
  delay(100);
  server.handleClient();
}
