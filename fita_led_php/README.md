
# Controle de Fitas de LED WS2812B via PHP e AJAX

Este projeto permite o controle simultâneo de até **3 fitas de LED WS2812B** conectadas a dispositivos ESP32 com IP fixo, por meio de uma interface web clara, responsiva e sensível ao toque.

## 📂 Estrutura de Diretórios

```
fita_led_php/
├── index.html            ← Página principal do painel de controle
├── css/
│   └── style.css         ← Estilos visuais para a interface
├── js/
│   └── script.js         ← Scripts JavaScript para AJAX e interação
├── php/
│   └── enviar.php        ← Backend responsável por enviar comandos aos ESPs
```

## ✅ Funcionalidades

- ✅ Seleção de uma ou mais fitas (Fita 1, Fita 2, Fita 3)
- ✅ 15 efeitos visuais com botão exclusivo para cada um
- ✅ Botão "Desligar Fita"
- ✅ Controle de brilho (0–255)
- ✅ Controle de velocidade (1–200, onde menor = mais lento)
- ✅ Aplicação simultânea de comandos em várias fitas
- ✅ Interface leve, responsiva e otimizada para telas pequenas (mobile-friendly)
- ✅ Feedback visual nos botões selecionados

## ⚙️ Requisitos

- Servidor com PHP (Apache, Nginx ou localhost via XAMPP/Laragon)
- ESP32 com firmware compatível com requisições HTTP
- As fitas devem estar associadas a IPs fixos nos ESPs

## 🧠 Como Funciona

- Os botões HTML ativam funções JavaScript que:
  1. Capturam os parâmetros escolhidos (efeito, brilho, velocidade)
  2. Identificam quais fitas estão ativas
  3. Enviam requisições AJAX ao `enviar.php` para aplicar os comandos
- O `enviar.php` itera sobre os IPs selecionados e envia comandos HTTP do tipo `GET` diretamente ao ESP32, no formato esperado por ele:
  ```
  http://<ip_da_fita>/config?efeito=3&brilho=150&vel=120
  ```
