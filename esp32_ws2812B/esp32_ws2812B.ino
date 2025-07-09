/*
  ================================================================================
  Projeto: Controle de Fita de LED WS2812B com ESP32
  Autor:   Epaminondas de Souza Lage
  Versão:  1.12

  Descrição:
    Controle de uma fita WS2812B via Wi-Fi com interface HTML. Permite escolher 
    efeitos visuais, ajustar brilho, velocidade, e desligar a fita. Reset de
    variáveis de efeitos adicionado para garantir transição limpa.

  Funcionalidades:
    - Interface responsiva via navegador
    - 15 efeitos visuais
    - Ajuste de brilho e velocidade
    - Fita inicia desligada
    - Mostra aviso se fita estiver ausente
    - Reset de variáveis ao trocar de efeito
    - IP estático

  ================================================================================
*/

#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

// ˜Configuração em qual pin esta a fita e numero de leds que ela contém
#define LED_PIN 5
#define NUM_LEDS 400

/*
Adafruit_NeoPixel
É a classe fornecida pela biblioteca Adafruit que controla LEDs endereçáveis como WS2812B. 
strip:É o nome da instância (objeto). Você usa ele depois para controlar os LEDs, ex.: strip.begin(), strip.setPixelColor(), strip.show(), etc.
NUM_LEDS:Número de LEDs conectados. Exemplo: 400.
LED_PIN:Pino do ESP32 que está conectado à entrada de dados da fita WS2812B.
NEO_GRB:Define a ordem dos canais de cor (Green, Red, Blue). A maioria das WS2812B usa GRB, mas alguns modelos usam RGB ou BRG.
NEO_KHZ800:Define a frequência do protocolo de comunicação, que no caso da WS2812B é 800kHz.
*/
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Configurações Wi-Fi
const char* ssid = "PLT-DIR";
const char* pass = "epaminondas";

WiFiServer server(80);

// Variáveis de controle
int efeitoAtual = 16; // Ajuste aqui para o efeito inicial da fita
int brilho = 100;
int velocidade = 50;
unsigned long ultimoTempo = 0;
int frameAtual = 0;
bool estadoPiscar = false;
int posCometa = 0;
uint8_t arcoIrisOffset = 0;
int ledAtual = 0;
int setor = 0;
float fase = 0.0;
int passo = 0;

/* Setup para dhcp
void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(brilho);
  strip.show();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
}
*/


// Setup para ip estático
void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(brilho);
  strip.show();

  IPAddress local_IP(10, 0, 2, 242);
  IPAddress gateway(10, 0, 2, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS(8, 8, 4, 4);

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("❌ Falha ao configurar IP estático");
  }

  WiFi.begin(ssid, pass);
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
      if (req.indexOf("efeito=") >= 0) {
        efeitoAtual = getParam(req, "efeito").toInt();
        resetarEfeitos();
      }
      if (req.indexOf("brilho=") >= 0) {
        brilho = constrain(getParam(req, "brilho").toInt(), 0, 255);
        strip.setBrightness(brilho);
      }
      if (req.indexOf("vel=") >= 0) {
        int val = constrain(getParam(req, "vel").toInt(), 1, 200);
        velocidade = 201 - val; // Inversão
      }
    }

    // Resposta HTML
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html\nConnection: close\n");
      client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
      client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
      client.println("<title>Controle de LEDs</title>");
      client.println("<style>");
      client.println("body{background:#fff;color:#000;font-family:sans-serif;text-align:center;margin:10px;padding:0;}");
      client.println("h2{font-size:1.5em;margin-bottom:10px;}");
      client.println("form{margin:0;}");
      client.println("button{min-width:120px;padding:14px 10px;font-size:16px;background:#fff;color:#000;border:2px solid #ccc;border-radius:8px;margin:6px;}");
      client.println("button.selected{font-weight:bold;border:2px solid #000;}");
      client.println("input[type=range]{width:90%;margin:14px auto;display:block;}");
      client.println("@media (max-width:600px){button{width:100%;max-width:300px;}}");
      client.println("</style></head><body>");
      client.println("<h2>Fita de LED: Passarela</h2>");


    // Botões de efeito
    client.println("<form action='/config' method='get'>");
    client.println("<div style='display:flex;flex-wrap:wrap;justify-content:center;gap:8px;margin:10px 0;'>");

    String nomes[] = {
      "Confete", "Cometa", "Piscar", "Arco-Íris",
      "Branco Frio", "Branco Quente", "Azul", "Verde",
      "Vermelho", "Arco-Íris Rotativo", "Progressivo",
      "Ligar Sequencial", "Chuva de Estrelas","Polícia","Explosão Central"
    };

    // este loop deve ser ajustado para o número de efeitos
    for (int i = 0; i < 15; i++) {
      String classe = (efeitoAtual == i ? "selected" : "");
      client.println("<button type='submit' name='efeito' value='" + String(i) + "' class='" + classe + "'>" + nomes[i] + "</button>");
    }
    client.println("</div></form>");

    client.println("<label>Brilho:</label>");
    client.println("<input type='range' min='0' max='255' value='" + String(brilho) + "' onchange=\"location.href='/config?brilho='+this.value\">");

    client.println("<label>Velocidade (esquerda = lenta, direita = rápida):</label>");
    client.println("<input type='range' min='1' max='200' value='" + String(201 - velocidade) + "' onchange=\"location.href='/config?vel='+this.value\">");

    client.println("<form action='/config' method='get' style='margin-top:20px;'>");

    // A variável "value" deve ser modificada todas as vezes que mudar a quantidade de efeitos. Ela aponta para o efeito "desliga"
    client.println("<input type='hidden' name='efeito' value='15'>");
    client.println("<button type='submit' style='background:#000;color:#fff;'>Desligar Fita</button>");
    client.println("</form>");

    if (!strip.canShow()) {
      client.println("<p style='color:red'>⚠ Fita de LEDs não detectada ou desconectada.</p>");
    }

    client.println("</body></html>");
    client.stop();
  }
// Menu de Efeitos
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
      case 11: efeitoAcenderSequencial(); break;
      case 12: efeitoChuvaDeEstrelas(); break;
      case 13: efeitoPolicia(); break;
      case 14: efeitoExplosaoCentral(); break;  
      case 15: efeitoCorFixa(0, 0, 0); break;
      
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

// === Efeitos Implantados ===

// === Resetar Efeitos na Fita ===

void resetarEfeitos() {
  frameAtual = 0;
  estadoPiscar = false;
  posCometa = 0;
  arcoIrisOffset = 0;
  ledAtual = 0;
  setor = 0;
  fase = 0.0;
  passo = 0;  
  strip.clear();
  strip.show();
}

// === Efeito Confete ===
void efeitoConfete() {
  int p = random(NUM_LEDS);
  strip.setPixelColor(p, strip.Color(random(255), random(255), random(255)));
  strip.show();
}

// === Efeito Cometa ===
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

// === Efeito Piscar leds ===
void efeitoPiscar() {
  if (estadoPiscar) strip.fill(strip.Color(255, 255, 255));
  else strip.clear();
  strip.show();
  estadoPiscar = !estadoPiscar;
}
// === Efeito Arco Iris ===
void efeitoArcoIris() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, wheel((i + arcoIrisOffset) & 255));
  }
  strip.show();
  arcoIrisOffset++;
}

// === Efeito Arco Iris Rotativo ===
void efeitoArcoIrisRotativo() {
  static uint16_t offset = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t pos = (i * 256 / NUM_LEDS + offset) & 255;
    strip.setPixelColor(i, wheel(pos));
  }
  strip.show();
  offset++;
}

//Gera cores RGB suaves em sequência, formando um espectro de arco-íris contínuo
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

// === Efeito Cor Fixa ===
void efeitoCorFixa(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

// === Efeito Cor progressiva por setor ===
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

// === Efeito Ligar em Sequencia ===
void efeitoAcenderSequencial() {
  if (ledAtual < NUM_LEDS) {
    strip.setPixelColor(ledAtual, strip.Color(255, 160, 60)); // cor âmbar
    strip.show();
    ledAtual++;
  }
}


// === Efeito Estrelas que Piscam ===
void efeitoChuvaDeEstrelas() {
  for (int i = 0; i < NUM_LEDS; i++) {
    uint32_t cor = strip.getPixelColor(i);
    uint8_t r = (cor >> 16) & 0xFF;
    uint8_t g = (cor >> 8) & 0xFF;
    uint8_t b = cor & 0xFF;
    r = r * 4 / 5;
    g = g * 4 / 5;
    b = b * 4 / 5;
    strip.setPixelColor(i, r, g, b);
  }
  for (int i = 0; i < 10; i++) {
    int p = random(NUM_LEDS);
    strip.setPixelColor(p, strip.Color(255, 200, 100));
  }
  strip.show();
}

// === Efeito Giroscópio Polícia ===
void efeitoPolicia() {
  static bool alterna = false;
  int metade = NUM_LEDS / 2;

  // Define as cores conforme o estado atual
  for (int i = 0; i < NUM_LEDS; i++) {
    if (alterna) {
      strip.setPixelColor(i, i < metade ? strip.Color(255, 0, 0) : strip.Color(0, 0, 255)); // vermelho / azul
    } else {
      strip.setPixelColor(i, i < metade ? strip.Color(0, 0, 255) : strip.Color(255, 0, 0)); // azul / vermelho
    }
  }

  strip.show();
  alterna = !alterna; // Inverte para o próximo ciclo
}

// === Efeito Explosao Centro da Fita ===
void efeitoExplosaoCentral() {
  static int passo = 0;
  int centro = NUM_LEDS / 2;

  strip.clear();

  for (int i = 0; i <= passo; i++) {
    int esquerda = centro - i;
    int direita = centro + i;

    if (esquerda >= 0) strip.setPixelColor(esquerda, strip.Color(255, 200, 50)); // cor de explosão
    if (direita < NUM_LEDS) strip.setPixelColor(direita, strip.Color(255, 200, 50));
  }

  strip.show();
  passo++;

  if (centro + passo >= NUM_LEDS && centro - passo < 0) passo = 0; // reinicia o efeito
}