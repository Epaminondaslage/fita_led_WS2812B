/*
  ================================================================================
  Projeto: Controle de Fita de LED WS2812B com ESP32
  Autor:   Epaminondas de Souza Lage
  Versão:  1.7 - Atualizado com efeito "Progressivo por Setores"

  Descrição:
    Controle de uma fita WS2812B via Wi-Fi com interface HTML. Permite escolher 
    efeitos visuais, ajustar brilho, velocidade, e desligar a fita.

  Funcionalidades:
    - Interface responsiva via navegador
    - 11 efeitos visuais
    - Ajuste de brilho e velocidade
    - Fita inicia desligada
    - Mostra aviso se fita estiver ausente
  ================================================================================
*/

#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 5
#define NUM_LEDS 400

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "SEU SSID";
const char* pass = "SUA SENHA";

WiFiServer server(80);

int efeitoAtual = 11; // Inicia desligado
int brilho = 100;
int velocidade = 50;
unsigned long ultimoTempo = 0;

int frameAtual = 0;
bool estadoPiscar = false;
int posCometa = 0;
uint8_t arcoIrisOffset = 0;

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(brilho);
  strip.show();

  WiFi.begin(ssid, pass);
  Serial.print("Conectando-se à rede Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    client.setTimeout(500);
    String req = "";
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;
      req += line;
    }
    client.flush();

    if (req.indexOf("GET /config") >= 0) {
      if (req.indexOf("efeito=") >= 0)
        efeitoAtual = getParam(req, "efeito").toInt();
      if (req.indexOf("brilho=") >= 0) {
        brilho = constrain(getParam(req, "brilho").toInt(), 0, 255);
        strip.setBrightness(brilho);
      }
      if (req.indexOf("vel=") >= 0) {
        int val = constrain(getParam(req, "vel").toInt(), 1, 200);
        velocidade = 201 - val; // Inversão: direita = mais rápido
      }
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>LEDs</title>");
    client.println("<style>");
    client.println("body{background:#fff;color:#000;font-family:sans-serif;text-align:center;margin:20px;}");
    client.println("button{padding:10px 20px;background:#fff;color:#000;border:1px solid #ccc;cursor:pointer;margin:4px;}");
    client.println("button.selected{font-weight:bold;border:2px solid #000;}");
    client.println("input[type=range]{width:80%;margin:10px auto;display:block;}");
    client.println("</style></head><body>");
    client.println("<h2>Controle de LEDs WS2812B</h2>");

    client.println("<form action='/config' method='get'>");
    client.println("<div style='display:flex;flex-wrap:wrap;justify-content:center;gap:8px;margin:10px 0;'>");

    String nomes[] = {
      "Confete", "Cometa", "Piscar", "Arco-Íris",
      "Branco Frio", "Branco Quente", "Azul", "Verde",
      "Vermelho", "Arco-Íris Rotativo", "Progressivo"
    };
    for (int i = 0; i < 11; i++) {
      String classe = (efeitoAtual == i ? "selected" : "");
      client.println("<button type='submit' name='efeito' value='" + String(i) + "' class='" + classe + "'>" + nomes[i] + "</button>");
    }
    client.println("</div></form>");

    client.println("<label>Brilho:</label>");
    client.println("<input type='range' min='0' max='255' value='" + String(brilho) + "' onchange=\"location.href='/config?brilho='+this.value\">");

    client.println("<label>Velocidade (esquerda = lenta, direita = rápida):</label>");
    client.println("<input type='range' min='1' max='200' value='" + String(201 - velocidade) + "' onchange=\"location.href='/config?vel='+this.value\">");

    client.println("<form action='/config' method='get' style='margin-top:20px;'>");
    client.println("<input type='hidden' name='efeito' value='11'>");
    client.println("<button type='submit' style='background:#000;color:#fff;'>Desligar Fita</button>");
    client.println("</form>");

    if (!strip.canShow()) {
      client.println("<p style='color:red'>⚠ Fita de LEDs não detectada ou desconectada.</p>");
    }

    client.println("</body></html>");
    client.stop();
  }

  if (millis() - ultimoTempo >= velocidade) {
    ultimoTempo = millis();
    switch (efeitoAtual) {
      case 0: efeitoConfete(); break;
      case 1: efeitoCometa(); break;
      case 2: efeitoPiscar(); break;
      case 3: efeitoArcoIris(); break;
      case 4: efeitoCorFixa(255, 255, 255); break;
      case 5: efeitoCorFixa(255, 160, 60); break;
      case 6: efeitoCorFixa(0, 0, 255); break;
      case 7: efeitoCorFixa(0, 255, 0); break;
      case 8: efeitoCorFixa(255, 0, 0); break;
      case 9: efeitoArcoIrisRotativo(); break;
      case 10: efeitoProgressivoPorSetores(); break;
      case 11: efeitoCorFixa(0, 0, 0); break;
    }
  }
}

String getParam(String req, String key) {
  int start = req.indexOf(key + "=");
  if (start == -1) return "";
  start += key.length() + 1;
  int end = req.indexOf('&', start);
  if (end == -1) end = req.indexOf(' ', start);
  return req.substring(start, end);
}

// === Efeitos ===

void efeitoConfete() {
  int p = random(NUM_LEDS);
  strip.setPixelColor(p, strip.Color(random(255), random(255), random(255)));
  strip.show();
}

void efeitoCometa() {
  int cauda = 15;
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

void efeitoArcoIrisRotativo() {
  static uint16_t offset = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t pos = (i * 256 / NUM_LEDS + offset) & 255;
    strip.setPixelColor(i, wheel(pos));
  }
  strip.show();
  offset++;
}

void efeitoCorFixa(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void efeitoProgressivoPorSetores() {
  static int setor = 0;
  int totalSetores = 10;
  int ledsPorSetor = NUM_LEDS / totalSetores;

  strip.clear();
  for (int i = 0; i <= setor && i < totalSetores; i++) {
    for (int j = 0; j < ledsPorSetor; j++) {
      int idx = i * ledsPorSetor + j;
      if (idx < NUM_LEDS)
        strip.setPixelColor(idx, strip.Color(0, 150, 255));
    }
  }
  strip.show();

  setor++;
  if (setor >= totalSetores) setor = 0;
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
