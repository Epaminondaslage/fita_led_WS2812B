
# X. Controle de Fitas de LED WS2812B via PHP e AJAX

Este projeto permite o controle simultâneo de até **3 fitas de LED WS2812B** conectadas a dispositivos ESP32 com IP fixo, por meio de uma interface web clara, responsiva e sensível ao toque.

## 📂 Estrutura de Diretórios

```
fita_led_php/
├── index.html              # Interface principal com os controles para as fitas
├── css/
│   └── style.css           # Estilos da interface (layout, botões, fontes, etc.)
├── js/
│   └── script.js           # Script JavaScript que envia comandos para o backend via AJAX
├── php/
│   ├── enviar.php          # Backend que recebe os parâmetros (fita, efeito, brilho, vel) e envia para o IP correto
│   └── controller.php      # Script auxiliar  para controle lógico mais amplo
```

---

## Descrição dos Arquivos

### `index.html`
Interface principal do usuário. Contém:
- Seleção de fitas (checkboxes) com valores `1`, `2` e `3`
- Botões para escolher efeitos de iluminação (valores `0` a `15`)
- Sliders para controle de brilho (`0-255`) e velocidade (`1-200`)
- Inclusão dos arquivos `style.css` e `script.js`
- Feedback visual no rodapé para erros de comunicação com os dispositivos

---

### `css/style.css`
Estilos aplicados à interface HTML:
- Layout centralizado e responsivo
- Estilo dos botões (cores, margens, bordas)
- Estilo dos sliders (input[type=range])
- Estilo do botão selecionado com classe `selected`

---

### `js/script.js`
Script JavaScript que:
- Captura os cliques dos botões de efeitos e valores dos sliders
- Verifica as fitas selecionadas
- Envia os comandos via `fetch()` para o script `php/enviar.php`
- Testa a conectividade de cada fita via `fetch()` assíncrono
- Exibe mensagem no rodapé caso uma fita esteja inacessível (timeout ou erro)
- Atualiza dinamicamente os parâmetros quando brilho ou velocidade são alterados

---

### `php/enviar.php`
Backend em PHP que:
- Recebe os parâmetros via `GET`: `fita`, `efeito`, `brilho`, `vel`
- Mapeia a fita selecionada para o respectivo IP:
  - Fita 1 → `10.0.2.240`
  - Fita 2 → `10.0.2.241`
  - Fita 3 → `10.0.2.242`
- Monta a URL no formato:
  ```
  http://<ip>/config?efeito=<efeito>&brilho=<brilho>&vel=<vel>
  ```
- Envia a requisição via `cURL` e retorna o código de resposta HTTP
- Exibe mensagens de log no terminal (caso usado via CLI)

---

### `php/controller.php` 
- Script auxiliar para lógica avançada 
- Pode conter:
  - Lógica de agendamento
  - Agrupamento de fitas
  - Logs de ações realizadas
  - Validações ou restrições de comando


---

## 🔧 Componentes do Sistema

- **Navegador Web (Cliente)**: Interface HTML/CSS/JS para seleção de fitas, efeitos, brilho e velocidade.
- **Servidor Web (Apache/Nginx)**: Executa scripts PHP localizados em `/var/www/html/fita_led_php/php`.
- **Script JavaScript (`script.js`)**: Captura interações do usuário e envia requisições HTTP (AJAX).
- **Script PHP (`enviar.php`)**: Recebe parâmetros via GET e envia comandos HTTP para dispositivos ESP.
- **Dispositivos ESP (10.0.2.240-242)**: Executam o efeito de iluminação via `http://<IP>/config`.

---

## ✅ Funcionalidades

- Seleção de uma ou mais fitas (Fita 1, Fita 2, Fita 3)
- 15 efeitos visuais com botão exclusivo para cada um
- Botão "Desligar Fita"
- Controle de brilho (0–255)
- Controle de velocidade (1–200, onde menor = mais lento)
- Aplicação simultânea de comandos em várias fitas
- Interface leve, responsiva e otimizada para telas pequenas (mobile-friendly)
- Feedback visual nos botões selecionados


## 🔁 Fluxo de Requisição

### 1. Seleção no Navegador

Usuário seleciona uma ou mais fitas e um botão de efeito. O JavaScript coleta:
- `efeito`: valor do botão clicado (0 a 15).
- `brilho`: valor do input de brilho (0 a 255).
- `vel`: valor do input de velocidade (1 a 200).
- `fitasSelecionadas`: valores 1, 2 e/ou 3 conforme checkbox ativado.

### 2. Envio via JavaScript

Para cada fita selecionada, é feita uma requisição:

```javascript
fetch(`php/enviar.php?fita=${fita}&efeito=${efeito}&brilho=${brilho}&vel=${vel}`);
```

---

## 🧠 Lógica do `enviar.php`

1. Recebe os parâmetros `fita`, `efeito`, `brilho`, `vel` via `$_GET`.
2. Mapeia o número da fita para o IP correspondente:
   - 1 → 10.0.2.240
   - 2 → 10.0.2.241
   - 3 → 10.0.2.242
3. Constrói a URL de comando:
   ```
   http://<IP>/config?efeito=X&brilho=Y&vel=Z
   ```
4. Utiliza `curl` para enviar a URL ao ESP correspondente.
5. Retorna resposta HTTP para debug.

---

## 🌐 Comunicação HTTP

Todos os comandos entre servidor e ESP usam protocolo HTTP via GET:

```
GET http://10.0.2.242/config?efeito=2&brilho=150&vel=50
```

---

## ⚠️ Teste de Acessibilidade (JavaScript)

Após marcar uma fita, é feito um teste com `fetch()` para verificar se o IP responde.

```javascript
fetch(`http://${ip}/config`, { method: 'HEAD' })
```

Se a resposta for falha (`catch()`), uma mensagem de erro é exibida no rodapé da página.

---

## 📌 Exemplo Completo

1. Usuário marca "Fita Direita", brilho 150, velocidade 50 e clica no efeito "Piscar" (efeito 2).
2. JS envia:  
   `php/enviar.php?fita=3&efeito=2&brilho=150&vel=50`
3. PHP constrói:  
   `http://10.0.2.242/config?efeito=2&brilho=150&vel=50`
4. Dispositivo ESP aplica o efeito.

---
