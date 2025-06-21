# 🧰 Tutorial: Instalação e Uso de Ferramentas para SPIFFS no ESP32 (macOS/Linux)

Este guia ensina como instalar as ferramentas necessárias e fazer o upload de arquivos para o SPIFFS do ESP32 sem utilizar a IDE do Arduino.

---

## 📦 1. Instalar Ferramentas Necessárias

### 🔹 Passo 1: Baixe o instalador

Baixe o script [`install_spiffs_tools.sh`](install_spiffs_tools.sh) e torne-o executável:

```bash
chmod +x install_spiffs_tools.sh
```

### 🔹 Passo 2: Execute o instalador

```bash
./install_spiffs_tools.sh
```

Este script irá:
- Instalar o [Homebrew](https://brew.sh/) se não estiver presente
- Instalar `esptool.py`
- Instalar `mkspiffs`

---

## 📁 2. Estrutura de Diretórios do Projeto

Seu projeto deve conter:

```
esp32_led_spiffs/
├── main.ino             # Código do ESP32
├── upload_spiffs_esp32.sh   # Script para enviar SPIFFS
└── data/
    ├── index.html
    └── style.css
```

---

## 🚀 3. Enviar Arquivos para o SPIFFS

### 🔹 Passo 1: Baixe e torne o script executável

```bash
chmod +x upload_spiffs_esp32.sh
```

### 🔹 Passo 2: Edite o script se necessário

Abra `upload_spiffs_esp32.sh` e edite a variável `PORT` para refletir a porta serial do seu ESP32:

```bash
PORT=/dev/tty.usbserial-XXXXX
```

Use `ls /dev/tty.*` para descobrir o nome da porta.

### 🔹 Passo 3: Execute o script

```bash
./upload_spiffs_esp32.sh
```

Este script irá:
- Criar a imagem SPIFFS com base na pasta `/data`
- Gravar a imagem no ESP32 no endereço `0x290000`

---

## ✅ Conclusão

Você agora pode atualizar arquivos no SPIFFS sem a IDE Arduino. Após o upload, reinicie seu ESP32 e ele carregará os arquivos diretamente da memória.

Se tiver dúvidas ou precisar adaptar para outra partição, entre em contato!

Desenvolvido por **Epaminondas de Souza Lage**
