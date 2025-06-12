#include <WiFiS3.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN    6
#define NUM_LEDS   400

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

char ssid[] = "PLT-DIR";
char pass[] = "epaminondas";

int status = WL_IDLE_STATUS;
WiFiServer server(80);

int efeitoAtual = 0;
int brilho = 100;
int velocidade = 50;

unsigned long ultimoTempo = 0;

// Controle de estado para efeitos não bloqueantes
int frameAtual = 0;
bool estadoPiscar = false;
int posCometa = 0;
uint8_t arcoIrisOffset = 0;

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(brilho);
  strip.show();

  while (status != WL_CONNECTED) {
    Serial.print("Conectando-se a ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(2000);
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
    Serial.println("Requisição recebida:");
    Serial.println(req);

    if (req.indexOf("GET /config") >= 0) {
      if (req.indexOf("efeito=") >= 0)
        efeitoAtual = getParam(req, "efeito").toInt();
      if (req.indexOf("brilho=") >= 0) {
        brilho = constrain(getParam(req, "brilho").toInt(), 0, 255);
        strip.setBrightness(brilho);
      }
      if (req.indexOf("vel=") >= 0)
        velocidade = constrain(getParam(req, "vel").toInt(), 1, 200);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>LEDs</title></head>");
    client.println("<body style='background:#111;color:#fff;font-family:sans-serif;text-align:center'>");
    client.println("<h2>LEDs WS2812B</h2>");
    client.println("<form action='/config' method='get'>");
    client.println("<label>Efeito:<select name='efeito'>");
    client.println("<option value='0'" + String(efeitoAtual == 0 ? " selected" : "") + ">Aleatório</option>");
    client.println("<option value='1'" + String(efeitoAtual == 1 ? " selected" : "") + ">Cometa</option>");
    client.println("<option value='2'" + String(efeitoAtual == 2 ? " selected" : "") + ">Piscar</option>");
    client.println("<option value='3'" + String(efeitoAtual == 3 ? " selected" : "") + ">Arco-Íris</option>");
    client.println("<option value='4'" + String(efeitoAtual == 4 ? " selected" : "") + ">Apagar</option>");
    client.println("<option value='5'" + String(efeitoAtual == 5 ? " selected" : "") + ">Branco Frio</option>");
    client.println("<option value='6'" + String(efeitoAtual == 6 ? " selected" : "") + ">Branco Quente</option>");
    client.println("<option value='7'" + String(efeitoAtual == 7 ? " selected" : "") + ">Azul</option>");
    client.println("<option value='8'" + String(efeitoAtual == 8 ? " selected" : "") + ">Verde</option>");
    client.println("<option value='9'" + String(efeitoAtual == 9 ? " selected" : "") + ">Vermelho</option>");
    client.println("<option value='10'" + String(efeitoAtual == 10 ? " selected" : "") + ">Arco-Íris Rotativo</option>");
    client.println("</select></label><br>");
    client.println("<label>Brilho: <input type='range' min='0' max='255' name='brilho' value='" + String(brilho) + "'></label><br>");
    client.println("<label>Velocidade: <input type='range' min='1' max='200' name='vel' value='" + String(velocidade) + "'></label><br>");
    client.println("<input type='submit' value='Aplicar'></form></body></html>");
    client.stop();
  }

  if (millis() - ultimoTempo >= velocidade) {
    ultimoTempo = millis();
    switch (efeitoAtual) {
      case 0: efeitoCoresAleatorias(); break;
      case 1: efeitoCometa(); break;
      case 2: efeitoPiscar(); break;
      case 3: efeitoArcoIris(); break;
      case 4: efeitoApagar(); break;
      case 5: efeitoCorFixa(255, 255, 255); break; // Branco frio
      case 6: efeitoCorFixa(255, 160, 60); break;  // Branco quente
      case 7: efeitoCorFixa(0, 0, 255); break;     // Azul
      case 8: efeitoCorFixa(0, 255, 0); break;     // Verde
      case 9: efeitoCorFixa(255, 0, 0); break;     // Vermelho
      case 10: efeitoArcoIrisRotativo(); break;   // Arco-Íris rotativo
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

void efeitoArcoIrisRotativo() {
  static uint16_t offset = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t pos = (i * 256 / NUM_LEDS + offset) & 255;
    strip.setPixelColor(i, wheel(pos));
  }
  strip.show();
  offset++;
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
