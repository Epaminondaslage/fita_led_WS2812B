# Projeto: Controle de Fita de LED WS2812B com ESP32 via Wi-Fi e arquivos em SPIFFS

Este projeto permite controlar uma fita de LED WS2812B com até 400 LEDs utilizando um ESP32 e uma interface web responsiva. Os efeitos podem ser selecionados através do navegador, e as configurações são salvas no próprio ESP32 (persistência).

## 🔧 Funcionalidades

- Interface web responsiva e moderna
- Controle de 15 efeitos visuais
- Ajuste de brilho e velocidade
- Destaque visual do botão ativo na interface
- Persistência de configurações (efeito e brilho)
- Separação de arquivos via SPIFFS (HTML, CSS, backend)

## 📁 Estrutura de Arquivos

```
esp32_led_spiffs/
├── main.ino             # Código principal do ESP32
└── data/
    ├── index.html       # Página web
    └── style.css        # Estilo visual da página
```

## 🌐 Interface Web

Acesse via navegador após o ESP32 conectar-se à rede Wi-Fi. A interface permite:

- Selecionar efeitos visuais
- Ajustar brilho e velocidade
- Ver botão do efeito atualmente ativo

## 💡 Efeitos Inclusos

1. Confete
2. Cometa
3. Piscar
4. Arco-Íris
5. Branco Frio
6. Branco Quente
7. Azul
8. Verde
9. Vermelho
10. Arco-Íris Rotativo
11. Progressivo por Setores
12. Desligar
13. Polícia/Emergência
14. Corrida de Teatro
15. Explosão Central

## ⚙️ Upload dos Arquivos SPIFFS

Para fazer o upload da pasta `/data` para o ESP32, siga este tutorial oficial:

👉 [Como instalar o ESP32 SPIFFS Uploader na IDE Arduino](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/)

Depois, vá em:

```
Arduino IDE > Ferramentas > ESP32 Sketch Data Upload
```

## ✅ Requisitos

- ESP32
- Fita de LED WS2812B (NeoPixel)
- Fonte de alimentação compatível (5V)
- Arduino IDE com suporte para ESP32
- Biblioteca:
  - Adafruit NeoPixel
  - Preferences

---

