/*
  ================================================================================
  Projeto: Controle de Fita de LED WS2812B com ESP32)
  Autor:   Epaminondas de Souza Lage
  Data:    18/06/2025
  Versão:  1.1 (ESP32)

  Descrição:
    Este projeto permite controlar uma fita de LEDs WS2812B (NeoPixel) via interface 
    web usando o ESP32. Através de um navegador conectado à mesma rede, 
    é possível escolher efeitos de iluminação, ajustar brilho e velocidade de animação.

  Funcionalidades:
    - Conexão Wi-Fi com rede local
    - Interface web simples (servidor HTTP embutido)
    - Controle de 11 efeitos visuais via navegador
    - Ajuste de brilho e velocidade dos efeitos
    - Sem uso de delay() (efeitos não bloqueantes)

  ================================================================================
  CONFIGURAÇÕES NECESSÁRIAS
  -------------------------------------------------------------------------------
  1. Conexão da Fita WS2812B:
     - Pino de dados: conecte ao pino digital 5 do ESP32 (ou outro pino adequado)
     - Alimente os LEDs com fonte externa adequada (5V e corrente suficiente)
     - GND da fonte dos LEDs deve estar conectado ao GND do ESP32

  2. Configuração da rede Wi-Fi:
     - Altere as variáveis abaixo com as credenciais da sua rede:
         const char* ssid = "seu ssid";        // Nome da rede Wi-Fi (SSID)
         const char* pass = "sua senha";       // Senha da rede Wi-Fi

  3. Biblioteca necessária:
     - Adafruit NeoPixel: instale pela IDE do Arduino
       (Gerenciador de Bibliotecas > Pesquisar: "Adafruit NeoPixel")

  4. Acesso à interface web:
     - Após carregar o código, abra o Monitor Serial (baud 115200)
     - Anote o endereço IP exibido (ex: 192.168.0.101)
     - Acesse via navegador: http://<ip_do_esp32>

  5. Ajustes opcionais:
     - Número de LEDs: altere a constante NUM_LEDS conforme sua fita
     - Pino de dados: altere LED_PIN caso deseje usar outro pino digital

  ================================================================================
  Licença:
    Este projeto é de uso livre para fins educacionais, acadêmicos e pessoais.
    Modificações são permitidas mediante créditos ao autor original.
  ================================================================================
*/

#include <WiFi.h>                 // Biblioteca para conexão Wi-Fi no ESP32
#include <Adafruit_NeoPixel.h>   // Biblioteca para controle de LEDs WS2812B (NeoPixel)

#define LED_PIN    5             // Pino onde a fita de LEDs está conectada
#define NUM_LEDS   400           // Quantidade total de LEDs na fita

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Credenciais da rede Wi-Fi
const char* ssid = "seu ssid";         // Nome da rede Wi-Fi
const char* pass = "sua senha";        // Senha da rede Wi-Fi

WiFiServer server(80);                 // Cria servidor web na porta 80

// Variáveis de configuração e controle
int efeitoAtual = 0;
int brilho = 100;
int velocidade = 50;
unsigned long ultimoTempo = 0;

// Variáveis internas para efeitos
int frameAtual = 0;
bool estadoPiscar = false;
int posCometa = 0;
uint8_t arcoIrisOffset = 0;

// ========================
// Função de configuração
// ========================
void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(brilho);
  strip.show();

  // Conecta-se à rede Wi-Fi
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

// ========================
// Loop principal
// ========================
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

    // Envia a página HTML
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
      case 5: efeitoCorFixa(255, 255, 255); break;
      case 6: efeitoCorFixa(255, 160, 60); break;
      case 7: efeitoCorFixa(0, 0, 255); break;
      case 8: efeitoCorFixa(0, 255, 0); break;
      case 9: efeitoCorFixa(255, 0, 0); break;
      case 10: efeitoArcoIrisRotativo(); break;
    }
  }
}

// ========================
// Função auxiliar para obter parâmetros GET da URL
// ========================
String getParam(String req, String key) {
  int start = req.indexOf(key + "=");
  if (start == -1) return "";
  start += key.length() + 1;
  int end = req.indexOf('&', start);
  if (end == -1) end = req.indexOf(' ', start);
  return req.substring(start, end);
}

// ========================
// Efeitos visuais
// ========================
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
