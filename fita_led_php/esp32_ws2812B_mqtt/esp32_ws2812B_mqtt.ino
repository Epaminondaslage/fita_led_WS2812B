/*
  ================================================================================
  Projeto : Controle de Fita de LED WS2812B com ESP32 + MQTT
  Autor   : Epaminondas de Souza Lage
  Versão  : 2.3.0

  Descrição:
    Controle de fita WS2812B via MQTT com servidor web de diagnóstico.
    O mesmo firmware roda em todos os ESP32.

  Novidades v2.3.0:
    - Dual WiFi: tenta rede primária, se falhar usa rede secundária
    - Servidor HTTP na porta 80: página de diagnóstico em tempo real
    - Serial em 19200 baud
    - Padrão de 480 LEDs
    - Log de mensagens MQTT recebidas e enviadas
    - Exibe efeito em andamento, MAC, IP, uptime, status MQTT

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
    - WebServer (incluída no ESP32 Arduino Core)

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

// ── Dual WiFi — tenta primária, se falhar usa secundária ───
const char* WIFI_SSID_1  = "PLT-DIR";
const char* WIFI_PASS_1  = "epaminondas";
const char* WIFI_SSID_2  = "PLT-DIR-5G";       // rede secundária
const char* WIFI_PASS_2  = "epaminondas";       // senha secundária
const int   WIFI_TIMEOUT = 15;                  // segundos por tentativa

// ── Configuração MQTT ──────────────────────────────────────
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

// ── Objetos globais ────────────────────────────────────────
Adafruit_NeoPixel* strip = nullptr;
WiFiClient         wifiClient;
PubSubClient       mqtt(wifiClient);
WebServer          server(80);

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
String macStr      = "";
String nomeFita    = "";
String redeConectada = "";
unsigned long bootTime = 0;

// ── Log de mensagens MQTT (últimas 10) ────────────────────
#define LOG_SIZE 10
struct LogEntry {
    String direcao;  // "RX" ou "TX"
    String topico;
    String payload;
    unsigned long ts;
};
LogEntry mqttLog[LOG_SIZE];
int logIndex = 0;

// ── Status de conexão para a página web ───────────────────
String statusConexao = "Inicializando...";

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

    // ── Config de hardware ─────────────────────────────────
    if (topicStr == String(TOPIC_CONFIG)) {
        StaticJsonDocument<128> doc;
        if (deserializeJson(doc, msg)) return;
        if (doc.containsKey("num_leds")) numLeds = doc["num_leds"].as<int>();
        if (doc.containsKey("led_pin"))  ledPin  = doc["led_pin"].as<int>();
        configRecebida = true;
        Serial.printf("[CONFIG] num_leds=%d led_pin=%d\n", numLeds, ledPin);
        return;
    }

    // ── Nome personalizado ─────────────────────────────────
    if (topicStr == String(TOPIC_NOME)) {
        nomeFita = String(msg);
        Serial.println("[NOME] " + nomeFita);
        publicarDiscovery();
        return;
    }

    // ── Comando de efeito ──────────────────────────────────
    if (topicStr == String(TOPIC_CMD)) {
        if (!strip) return;
        StaticJsonDocument<200> doc;
        if (deserializeJson(doc, msg)) return;

        if (doc.containsKey("efeito")) {
            int novo = doc["efeito"].as<int>();
            if (novo != efeitoAtual) { efeitoAtual = novo; resetarEfeitos(); }
        }
        if (doc.containsKey("brilho")) {
            brilho = constrain(doc["brilho"].as<int>(), 0, 255);
            strip->setBrightness(brilho);
        }
        if (doc.containsKey("vel")) {
            velocidade = 201 - constrain(doc["vel"].as<int>(), 1, 200);
        }
        publicarStatus();
    }
}

// ══════════════════════════════════════════════════════════
// PÁGINA WEB — HTML principal
// ══════════════════════════════════════════════════════════
void handleRoot() {
    String html = R"rawhtml(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta http-equiv="refresh" content="5">
<title>Fita LED — Diagnostico</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0}
  body{font-family:system-ui,sans-serif;background:#f5f6f8;color:#1a1d23;font-size:14px}
  .topbar{background:#fff;border-bottom:1px solid #e2e5ea;padding:12px 20px;
          display:flex;align-items:center;justify-content:space-between;
          box-shadow:0 1px 3px rgba(0,0,0,.08)}
  .topbar-title{font-weight:700;font-size:16px}
  .topbar-sub{font-size:11px;color:#6b7280}
  .dot{width:8px;height:8px;border-radius:50%;display:inline-block;margin-right:5px}
  .dot-green{background:#22c55e;animation:pulse 2s infinite}
  .dot-red{background:#ef4444}
  .dot-yellow{background:#f59e0b}
  @keyframes pulse{0%,100%{opacity:1}50%{opacity:.4}}
  .container{max-width:800px;margin:0 auto;padding:16px;display:flex;flex-direction:column;gap:12px}
  .card{background:#fff;border:1px solid #e2e5ea;border-radius:8px;overflow:hidden;
        box-shadow:0 1px 3px rgba(0,0,0,.06)}
  .card-header{background:#f5f6f8;padding:10px 16px;border-bottom:1px solid #e2e5ea;
               font-size:11px;font-weight:600;text-transform:uppercase;letter-spacing:.05em;color:#6b7280}
  .card-body{padding:14px 16px}
  .row{display:flex;justify-content:space-between;align-items:center;
       padding:6px 0;border-bottom:1px solid #f0f1f3}
  .row:last-child{border-bottom:none}
  .row-label{color:#6b7280;font-size:12px}
  .row-val{font-weight:600;font-family:monospace;font-size:13px}
  .badge{display:inline-block;padding:2px 10px;border-radius:20px;font-size:11px;font-weight:600}
  .badge-on{background:#dcfce7;color:#15803d}
  .badge-off{background:#fee2e2;color:#b91c1c}
  .badge-yellow{background:#fef9c3;color:#854d0e}
  .log-table{width:100%;border-collapse:collapse;font-size:11px}
  .log-table th{background:#f5f6f8;padding:6px 10px;text-align:left;color:#6b7280;
                font-size:10px;text-transform:uppercase;letter-spacing:.05em}
  .log-table td{padding:5px 10px;border-bottom:1px solid #f0f1f3;font-family:monospace}
  .log-table tr:last-child td{border-bottom:none}
  .rx{color:#2563eb}.tx{color:#16a34a}
  .efeito-badge{background:#e0f2fe;color:#0369a1;padding:4px 12px;
                border-radius:20px;font-weight:700;font-size:14px}
  .progress-bar{height:6px;background:#e2e5ea;border-radius:3px;overflow:hidden;margin-top:4px}
  .progress-fill{height:100%;background:#22c55e;border-radius:3px;transition:width .3s}
  .status-conn{padding:8px 12px;border-radius:6px;font-size:12px;margin-bottom:8px}
  .conn-ok{background:#f0fdf4;color:#15803d;border:1px solid #bbf7d0}
  .conn-err{background:#fef2f2;color:#b91c1c;border:1px solid #fecaca}
  .conn-warn{background:#fffbeb;color:#854d0e;border:1px solid #fde68a}
  .refresh-note{text-align:center;font-size:11px;color:#9ca3af;margin-top:4px}
</style>
</head>
<body>
<div class="topbar">
  <div>
    <div class="topbar-title">)rawhtml";

    html += nomeFita;
    html += R"rawhtml( — Diagnóstico</div>
    <div class="topbar-sub">Fita LED WS2812B + MQTT v2.3.0</div>
  </div>
  <div>)rawhtml";

    if (mqtt.connected())
        html += "<span class='dot dot-green'></span><span style='font-size:12px;color:#15803d'>MQTT OK</span>";
    else
        html += "<span class='dot dot-red'></span><span style='font-size:12px;color:#b91c1c'>MQTT OFF</span>";

    html += R"rawhtml(
  </div>
</div>
<div class="container">
)rawhtml";

    // Status de conexão
    html += "<div class='status-conn ";
    if (WiFi.status() == WL_CONNECTED && mqtt.connected()) html += "conn-ok'>";
    else if (WiFi.status() == WL_CONNECTED)                html += "conn-warn'>";
    else                                                    html += "conn-err'>";
    html += statusConexao + "</div>";

    // Card — Efeito atual
    String nomeEfeito = (efeitoAtual >= 0 && efeitoAtual <= 16) ? NOMES_EFEITOS[efeitoAtual] : "Desconhecido";
    html += "<div class='card'>";
    html += "<div class='card-header'>Efeito em Andamento</div>";
    html += "<div class='card-body' style='text-align:center;padding:20px'>";
    html += "<span class='efeito-badge'>" + nomeEfeito + "</span>";
    html += "<div style='margin-top:14px'>";
    html += "<div class='row-label'>Brilho</div>";
    html += "<div class='progress-bar'><div class='progress-fill' style='width:" + String(brilho * 100 / 255) + "%'></div></div>";
    html += "<div style='font-size:11px;color:#6b7280;margin-top:3px'>" + String(brilho) + " / 255</div>";
    html += "</div></div></div>";

    // Card — Informações do dispositivo
    unsigned long upSec = (millis() - bootTime) / 1000;
    int upH = upSec / 3600, upM = (upSec % 3600) / 60, upS = upSec % 60;
    char uptime[20];
    snprintf(uptime, sizeof(uptime), "%02d:%02d:%02d", upH, upM, upS);

    html += "<div class='card'><div class='card-header'>Dispositivo</div><div class='card-body'>";
    html += "<div class='row'><span class='row-label'>MAC</span><span class='row-val'>" + macStr + "</span></div>";
    html += "<div class='row'><span class='row-label'>IP</span><span class='row-val'>" + WiFi.localIP().toString() + "</span></div>";
    html += "<div class='row'><span class='row-label'>Rede WiFi</span><span class='row-val'>" + redeConectada + "</span></div>";
    html += "<div class='row'><span class='row-label'>RSSI</span><span class='row-val'>" + String(WiFi.RSSI()) + " dBm</span></div>";
    html += "<div class='row'><span class='row-label'>Uptime</span><span class='row-val'>" + String(uptime) + "</span></div>";
    html += "<div class='row'><span class='row-label'>Firmware</span><span class='row-val'>v2.3.0</span></div>";
    html += "</div></div>";

    // Card — Configuração da fita
    html += "<div class='card'><div class='card-header'>Configuração da Fita</div><div class='card-body'>";
    html += "<div class='row'><span class='row-label'>Num. LEDs</span><span class='row-val'>" + String(numLeds) + "</span></div>";
    html += "<div class='row'><span class='row-label'>Pino de dados</span><span class='row-val'>" + String(ledPin) + "</span></div>";
    html += "<div class='row'><span class='row-label'>Brilho</span><span class='row-val'>" + String(brilho) + " / 255</span></div>";
    html += "<div class='row'><span class='row-label'>Velocidade (intervalo)</span><span class='row-val'>" + String(velocidade) + " ms</span></div>";
    html += "<div class='row'><span class='row-label'>Config via broker</span>";
    html += configRecebida ? "<span class='badge badge-on'>Sim</span>" : "<span class='badge badge-yellow'>Padrão</span>";
    html += "</div></div></div>";

    // Card — MQTT
    html += "<div class='card'><div class='card-header'>MQTT</div><div class='card-body'>";
    html += "<div class='row'><span class='row-label'>Broker</span><span class='row-val'>" + String(MQTT_HOST) + ":" + String(MQTT_PORT) + "</span></div>";
    html += "<div class='row'><span class='row-label'>Status</span>";
    html += mqtt.connected() ? "<span class='badge badge-on'>Conectado</span>" : "<span class='badge badge-off'>Desconectado</span>";
    html += "</div>";
    html += "<div class='row'><span class='row-label'>Discovery</span><span class='row-val' style='font-size:10px'>" + String(TOPIC_DISCOVERY) + "</span></div>";
    html += "<div class='row'><span class='row-label'>CMD</span><span class='row-val' style='font-size:10px'>" + String(TOPIC_CMD) + "</span></div>";
    html += "</div></div>";

    // Card — Log MQTT
    html += "<div class='card'><div class='card-header'>Log MQTT (últimas mensagens)</div><div class='card-body' style='padding:0'>";
    html += "<table class='log-table'><thead><tr><th>Dir</th><th>ms</th><th>Tópico</th><th>Payload</th></tr></thead><tbody>";

    int total = (logIndex < LOG_SIZE) ? logIndex : LOG_SIZE;
    for (int i = total - 1; i >= 0; i--) {
        int idx = (logIndex - 1 - i + LOG_SIZE * 10) % LOG_SIZE;
        html += "<tr>";
        html += "<td class='" + String(mqttLog[idx].direcao == "RX" ? "rx" : "tx") + "'>" + mqttLog[idx].direcao + "</td>";
        html += "<td>" + String(mqttLog[idx].ts / 1000) + "s</td>";
        // Mostra só a última parte do tópico
        String t = mqttLog[idx].topico;
        int sl = t.lastIndexOf('/');
        html += "<td>" + (sl >= 0 ? t.substring(sl+1) : t) + "</td>";
        html += "<td>" + mqttLog[idx].payload.substring(0, 60) + "</td>";
        html += "</tr>";
    }
    if (total == 0) html += "<tr><td colspan='4' style='text-align:center;color:#9ca3af;padding:12px'>Nenhuma mensagem ainda</td></tr>";
    html += "</tbody></table></div></div>";

    html += "<p class='refresh-note'>Atualização automática a cada 5 segundos</p>";
    html += "</div></body></html>";

    server.send(200, "text/html; charset=utf-8", html);
}

// ── Endpoint JSON para dados em tempo real ─────────────────
void handleStatus() {
    StaticJsonDocument<512> doc;
    doc["mac"]          = macStr;
    doc["ip"]           = WiFi.localIP().toString();
    doc["nome"]         = nomeFita;
    doc["rede"]         = redeConectada;
    doc["rssi"]         = WiFi.RSSI();
    doc["mqtt"]         = mqtt.connected();
    doc["efeito"]       = efeitoAtual;
    doc["efeito_nome"]  = NOMES_EFEITOS[efeitoAtual];
    doc["brilho"]       = brilho;
    doc["velocidade"]   = velocidade;
    doc["num_leds"]     = numLeds;
    doc["led_pin"]      = ledPin;
    doc["config_broker"]= configRecebida;
    doc["uptime_ms"]    = millis() - bootTime;
    doc["firmware"]     = "v2.3.0";

    String json;
    serializeJson(doc, json);
    server.send(200, "application/json", json);
}

// ══════════════════════════════════════════════════════════
// SETUP WEB SERVER
// ══════════════════════════════════════════════════════════
void setupWebServer() {
    server.on("/",       handleRoot);
    server.on("/status", handleStatus);
    server.begin();
    Serial.println("[WEB] Servidor HTTP na porta 80");
    Serial.println("[WEB] http://" + WiFi.localIP().toString() + "/");
}

// ══════════════════════════════════════════════════════════
// CONEXÃO DUAL WiFi
// ══════════════════════════════════════════════════════════
void conectarWiFi() {
    // Tenta rede primária
    Serial.print("[WiFi] Tentando rede primaria: ");
    Serial.println(WIFI_SSID_1);
    statusConexao = "Conectando ao WiFi primário: " + String(WIFI_SSID_1) + "...";

    WiFi.begin(WIFI_SSID_1, WIFI_PASS_1);
    int t = 0;
    while (WiFi.status() != WL_CONNECTED && t < WIFI_TIMEOUT) {
        delay(1000);
        Serial.print(".");
        t++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        redeConectada = String(WIFI_SSID_1);
        statusConexao = "✓ Conectado ao WiFi: " + redeConectada + " | IP: " + WiFi.localIP().toString();
        Serial.println("\n[WiFi] OK! Rede: " + redeConectada + " | IP: " + WiFi.localIP().toString());
        return;
    }

    // Tenta rede secundária
    Serial.println("\n[WiFi] Primaria falhou. Tentando secundaria: " + String(WIFI_SSID_2));
    statusConexao = "WiFi primário falhou. Tentando: " + String(WIFI_SSID_2) + "...";

    WiFi.begin(WIFI_SSID_2, WIFI_PASS_2);
    t = 0;
    while (WiFi.status() != WL_CONNECTED && t < WIFI_TIMEOUT) {
        delay(1000);
        Serial.print(".");
        t++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        redeConectada = String(WIFI_SSID_2);
        statusConexao = "✓ Conectado ao WiFi secundário: " + redeConectada + " | IP: " + WiFi.localIP().toString();
        Serial.println("\n[WiFi] OK! Rede: " + redeConectada + " | IP: " + WiFi.localIP().toString());
        return;
    }

    // Ambas falharam
    statusConexao = "✗ Falha em ambas as redes WiFi. Reiniciando...";
    Serial.println("\n[WiFi] FALHOU em ambas as redes. Reiniciando...");
    delay(3000);
    ESP.restart();
}

// ══════════════════════════════════════════════════════════
// CONEXÃO MQTT
// ══════════════════════════════════════════════════════════
void conectarMQTT() {
    String clientId = "fita-led-" + macStr;
    Serial.print("[MQTT] Conectando...");
    statusConexao = "Conectando ao broker MQTT: " + String(MQTT_HOST) + "...";

    while (!mqtt.connected()) {
        if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
            Serial.println(" OK!");
            statusConexao = "✓ WiFi: " + redeConectada + " | MQTT: " + String(MQTT_HOST) + " | IP: " + WiFi.localIP().toString();
            mqtt.subscribe(TOPIC_CMD);
            mqtt.subscribe(TOPIC_NOME);
            mqtt.subscribe(TOPIC_CONFIG);
            publicarDiscovery();
            publicarStatus();
        } else {
            Serial.print(" Falhou (rc=");
            Serial.print(mqtt.state());
            Serial.println("). Aguardando 5s...");
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
    doc["efeito"]    = efeitoAtual;
    doc["brilho"]    = brilho;
    doc["vel"]       = 201 - velocidade;
    doc["ip"]        = WiFi.localIP().toString();
    doc["num_leds"]  = numLeds;

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
    Serial.println("\n=== Fita LED WS2812B + MQTT v2.3.0 ===");
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
    Serial.println("[INFO] Rede   : " + redeConectada);
    Serial.println("[INFO] CONFIG : " + String(TOPIC_CONFIG));

    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(onMensagemMQTT);
    mqtt.setBufferSize(512);

    // Conecta MQTT e aguarda config
    String clientId = "fita-led-" + macStr;
    while (!mqtt.connected()) {
        if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
            Serial.println("[MQTT] Conectado — aguardando config...");
            mqtt.subscribe(TOPIC_CONFIG);
            mqtt.subscribe(TOPIC_NOME);
        } else { delay(3000); }
    }

    // Inicia servidor web para mostrar progresso
    setupWebServer();

    // Aguarda config por 5s
    Serial.print("[CONFIG] Aguardando config do broker");
    unsigned long t = millis();
    while (!configRecebida && millis() - t < 5000) {
        mqtt.loop();
        server.handleClient();  // responde requisições durante espera
        delay(100);
        Serial.print(".");
    }
    Serial.println();

    if (!configRecebida) {
        Serial.printf("[CONFIG] Nao encontrada. Usando padrao: num_leds=%d led_pin=%d\n",
                      NUM_LEDS_DEFAULT, LED_PIN_DEFAULT);
    } else {
        Serial.printf("[CONFIG] Aplicada: num_leds=%d led_pin=%d\n", numLeds, ledPin);
    }

    // Inicializa fita
    strip = new Adafruit_NeoPixel(numLeds, ledPin, NEO_GRB + NEO_KHZ800);
    strip->begin();
    strip->setBrightness(brilho);
    strip->clear();
    strip->show();
    Serial.printf("[FITA] Iniciada: %d LEDs no pino %d\n", numLeds, ledPin);

    // Assina demais tópicos
    mqtt.subscribe(TOPIC_CMD);
    publicarDiscovery();
    publicarStatus();

    statusConexao = "✓ WiFi: " + redeConectada + " | MQTT OK | " + String(numLeds) + " LEDs no pino " + String(ledPin);
    Serial.println("[OK] Sistema pronto!");
    Serial.println("[WEB] Acesse: http://" + WiFi.localIP().toString());
}

// ══════════════════════════════════════════════════════════
// LOOP
// ══════════════════════════════════════════════════════════
void loop() {
    // Mantém conexões
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

    // Executa efeito
    if (strip && millis() - ultimoTempo >= (unsigned long)velocidade) {
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
    strip->clear(); strip->show();
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
    if (pos < 85)        return strip->Color(pos*3, 255-pos*3, 0);
    else if (pos < 170) { pos-=85;  return strip->Color(255-pos*3, 0, pos*3); }
    else                { pos-=170; return strip->Color(0, pos*3, 255-pos*3); }
}

void efeitoConfete() {
    strip->setPixelColor(random(numLeds), strip->Color(random(255),random(255),random(255)));
    strip->show();
}

void efeitoCometa() {
    int cauda = 15; strip->clear();
    for (int j = 0; j < cauda; j++) {
        int p = posCometa - j;
        if (p >= 0 && p < numLeds) {
            float b = 1.0 - float(j)/cauda;
            strip->setPixelColor(p, strip->Color(255*b, 100*b, 0));
        }
    }
    strip->show(); posCometa++;
    if (posCometa > numLeds + cauda) posCometa = 0;
}

void efeitoPiscar() {
    if (estadoPiscar) strip->fill(strip->Color(255,255,255));
    else              strip->clear();
    strip->show(); estadoPiscar = !estadoPiscar;
}

void efeitoArcoIris() {
    for (int i = 0; i < numLeds; i++)
        strip->setPixelColor(i, wheel((i+arcoIrisOffset)&255));
    strip->show(); arcoIrisOffset++;
}

void efeitoArcoIrisRotativo() {
    static uint16_t offset = 0;
    for (int i = 0; i < numLeds; i++)
        strip->setPixelColor(i, wheel((i*256/numLeds+offset)&255));
    strip->show(); offset++;
}

void efeitoCorFixa(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < numLeds; i++)
        strip->setPixelColor(i, strip->Color(r,g,b));
    strip->show();
}

void efeitoProgressivoPorSetores() {
    static int s = 0; int tot = 10, lps = numLeds/tot;
    strip->clear();
    for (int i = 0; i <= s && i < tot; i++)
        for (int j = 0; j < lps; j++) {
            int idx = i*lps+j;
            if (idx < numLeds) strip->setPixelColor(idx, strip->Color(0,150,255));
        }
    strip->show(); if (++s >= tot) s = 0;
}

void efeitoAcenderSequencial() {
    if (ledAtual < numLeds) {
        strip->setPixelColor(ledAtual, strip->Color(255,160,60));
        strip->show(); ledAtual++;
    }
}

void efeitoChuvaDeEstrelas() {
    for (int i = 0; i < numLeds; i++) {
        uint32_t cor = strip->getPixelColor(i);
        strip->setPixelColor(i, ((cor>>16)&0xFF)*4/5, ((cor>>8)&0xFF)*4/5, (cor&0xFF)*4/5);
    }
    for (int i = 0; i < 10; i++)
        strip->setPixelColor(random(numLeds), strip->Color(255,200,100));
    strip->show();
}

void efeitoPolicia() {
    static bool a = false; int m = numLeds/2;
    for (int i = 0; i < numLeds; i++)
        strip->setPixelColor(i, a ? (i<m?strip->Color(255,0,0):strip->Color(0,0,255))
                                  : (i<m?strip->Color(0,0,255):strip->Color(255,0,0)));
    strip->show(); a = !a;
}

void efeitoExplosaoCentral() {
    static int p = 0; int c = numLeds/2; strip->clear();
    for (int i = 0; i <= p; i++) {
        if (c-i >= 0)       strip->setPixelColor(c-i, strip->Color(255,200,50));
        if (c+i < numLeds)  strip->setPixelColor(c+i, strip->Color(255,200,50));
    }
    strip->show(); if (++p >= numLeds/2) p = 0;
}

void efeitoBrasil() {
    static uint16_t offset = 0;
    uint32_t cores[4] = {
        strip->Color(0,156,59), strip->Color(255,223,0),
        strip->Color(0,39,118), strip->Color(255,255,255)
    };
    for (int i = 0; i < numLeds; i++)
        strip->setPixelColor(i, cores[((i+offset)/25)%4]);
    strip->show(); offset++;
}
