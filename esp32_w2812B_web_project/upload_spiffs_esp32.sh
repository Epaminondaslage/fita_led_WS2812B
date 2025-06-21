#!/bin/bash

# === Configurações ===
PORT=/dev/ttyUSB0
SIZE=0x170000
OFFSET=0x290000
MKSPIFFS=mkspiffs
ESPTOOL=esptool.py

# === Verificar ferramentas ===
command -v $MKSPIFFS >/dev/null 2>&1 || { echo >&2 "Erro: mkspiffs não encontrado."; exit 1; }
command -v python3 >/dev/null 2>&1 || { echo >&2 "Erro: Python3 não encontrado."; exit 1; }

# === Criar diretório de build se necessário ===
mkdir -p build

# === Gerar imagem SPIFFS ===
echo "Gerando imagem SPIFFS da pasta /data ..."
$MKSPIFFS -c data -b 4096 -p 256 -s $SIZE build/spiffs.bin

# === Enviar imagem SPIFFS para ESP32 ===
echo "Gravando imagem SPIFFS no ESP32 na porta $PORT ..."
python3 $ESPTOOL --chip esp32 --port $PORT --baud 921600 write_flash $OFFSET build/spiffs.bin

echo "✅ Processo concluído com sucesso."
