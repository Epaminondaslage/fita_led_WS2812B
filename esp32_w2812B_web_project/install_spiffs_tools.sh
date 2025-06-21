#!/bin/bash

echo "🔧 Instalando ferramentas necessárias para SPIFFS no ESP32..."

# Verifica Homebrew
if ! command -v brew &> /dev/null
then
    echo "⚠️ Homebrew não encontrado. Instalando..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
else
    echo "✅ Homebrew já instalado."
fi

# Atualiza Homebrew
echo "🔄 Atualizando Homebrew..."
brew update

# Instala esptool
if ! command -v esptool.py &> /dev/null
then
    echo "📦 Instalando esptool..."
    brew install esptool
else
    echo "✅ esptool já instalado."
fi

# Instala mkspiffs
if ! command -v mkspiffs &> /dev/null
then
    echo "📦 Instalando mkspiffs..."
    brew install mkspiffs
else
    echo "✅ mkspiffs já instalado."
fi

# Verificações finais
echo
echo "✅ Ferramentas instaladas:"
echo " - esptool.py: $(which esptool.py)"
echo " - mkspiffs:   $(which mkspiffs)"
echo
echo "🚀 Ambiente pronto para gerar e enviar SPIFFS ao ESP32."
