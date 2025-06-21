#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <FS.h>
#include <Preferences.h>
#include <SPIFFS.h>

#define LED_PIN 5
#define NUM_LEDS 400

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "SEU_SSID";
const char* pass = "SUA_SENHA";

WiFiServer server(80);
Preferences prefs;

int efeitoAtual = 11;
int brilho = 100;
int velocidade = 50;
unsigned long ultimoTempo = 0;
int posCometa = 0;
int posTeatro = 0;
int frameExplosao = 0;
bool estadoPiscar = false;
uint8_t arcoIrisOffset = 0;
bool emergenciaToggle = false;

void setup() {
  Serial.begin(115200);
  strip.begin();
  prefs.begin("ledprefs", false);
brilho = prefs.getInt("brilho", 100);
efeitoAtual = prefs.getInt("efeito", 11);
strip.setBrightness(brilho);
prefs.putInt("brilho", brilho);
  strip.show();

  if (!SPIFFS.begin(true)) {
    Serial.println("Erro ao montar SPIFFS");
    return;
  }

  WiFi.begin(ssid, pass);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado. IP: " + WiFi.localIP().toString());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.readStringUntil('\n');
    Serial.println(req);

    
    if (req.indexOf("get=efeito") >= 0) {
      client.println("HTTP/1.1 200 OK
Content-Type: text/plain

" + String(efeitoAtual));
    } else {

      if (req.indexOf("efeito=") >= 0)
        efeitoAtual = getParam(req, "efeito").toInt();
prefs.putInt("efeito", efeitoAtual);
      if (req.indexOf("brilho=") >= 0) {
        brilho = constrain(getParam(req, "brilho").toInt(), 0, 255);
        prefs.begin("ledprefs", false);
brilho = prefs.getInt("brilho", 100);
efeitoAtual = prefs.getInt("efeito", 11);
strip.setBrightness(brilho);
prefs.putInt("brilho", brilho);
      }
      if (req.indexOf("vel=") >= 0) {
        int val = constrain(getParam(req, "vel").toInt(), 1, 200);
        velocidade = 201 - val;
      }

      client.println("HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n");
    }
    else if (req.startsWith("GET /style.css")) {
      serveFile(client, "/style.css", "text/css");
    }
    else {
      serveFile(client, "/index.html", "text/html");
    }

    client.stop();
  }

  if (millis() - ultimoTempo >= velocidade) {
    ultimoTempo = millis();
    executarEfeito();
  }
}

void serveFile(WiFiClient& client, const char* path, const char* type) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    client.println("HTTP/1.1 404 Not Found\r\n\r\nArquivo não encontrado");
    return;
  }
  client.printf("HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", type);
  while (file.available()) {
    client.write(file.read());
  }
  file.close();
}

String getParam(String req, String key) {
  int start = req.indexOf(key + "=");
  if (start == -1) return "";
  start += key.length() + 1;
  int end = req.indexOf('&', start);
  if (end == -1) end = req.indexOf(' ', start);
  return req.substring(start, end);
}

void executarEfeito() {
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
    case 12: efeitoEmergencia(); break;
    case 13: efeitoTheaterChase(); break;
    case 14: efeitoExplosaoCentral(); break;
  }
}

// ==== Efeitos ====

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

void efeitoEmergencia() {
  emergenciaToggle = !emergenciaToggle;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i % 20 < 10)
      strip.setPixelColor(i, emergenciaToggle ? strip.Color(255, 0, 0) : strip.Color(0, 0, 255));
    else
      strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}

void efeitoTheaterChase() {
  strip.clear();
  for (int i = posTeatro; i < NUM_LEDS; i += 3) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));
  }
  strip.show();
  posTeatro = (posTeatro + 1) % 3;
}

void efeitoExplosaoCentral() {
  strip.clear();
  int centro = NUM_LEDS / 2;
  for (int i = 0; i < NUM_LEDS; i++) {
    int dist = abs(i - centro);
    if (dist == frameExplosao)
      strip.setPixelColor(i, strip.Color(255, 255, 0));
  }
  strip.show();
  frameExplosao++;
  if (frameExplosao > centro) frameExplosao = 0;
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
