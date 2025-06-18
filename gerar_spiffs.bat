@echo off
setlocal EnableDelayedExpansion

:: Caminho para o executável mkspiffs
set MKSPIFFS="C:\Users\epami\AppData\Local\Programs\Python\Python313\Scripts\mkspiffs-0.2.3-arduino-esp32-win32\mkspiffs.exe"

:: Configurações da flash
set PORT=COM8
set SIZE=1441792
set ADDRESS=0x290000

:: Diretório de dados relativo ao diretório atual
set DATADIR=.\esp32_led_web_project\data

echo ===============================================
echo  GERANDO SPIFFS.BIN E ENVIANDO PARA O ESP32
echo ===============================================

:: Verifica se mkspiffs.exe existe
if not exist %MKSPIFFS% (
    echo [ERRO] mkspiffs.exe nao encontrado em: %MKSPIFFS%
    pause
    exit /b
)

:: Verifica se a pasta de dados existe
if not exist %DATADIR% (
    echo [ERRO] Pasta de dados não encontrada: %DATADIR%
    pause
    exit /b
)

:: Gera a imagem spiffs.bin
echo [1] Gerando imagem spiffs.bin da pasta %DATADIR%
%MKSPIFFS% -c %DATADIR% -b 4096 -p 256 -s %SIZE% spiffs.bin

if not exist spiffs.bin (
    echo [ERRO] A imagem spiffs.bin nao foi gerada!
    pause
    exit /b
)

:: Debug se tem arquivos na pasta data
echo [DEBUG] Listando arquivos gerados em spiffs.bin
type nul | %MKSPIFFS% -l -b 4096 -p 256 -s %SIZE% spiffs.bin

:: Envia spiffs.bin para o ESP32
echo [2] Enviando spiffs.bin para o ESP32 via porta %PORT%
python -m esptool --chip esp32 --port %PORT% --baud 460800 write_flash %ADDRESS% spiffs.bin

echo [OK] Processo finalizado com sucesso.
pause
