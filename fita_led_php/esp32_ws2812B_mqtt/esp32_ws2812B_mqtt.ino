/*
  ================================================================================
  Projeto : Controle de Fita de LED WS2812B com ESP32 + MQTT
  Autor   : Epaminondas de Souza Lage
  Versão  : 2.3.1

  Descrição:
    Controle de fita WS2812B via MQTT com servidor web de diagnóstico.
    O mesmo firmware roda em todos os ESP32.

  Correção v2.3.1:
    - Inicialização estática da NeoPixel (sem new/delete)
    - updateLength() e setPin() para reconfigurar sem alocar memoria
    - Mais estável em todos os modelos de ESP32

  Novidades v2.3.0:
    - Dual WiFi: tenta rede primária, se falhar usa rede secundária
    - Servidor HTTP na porta 80: página de diagnóstico em tempo real
    - Serial em 19200 baud
    - Padrão de 480 LEDs
    - Log de mensagens MQTT recebidas e enviadas

  Arquitetura MQTT:
    fita-led/discovery/{MAC}    ← ESP32 publica ao conectar (retain=true)
    fita-led/{MAC}/cmd          ← ESP32 assina para receber comandos
    fita-led/{MAC}/status       ← ESP32 publica estado atual
    fita-led/{MAC}/nome         ← Frontend publica nome personalizado (retain=true)
    fita-led/{MAC}/config       ← Frontend publica config de hardware (retain=true)

  Bibliotecas necessárias (Arduino Library Manager):
    - Adafruit NeoPixel
    - PubSubClient (Nick O'Leary)
    - ArduinoJson

  ================================================================================
*/

#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// ── Valores padrão ─────────────────────────────────────────
#define LED_PIN_DEFAULT   5
#define NUM_LEDS_DEFAULT  480
#define MAX_LEDS          2000   // limite máximo para alocação estática

// ── Dual WiFi ──────────────────────────────────────────────
const char* WIFI_SSID_1  = "PLT-DIR";
const char* WIFI_PASS_1  = "epaminondas";
const char* WIFI_SSID_2  = "PLT-DIR-5G";
const char* WIFI_PASS_2  = "epaminondas";
const int   WIFI_TIMEOUT = 15;

// ── MQTT ───────────────────────────────────────────────────
const char* MQTT_HOST = "10.0.0.141";
const int   MQTT_PORT = 1883;
const char* MQTT_USER = "mqtt";
const char* MQTT_PASS = "planeta";

// ── Tópicos MQTT ───────────────────────────────────────────
char TOPIC_DISCOVERY[60];
char TOPIC_CMD[60];
char TOPIC_STATUS[60];
char TOPIC_NOME[60];
char TOPIC_CONFIG[60];

// ── Parâmetros de hardware ─────────────────────────────────
int  ledPin         = LED_PIN_DEFAULT;
int  numLeds        = NUM_LEDS_DEFAULT;
bool configRecebida = false;
bool fitaIniciada   = false;

// ── Objetos globais ────────────────────────────────────────
// Inicialização ESTÁTICA — evita problemas com alocação dinâmica
Adafruit_NeoPixel strip(NUM_LEDS_DEFAULT, LED_PIN_DEFAULT, NEO_GRB + NEO_KHZ800);
WiFiClient        wifiClient;
PubSubClient      mqtt(wifiClient);
WebServer         server(80);

// ── Estado da fita ─────────────────────────────────────────
int           efeitoAtual    = 16;
int           brilho         = 100;
int           velocidade     = 50;
unsigned long ultimoTempo    = 0;
bool          estadoPiscar   = false;
int           posCometa      = 0;
uint8_t       arcoIrisOffset = 0;
int           ledAtual       = 0;
float         fase           = 0.0;
int           passo          = 0;

// ── Identificação ──────────────────────────────────────────
String macStr        = "";
String nomeFita      = "";
String redeConectada = "";
String statusConexao = "Inicializando...";
unsigned long bootTime = 0;

// ── Log MQTT ───────────────────────────────────────────────
#define LOG_SIZE 10
struct LogEntry {
    String direcao;
    String topico;
    String payload;
    unsigned long ts;
};
LogEntry mqttLog[LOG_SIZE];
int logIndex = 0;

// ── Nomes dos efeitos ──────────────────────────────────────
const char* NOMES_EFEITOS[] = {
    "Confete", "Cometa", "Piscar", "Arco-Iris",
    "Branco Frio", "Branco Quente", "Azul", "Verde", "Vermelho",
    "Arco-Iris Rotativo", "Progressivo", "Sequencial",
    "Chuva de Estrelas", "Policia", "Explosao Central", "Brasil", "Desligado"
};

// ── Protótipos ─────────────────────────────────────────────
void conectarWiFi();
void conectarMQTT();
void publicarDiscovery();
void publicarStatus();
void executarEfeito();
void resetarEfeitos();
void adicionarLog(String direcao, String topico, String payload);
void setupWebServer();
void handleRoot();
void handleStatus();
uint32_t wheel(byte pos);

// ══════════════════════════════════════════════════════════
// LOG MQTT
// ══════════════════════════════════════════════════════════
void adicionarLog(String direcao, String topico, String payload) {
    mqttLog[logIndex % LOG_SIZE] = { direcao, topico, payload, millis() };
    logIndex++;
}

// ══════════════════════════════════════════════════════════
// CALLBACK MQTT
// ══════════════════════════════════════════════════════════
void onMensagemMQTT(char* topic, byte* payload, unsigned int length) {
    char msg[length + 1];
    memcpy(msg, payload, length);
    msg[length] = '\0';

    String topicStr = String(topic);
    adicionarLog("RX", topicStr, String(msg));
    Serial.println("[MQTT RX] " + topicStr + " : " + String(msg));

    // Config de hardware
    if (topicStr == String(TOPIC_CONFIG)) {
        StaticJsonDocument<128> doc;
        if (deserializeJson(doc, msg)) return;
        if (doc.containsKey("num_leds")) numLeds = constrain(doc["num_leds"].as<int>(), 1, MAX_LEDS);
        if (doc.containsKey("led_pin"))  ledPin  = doc["led_pin"].as<int>();
        configRecebida = true;
        Serial.printf("[CONFIG] num_leds=%d led_pin=%d\n", numLeds, ledPin);

        // Reconfigura a fita se já iniciada
        if (fitaIniciada) {
            strip.updateLength(numLeds);
            strip.setPin(ledPin);
            strip.begin();
            strip.clear();
            strip.show();
            Serial.printf("[FITA] Reconfigurada: %d LEDs pino %d\n", numLeds, ledPin);
            publicarDiscovery();
        }
        return;
    }

    // Nome personalizado
    if (topicStr == String(TOPIC_NOME)) {
        nomeFita = String(msg);
        Serial.println("[NOME] " + nomeFita);
        publicarDiscovery();
        return;
    }

    // Comando de efeito
    if (topicStr == String(TOPIC_CMD)) {
        if (!fitaIniciada) return;
        StaticJsonDocument<200> doc;
        if (deserializeJson(doc, msg)) return;

        if (doc.containsKey("efeito")) {
            int novo = doc["efeito"].as<int>();
            if (novo != efeitoAtual) { efeitoAtual = novo; resetarEfeitos(); }
        }
        if (doc.containsKey("brilho")) {
            brilho = constrain(doc["brilho"].as<int>(), 0, 255);
            strip.setBrightness(brilho);
        }
        if (doc.containsKey("vel")) {
            velocidade = 201 - constrain(doc["vel"].as<int>(), 1, 200);
        }
        publicarStatus();
    }
}

// ══════════════════════════════════════════════════════════
// PÁGINA WEB — DIAGNÓSTICO
// ══════════════════════════════════════════════════════════
void handleRoot() {
    String html = "<!DOCTYPE html><html lang='pt-BR'><head>";
    html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
    html += "<meta http-equiv='refresh' content='5'>";
    html += "<title>" + nomeFita + " — Diagnostico</title>";
    html += "<style>";
    html += "*{box-sizing:border-box;margin:0;padding:0}";
    html += "body{font-family:system-ui,sans-serif;background:#f5f6f8;color:#1a1d23;font-size:14px}";
    html += ".topbar{background:#fff;border-bottom:1px solid #e2e5ea;padding:12px 20px;display:flex;align-items:center;justify-content:space-between;box-shadow:0 1px 3px rgba(0,0,0,.08)}";
    html += ".topbar-title{font-weight:700;font-size:15px}";
    html += ".topbar-sub{font-size:11px;color:#6b7280}";
    html += ".dot{width:8px;height:8px;border-radius:50%;display:inline-block;margin-right:4px}";
    html += ".green{background:#22c55e;animation:pulse 2s infinite}.red{background:#ef4444}.yellow{background:#f59e0b}";
    html += "@keyframes pulse{0%,100%{opacity:1}50%{opacity:.3}}";
    html += ".container{max-width:700px;margin:0 auto;padding:16px;display:flex;flex-direction:column;gap:10px}";
    html += ".card{background:#fff;border:1px solid #e2e5ea;border-radius:8px;overflow:hidden;box-shadow:0 1px 3px rgba(0,0,0,.05)}";
    html += ".card-header{background:#f5f6f8;padding:8px 14px;border-bottom:1px solid #e2e5ea;font-size:10px;font-weight:700;text-transform:uppercase;letter-spacing:.06em;color:#6b7280}";
    html += ".card-body{padding:12px 14px}";
    html += ".row{display:flex;justify-content:space-between;align-items:center;padding:5px 0;border-bottom:1px solid #f0f1f3}";
    html += ".row:last-child{border-bottom:none}";
    html += ".lbl{color:#6b7280;font-size:12px}.val{font-weight:600;font-family:monospace;font-size:12px}";
    html += ".badge{display:inline-block;padding:2px 8px;border-radius:20px;font-size:11px;font-weight:600}";
    html += ".b-green{background:#dcfce7;color:#15803d}.b-red{background:#fee2e2;color:#b91c1c}.b-yellow{background:#fef9c3;color:#854d0e}";
    html += ".efeito{text-align:center;padding:16px;font-size:18px;font-weight:700;color:#0369a1;background:#e0f2fe;border-radius:6px;margin-bottom:10px}";
    html += ".bar-bg{height:6px;background:#e2e5ea;border-radius:3px;overflow:hidden;margin-top:3px}";
    html += ".bar-fill{height:100%;background:#22c55e;border-radius:3px}";
    html += ".status{padding:8px 12px;border-radius:6px;font-size:12px;margin-bottom:8px}";
    html += ".s-ok{background:#f0fdf4;color:#15803d;border:1px solid #bbf7d0}";
    html += ".s-warn{background:#fffbeb;color:#854d0e;border:1px solid #fde68a}";
    html += ".s-err{background:#fef2f2;color:#b91c1c;border:1px solid #fecaca}";
    html += ".log-t{width:100%;border-collapse:collapse;font-size:11px}";
    html += ".log-t th{background:#f5f6f8;padding:5px 8px;text-align:left;color:#6b7280;font-size:10px;text-transform:uppercase}";
    html += ".log-t td{padding:4px 8px;border-bottom:1px solid #f0f1f3;font-family:monospace}";
    html += ".log-t tr:last-child td{border:none}";
    html += ".rx{color:#2563eb;font-weight:700}.tx{color:#16a34a;font-weight:700}";
    html += ".note{text-align:center;font-size:10px;color:#9ca3af;margin-top:4px}";
    html += "</style></head><body>";

    // Topbar
    html += "<div class='topbar'><div>";
    html += "<div class='topbar-title'>" + nomeFita + "</div>";
    html += "<div class='topbar-sub'>WS2812B + MQTT v2.3.1 | MAC: " + macStr + "</div>";
    html += "</div><div>";
    if (mqtt.connected())
        html += "<span class='dot green'></span><span style='font-size:12px;color:#15803d'>MQTT OK</span>";
    else
        html += "<span class='dot red'></span><span style='font-size:12px;color:#b91c1c'>MQTT OFF</span>";
    html += "</div></div>";

    html += "<div class='container'>";

    // Status de conexao
    String sClass = (WiFi.status() == WL_CONNECTED && mqtt.connected()) ? "s-ok" :
                    (WiFi.status() == WL_CONNECTED) ? "s-warn" : "s-err";
    html += "<div class='status " + sClass + "'>" + statusConexao + "</div>";

    // Efeito atual
    String nomeEfeito = (efeitoAtual >= 0 && efeitoAtual <= 16) ? NOMES_EFEITOS[efeitoAtual] : "?";
    html += "<div class='card'><div class='card-header'>Efeito em Andamento</div><div class='card-body'>";
    html += "<div class='efeito'>" + nomeEfeito + "</div>";
    html += "<div class='lbl'>Brilho: " + String(brilho) + " / 255</div>";
    html += "<div class='bar-bg'><div class='bar-fill' style='width:" + String(brilho * 100 / 255) + "%'></div></div>";
    html += "</div></div>";

    // Dispositivo
    unsigned long upSec = (millis() - bootTime) / 1000;
    char uptime[20];
    snprintf(uptime, sizeof(uptime), "%02d:%02d:%02d", (int)(upSec/3600), (int)((upSec%3600)/60), (int)(upSec%60));

    html += "<div class='card'><div class='card-header'>Dispositivo</div><div class='card-body'>";
    html += "<div class='row'><span class='lbl'>MAC</span><span class='val'>" + macStr + "</span></div>";
    html += "<div class='row'><span class='lbl'>IP</span><span class='val'>" + WiFi.localIP().toString() + "</span></div>";
    html += "<div class='row'><span class='lbl'>Rede WiFi</span><span class='val'>" + redeConectada + "</span></div>";
    html += "<div class='row'><span class='lbl'>RSSI</span><span class='val'>" + String(WiFi.RSSI()) + " dBm</span></div>";
    html += "<div class='row'><span class='lbl'>Uptime</span><span class='val'>" + String(uptime) + "</span></div>";
    html += "</div></div>";

    // Config da fita
    html += "<div class='card'><div class='card-header'>Configuracao da Fita</div><div class='card-body'>";
    html += "<div class='row'><span class='lbl'>LEDs</span><span class='val'>" + String(numLeds) + "</span></div>";
    html += "<div class='row'><span class='lbl'>Pino</span><span class='val'>" + String(ledPin) + "</span></div>";
    html += "<div class='row'><span class='lbl'>Velocidade</span><span class='val'>" + String(velocidade) + " ms</span></div>";
    html += "<div class='row'><span class='lbl'>Config via broker</span>";
    html += configRecebida ? "<span class='badge b-green'>Sim</span>" : "<span class='badge b-yellow'>Padrao</span>";
    html += "</div></div></div>";

    // MQTT
    html += "<div class='card'><div class='card-header'>MQTT</div><div class='card-body'>";
    html += "<div class='row'><span class='lbl'>Broker</span><span class='val'>" + String(MQTT_HOST) + ":" + String(MQTT_PORT) + "</span></div>";
    html += "<div class='row'><span class='lbl'>Status</span>";
    html += mqtt.connected() ? "<span class='badge b-green'>Conectado</span>" : "<span class='badge b-red'>Desconectado</span>";
    html += "</div>";
    html += "<div class='row'><span class='lbl'>CMD</span><span class='val' style='font-size:10px'>" + String(TOPIC_CMD) + "</span></div>";
    html += "</div></div>";

    // Log MQTT
    html += "<div class='card'><div class='card-header'>Log MQTT</div>";
    html += "<table class='log-t'><thead><tr><th>Dir</th><th>t(s)</th><th>Topico</th><th>Payload</th></tr></thead><tbody>";
    int total = (logIndex < LOG_SIZE) ? logIndex : LOG_SIZE;
    for (int i = total - 1; i >= 0; i--) {
        int idx = (logIndex - 1 - i + LOG_SIZE * 100) % LOG_SIZE;
        String t = mqttLog[idx].topico;
        int sl = t.lastIndexOf('/');
        String subtopico = (sl >= 0) ? t.substring(sl+1) : t;
        html += "<tr><td class='" + String(mqttLog[idx].direcao == "RX" ? "rx" : "tx") + "'>" + mqttLog[idx].direcao + "</td>";
        html += "<td>" + String(mqttLog[idx].ts / 1000) + "</td>";
        html += "<td>" + subtopico + "</td>";
        html += "<td>" + mqttLog[idx].payload.substring(0, 50) + "</td></tr>";
    }
    if (total == 0) html += "<tr><td colspan='4' style='text-align:center;color:#9ca3af;padding:10px'>Sem mensagens</td></tr>";
    html += "</tbody></table></div>";

    html += "<p class='note'>Atualiza a cada 5s | <a href='/status'>JSON</a></p>";
    html += "</div></body></html>";

    server.send(200, "text/html; charset=utf-8", html);
}

void handleStatus() {
    StaticJsonDocument<512> doc;
    doc["mac"]           = macStr;
    doc["ip"]            = WiFi.localIP().toString();
    doc["nome"]          = nomeFita;
    doc["rede"]          = redeConectada;
    doc["rssi"]          = WiFi.RSSI();
    doc["mqtt"]          = mqtt.connected();
    doc["efeito"]        = efeitoAtual;
    doc["efeito_nome"]   = NOMES_EFEITOS[efeitoAtual];
    doc["brilho"]        = brilho;
    doc["velocidade"]    = velocidade;
    doc["num_leds"]      = numLeds;
    doc["led_pin"]       = ledPin;
    doc["config_broker"] = configRecebida;
    doc["uptime_ms"]     = millis() - bootTime;
    doc["firmware"]      = "v2.3.1";
    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

void setupWebServer() {
    server.on("/",       handleRoot);
    server.on("/status", handleStatus);
    server.begin();
    Serial.println("[WEB] http://" + WiFi.localIP().toString());
}

// ══════════════════════════════════════════════════════════
// DUAL WiFi
// ══════════════════════════════════════════════════════════
void conectarWiFi() {
    Serial.println("[WiFi] Tentando: " + String(WIFI_SSID_1));
    statusConexao = "Conectando ao WiFi: " + String(WIFI_SSID_1) + "...";
    WiFi.begin(WIFI_SSID_1, WIFI_PASS_1);
    int t = 0;
    while (WiFi.status() != WL_CONNECTED && t < WIFI_TIMEOUT) { delay(1000); Serial.print("."); t++; }

    if (WiFi.status() == WL_CONNECTED) {
        redeConectada = String(WIFI_SSID_1);
        statusConexao = "WiFi OK: " + redeConectada + " | IP: " + WiFi.localIP().toString();
        Serial.println("\n[WiFi] OK! " + redeConectada + " | " + WiFi.localIP().toString());
        return;
    }

    Serial.println("\n[WiFi] Falhou. Tentando: " + String(WIFI_SSID_2));
    statusConexao = "WiFi primario falhou. Tentando: " + String(WIFI_SSID_2) + "...";
    WiFi.begin(WIFI_SSID_2, WIFI_PASS_2);
    t = 0;
    while (WiFi.status() != WL_CONNECTED && t < WIFI_TIMEOUT) { delay(1000); Serial.print("."); t++; }

    if (WiFi.status() == WL_CONNECTED) {
        redeConectada = String(WIFI_SSID_2);
        statusConexao = "WiFi OK (secundario): " + redeConectada + " | IP: " + WiFi.localIP().toString();
        Serial.println("\n[WiFi] OK! " + redeConectada + " | " + WiFi.localIP().toString());
        return;
    }

    Serial.println("\n[WiFi] FALHOU em ambas. Reiniciando...");
    statusConexao = "Falha em ambas as redes. Reiniciando...";
    delay(3000);
    ESP.restart();
}

// ══════════════════════════════════════════════════════════
// MQTT
// ══════════════════════════════════════════════════════════
void conectarMQTT() {
    String clientId = "fita-led-" + macStr;
    while (!mqtt.connected()) {
        Serial.print("[MQTT] Conectando...");
        if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
            Serial.println(" OK!");
            mqtt.subscribe(TOPIC_CMD);
            mqtt.subscribe(TOPIC_NOME);
            mqtt.subscribe(TOPIC_CONFIG);
            publicarDiscovery();
            publicarStatus();
            statusConexao = "OK | WiFi: " + redeConectada + " | MQTT: " + String(MQTT_HOST) + " | " + String(numLeds) + " LEDs";
        } else {
            Serial.println(" Falhou. Aguardando 5s...");
            delay(5000);
        }
    }
}

// ══════════════════════════════════════════════════════════
// PUBLICAÇÕES
// ══════════════════════════════════════════════════════════
void publicarDiscovery() {
    StaticJsonDocument<256> doc;
    doc["mac"]      = macStr;
    doc["ip"]       = WiFi.localIP().toString();
    doc["nome"]     = nomeFita;
    doc["num_leds"] = numLeds;
    doc["led_pin"]  = ledPin;
    char payload[256];
    serializeJson(doc, payload);
    mqtt.publish(TOPIC_DISCOVERY, payload, true);
    adicionarLog("TX", String(TOPIC_DISCOVERY), String(payload));
    Serial.println("[MQTT TX] Discovery: " + String(payload));
}

void publicarStatus() {
    StaticJsonDocument<128> doc;
    doc["efeito"]   = efeitoAtual;
    doc["brilho"]   = brilho;
    doc["vel"]      = 201 - velocidade;
    doc["ip"]       = WiFi.localIP().toString();
    doc["num_leds"] = numLeds;
    char payload[128];
    serializeJson(doc, payload);
    mqtt.publish(TOPIC_STATUS, payload, true);
    adicionarLog("TX", String(TOPIC_STATUS), String(payload));
}

// ══════════════════════════════════════════════════════════
// SETUP
// ══════════════════════════════════════════════════════════
void setup() {
    Serial.begin(19200);
    delay(500);
    Serial.println("\n=== Fita LED WS2812B + MQTT v2.3.1 ===");
    bootTime = millis();

    conectarWiFi();

    // MAC e tópicos
    macStr = WiFi.macAddress();
    macStr.replace(":", "");
    macStr.toUpperCase();
    nomeFita = "Fita " + macStr.substring(0, 6);

    snprintf(TOPIC_DISCOVERY, sizeof(TOPIC_DISCOVERY), "fita-led/discovery/%s", macStr.c_str());
    snprintf(TOPIC_CMD,       sizeof(TOPIC_CMD),       "fita-led/%s/cmd",       macStr.c_str());
    snprintf(TOPIC_STATUS,    sizeof(TOPIC_STATUS),    "fita-led/%s/status",    macStr.c_str());
    snprintf(TOPIC_NOME,      sizeof(TOPIC_NOME),      "fita-led/%s/nome",      macStr.c_str());
    snprintf(TOPIC_CONFIG,    sizeof(TOPIC_CONFIG),    "fita-led/%s/config",    macStr.c_str());

    Serial.println("[INFO] MAC    : " + macStr);
    Serial.println("[INFO] IP     : " + WiFi.localIP().toString());
    Serial.println("[INFO] CONFIG : " + String(TOPIC_CONFIG));

    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(onMensagemMQTT);
    mqtt.setBufferSize(512);

    // Conecta MQTT e assina config antes de inicializar a fita
    String clientId = "fita-led-" + macStr;
    while (!mqtt.connected()) {
        if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
            Serial.println("[MQTT] Conectado — aguardando config...");
            mqtt.subscribe(TOPIC_CONFIG);
            mqtt.subscribe(TOPIC_NOME);
        } else { delay(3000); }
    }

    // Inicia servidor web para mostrar progresso durante espera
    setupWebServer();

    // Aguarda config por 5s processando requests web simultaneamente
    Serial.print("[CONFIG] Aguardando");
    unsigned long t = millis();
    while (!configRecebida && millis() - t < 5000) {
        mqtt.loop();
        server.handleClient();
        delay(50);
        Serial.print(".");
    }
    Serial.println();

    if (configRecebida) {
        Serial.printf("[CONFIG] Aplicada: num_leds=%d led_pin=%d\n", numLeds, ledPin);
    } else {
        Serial.printf("[CONFIG] Nao encontrada. Padrao: num_leds=%d led_pin=%d\n", NUM_LEDS_DEFAULT, LED_PIN_DEFAULT);
    }

    // ── Inicialização ESTÁTICA da fita ─────────────────────
    // updateLength e setPin recoonfiguram sem alocação dinâmica
    strip.updateLength(numLeds);
    strip.setPin(ledPin);
    strip.begin();
    strip.setBrightness(brilho);
    strip.clear();
    strip.show();
    fitaIniciada = true;
    Serial.printf("[FITA] Iniciada: %d LEDs no pino %d\n", numLeds, ledPin);

    // Assina demais tópicos e publica presença
    mqtt.subscribe(TOPIC_CMD);
    publicarDiscovery();
    publicarStatus();

    statusConexao = "OK | WiFi: " + redeConectada + " | MQTT: " + String(MQTT_HOST) + " | " + String(numLeds) + " LEDs pino " + String(ledPin);
    Serial.println("[OK] Sistema pronto! http://" + WiFi.localIP().toString());
}

// ══════════════════════════════════════════════════════════
// LOOP
// ══════════════════════════════════════════════════════════
void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        statusConexao = "WiFi desconectado. Reconectando...";
        conectarWiFi();
        setupWebServer();
    }
    if (!mqtt.connected()) {
        statusConexao = "MQTT desconectado. Reconectando...";
        conectarMQTT();
    }

    mqtt.loop();
    server.handleClient();

    if (fitaIniciada && millis() - ultimoTempo >= (unsigned long)velocidade) {
        ultimoTempo = millis();
        executarEfeito();
    }
}

// ══════════════════════════════════════════════════════════
// EFEITOS
// ══════════════════════════════════════════════════════════
void resetarEfeitos() {
    estadoPiscar = false; posCometa = 0; arcoIrisOffset = 0;
    ledAtual = 0; fase = 0.0; passo = 0;
    strip.clear(); strip.show();
}

void executarEfeito() {
    switch (efeitoAtual) {
        case 0:  efeitoConfete();               break;
        case 1:  efeitoCometa();                break;
        case 2:  efeitoPiscar();                break;
        case 3:  efeitoArcoIris();              break;
        case 4:  efeitoCorFixa(255,255,255);    break;
        case 5:  efeitoCorFixa(255,160, 60);    break;
        case 6:  efeitoCorFixa(  0,  0,255);    break;
        case 7:  efeitoCorFixa(  0,255,  0);    break;
        case 8:  efeitoCorFixa(255,  0,  0);    break;
        case 9:  efeitoArcoIrisRotativo();      break;
        case 10: efeitoProgressivoPorSetores(); break;
        case 11: efeitoAcenderSequencial();     break;
        case 12: efeitoChuvaDeEstrelas();       break;
        case 13: efeitoPolicia();               break;
        case 14: efeitoExplosaoCentral();       break;
        case 15: efeitoBrasil();                break;
        case 16: efeitoCorFixa(0, 0, 0);       break;
        default: efeitoCorFixa(0, 0, 0);       break;
    }
}

uint32_t wheel(byte pos) {
    if (pos < 85)        return strip.Color(pos*3, 255-pos*3, 0);
    else if (pos < 170) { pos-=85;  return strip.Color(255-pos*3, 0, pos*3); }
    else                { pos-=170; return strip.Color(0, pos*3, 255-pos*3); }
}

void efeitoConfete() {
    strip.setPixelColor(random(numLeds), strip.Color(random(255),random(255),random(255)));
    strip.show();
}

void efeitoCometa() {
    int cauda = 15; strip.clear();
    for (int j = 0; j < cauda; j++) {
        int p = posCometa - j;
        if (p >= 0 && p < numLeds) {
            float b = 1.0 - float(j)/cauda;
            strip.setPixelColor(p, strip.Color(255*b, 100*b, 0));
        }
    }
    strip.show(); posCometa++;
    if (posCometa > numLeds + cauda) posCometa = 0;
}

void efeitoPiscar() {
    if (estadoPiscar) strip.fill(strip.Color(255,255,255));
    else              strip.clear();
    strip.show(); estadoPiscar = !estadoPiscar;
}

void efeitoArcoIris() {
    for (int i = 0; i < numLeds; i++)
        strip.setPixelColor(i, wheel((i+arcoIrisOffset)&255));
    strip.show(); arcoIrisOffset++;
}

void efeitoArcoIrisRotativo() {
    static uint16_t offset = 0;
    for (int i = 0; i < numLeds; i++)
        strip.setPixelColor(i, wheel((i*256/numLeds+offset)&255));
    strip.show(); offset++;
}

void efeitoCorFixa(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < numLeds; i++)
        strip.setPixelColor(i, strip.Color(r,g,b));
    strip.show();
}

void efeitoProgressivoPorSetores() {
    static int s = 0; int tot = 10, lps = numLeds/tot;
    strip.clear();
    for (int i = 0; i <= s && i < tot; i++)
        for (int j = 0; j < lps; j++) {
            int idx = i*lps+j;
            if (idx < numLeds) strip.setPixelColor(idx, strip.Color(0,150,255));
        }
    strip.show(); if (++s >= tot) s = 0;
}

void efeitoAcenderSequencial() {
    if (ledAtual < numLeds) {
        strip.setPixelColor(ledAtual, strip.Color(255,160,60));
        strip.show(); ledAtual++;
    }
}

void efeitoChuvaDeEstrelas() {
    for (int i = 0; i < numLeds; i++) {
        uint32_t cor = strip.getPixelColor(i);
        strip.setPixelColor(i, ((cor>>16)&0xFF)*4/5, ((cor>>8)&0xFF)*4/5, (cor&0xFF)*4/5);
    }
    for (int i = 0; i < 10; i++)
        strip.setPixelColor(random(numLeds), strip.Color(255,200,100));
    strip.show();
}

void efeitoPolicia() {
    static bool a = false; int m = numLeds/2;
    for (int i = 0; i < numLeds; i++)
        strip.setPixelColor(i, a ? (i<m?strip.Color(255,0,0):strip.Color(0,0,255))
                                 : (i<m?strip.Color(0,0,255):strip.Color(255,0,0)));
    strip.show(); a = !a;
}

void efeitoExplosaoCentral() {
    static int p = 0; int c = numLeds/2; strip.clear();
    for (int i = 0; i <= p; i++) {
        if (c-i >= 0)       strip.setPixelColor(c-i, strip.Color(255,200,50));
        if (c+i < numLeds)  strip.setPixelColor(c+i, strip.Color(255,200,50));
    }
    strip.show(); if (++p >= numLeds/2) p = 0;
}

void efeitoBrasil() {
    static uint16_t offset = 0;
    uint32_t cores[4] = {
        strip.Color(0,156,59), strip.Color(255,223,0),
        strip.Color(0,39,118), strip.Color(255,255,255)
    };
    for (int i = 0; i < numLeds; i++)
        strip.setPixelColor(i, cores[((i+offset)/25)%4]);
    strip.show(); offset++;
}
