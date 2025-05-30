#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// ===== CONFIGURAÇÕES =====
const char* ssid = "SEU_SSID";         // Coloque aqui o nome da sua rede Wi-Fi
const char* password = "SUA_SENHA";    // E a senha do seu Wi-Fi

#define PINO_LEDS 5
#define NUM_LEDS 60

int efeitoAtual = 0;
int velocidade = 50;                   // Velocidade base (delay entre frames)
int brilho = 100;                      // Brilho inicial (0 a 255)

Adafruit_NeoPixel fita(NUM_LEDS, PINO_LEDS, NEO_GRB + NEO_KHZ800);
WebServer server(80);

// Cores padrão
const uint32_t COLOR_RED    = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t COLOR_GREEN  = Adafruit_NeoPixel::Color(0, 255, 0);
const uint32_t COLOR_BLUE   = Adafruit_NeoPixel::Color(0, 0, 255);
const uint32_t COLOR_WHITE  = Adafruit_NeoPixel::Color(255, 255, 255);
const uint32_t COLOR_YELLOW = Adafruit_NeoPixel::Color(255, 150, 0);

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  fita.begin();
  fita.setBrightness(brilho);
  fita.show();

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());

  // Página principal
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", gerarPagina());
  });

  // Mudar efeito
  server.on("/efeito", HTTP_GET, []() {
    if (server.hasArg("id")) {
      efeitoAtual = server.arg("id").toInt();
      server.send(200, "text/plain", "OK");
    }
  });

  // Ajuste de velocidade
  server.on("/velocidade", HTTP_GET, []() {
    if (server.hasArg("acao")) {
      String acao = server.arg("acao");
      if (acao == "mais") velocidade = max(10, velocidade - 10);
      if (acao == "menos") velocidade = min(300, velocidade + 10);
      server.send(200, "text/plain", "Velocidade: " + String(velocidade));
    }
  });

  // Ajuste de brilho
  server.on("/brilho", HTTP_GET, []() {
    if (server.hasArg("acao")) {
      String acao = server.arg("acao");
      if (acao == "mais") brilho = min(255, brilho + 20);
      if (acao == "menos") brilho = max(0, brilho - 20);
      fita.setBrightness(brilho);
      server.send(200, "text/plain", "Brilho: " + String(brilho));
    }
  });

  server.begin();
}

// ===== LOOP =====
void loop() {
  server.handleClient();

  switch (efeitoAtual) {
    case 0: efeitoArcoIris(velocidade); break;
    case 1: efeitoCorrer(COLOR_RED, velocidade); break;
    case 2: efeitoTeclado(COLOR_BLUE, velocidade); break;
    case 3: efeitoPiscadaDupla(COLOR_YELLOW, velocidade); break;
    case 4: efeitoChuva(velocidade); break;
    case 5: efeitoBrancoEstatico(); break;
  }
}

// ===== INTERFACE HTML DINÂMICA =====
String gerarPagina() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Controle WS2812B</title>
  <style>
    body { background: #111; color: white; font-family: sans-serif; text-align: center; padding: 10px; }
    h2 { color: #0f0; }
    button {
      padding: 12px; margin: 8px; border: none;
      border-radius: 8px; background: #333; color: white; font-size: 16px;
      width: 180px; cursor: pointer;
    }
    button:hover { background: #0a0; }
    .grupo { margin-top: 20px; }
  </style>
</head>
<body>
  <h2>Controle de Efeitos WS2812B</h2>

  <div class="grupo">
    <button onclick="efeito(0)">Arco-Íris</button>
    <button onclick="efeito(1)">Correr</button>
    <button onclick="efeito(2)">Teclado</button>
    <button onclick="efeito(3)">Piscada Dupla</button>
    <button onclick="efeito(4)">Chuva</button>
    <button onclick="efeito(5)">Branco Estático</button>
  </div>

  <div class="grupo">
    <h3>Velocidade</h3>
    <button onclick="velocidade('mais')">Mais Rápido</button>
    <button onclick="velocidade('menos')">Mais Lento</button>
  </div>

  <div class="grupo">
    <h3>Brilho</h3>
    <button onclick="brilho('mais')">Aumentar</button>
    <button onclick="brilho('menos')">Diminuir</button>
  </div>

  <script>
    function efeito(id) {
      fetch('/efeito?id=' + id);
    }
    function velocidade(acao) {
      fetch('/velocidade?acao=' + acao).then(r => r.text()).then(t => alert(t));
    }
    function brilho(acao) {
      fetch('/brilho?acao=' + acao).then(r => r.text()).then(t => alert(t));
    }
  </script>
</body>
</html>
)rawliteral";
}

// ===== EFEITOS =====
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
  fita.fill(cor, 0, NUM_LEDS); fita.show(); delay(wait);
  fita.clear(); fita.show(); delay(wait / 2);
  fita.fill(cor, 0, NUM_LEDS); fita.show(); delay(wait);
  fita.clear(); fita.show(); delay(800);
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

void efeitoBrancoEstatico() {
  fita.fill(COLOR_WHITE, 0, NUM_LEDS);
  fita.show();
  delay(100);
  server.handleClient();
}
