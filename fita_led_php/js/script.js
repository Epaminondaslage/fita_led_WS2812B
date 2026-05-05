/**
 * ============================================================
 * fita_led_php / js/script.js
 * Logica de controle das fitas de LED
 * ============================================================
 * Funcionalidades:
 *  - Envio de comandos via PHP proxy (enviar.php)
 *  - Verificacao de acessibilidade do dispositivo
 *  - Feedback visual via toast e highlight de efeito ativo
 *  - Atualizacao de sliders em tempo real
 *
 * Autor  : Epaminondas Lage
 * Versao : 2.0.0
 * Data   : 2025-05
 * ============================================================
 */

// ── Toast de feedback ──────────────────────────────────────
function mostrarToast(msg, erro) {
    var toast = document.getElementById('toast');
    toast.textContent = msg;
    toast.style.background = erro ? '#dc2626' : '#1a1d23';
    toast.classList.add('visivel');
    setTimeout(function() { toast.classList.remove('visivel'); }, 3000);
}

// ── Verifica se o IP esta acessivel ───────────────────────
async function ipAcessivel(ip) {
    try {
        var controller = new AbortController();
        var timeoutId  = setTimeout(function() { controller.abort(); }, 1500);
        await fetch('http://' + ip + '/config', {
            method: 'HEAD', mode: 'no-cors', signal: controller.signal
        });
        clearTimeout(timeoutId);
        return true;
    } catch (e) {
        return false;
    }
}

// ── Mapa de fitas ──────────────────────────────────────────
var MAPA_FITAS = { '1': '10.0.2.240', '2': '10.0.2.241', '3': '10.0.2.242' };

// ── Envia comando para uma fita ───────────────────────────
async function enviarComando(fita, efeito, brilho, vel) {
    var ip        = MAPA_FITAS[fita];
    var acessivel = await ipAcessivel(ip);

    if (!acessivel) {
        mostrarToast('Fita ' + fita + ' (' + ip + ') inacessível', true);
        return;
    }

    var url = 'php/enviar.php?fita=' + fita + '&efeito=' + efeito + '&brilho=' + brilho + '&vel=' + vel;
    fetch(url);
}

// ── Sliders — atualiza valor em tempo real ─────────────────
document.getElementById('brilho').addEventListener('input', function() {
    document.getElementById('val-brilho').textContent = this.value;
});

document.getElementById('velocidade').addEventListener('input', function() {
    document.getElementById('val-velocidade').textContent = this.value;
});

// ── Sliders — envia ao soltar ──────────────────────────────
document.getElementById('brilho').addEventListener('change', async function() {
    var brilho = this.value;
    var vel    = document.getElementById('velocidade').value;
    var btn    = document.querySelector('.btn-efeito.ativo');
    var efeito = btn ? btn.getAttribute('data-efeito') : '0';
    var fitas  = Array.from(document.querySelectorAll('.fita:checked')).map(function(cb) { return cb.value; });

    if (!fitas.length) { mostrarToast('Selecione ao menos uma fita', true); return; }
    for (var i = 0; i < fitas.length; i++) {
        await enviarComando(fitas[i], efeito, brilho, vel);
    }
});

document.getElementById('velocidade').addEventListener('change', async function() {
    var vel    = this.value;
    var brilho = document.getElementById('brilho').value;
    var btn    = document.querySelector('.btn-efeito.ativo');
    var efeito = btn ? btn.getAttribute('data-efeito') : '0';
    var fitas  = Array.from(document.querySelectorAll('.fita:checked')).map(function(cb) { return cb.value; });

    if (!fitas.length) { mostrarToast('Selecione ao menos uma fita', true); return; }
    for (var i = 0; i < fitas.length; i++) {
        await enviarComando(fitas[i], efeito, brilho, vel);
    }
});

// ── Botoes de efeito ───────────────────────────────────────
document.querySelectorAll('.btn-efeito').forEach(function(btn) {
    btn.addEventListener('click', async function() {
        var efeito = this.getAttribute('data-efeito');
        var brilho = document.getElementById('brilho').value;
        var vel    = document.getElementById('velocidade').value;
        var fitas  = Array.from(document.querySelectorAll('.fita:checked')).map(function(cb) { return cb.value; });

        if (!fitas.length) { mostrarToast('Selecione ao menos uma fita', true); return; }

        // Marca o efeito ativo
        document.querySelectorAll('.btn-efeito').forEach(function(b) { b.classList.remove('ativo'); });
        this.classList.add('ativo');

        // Atualiza hint
        document.getElementById('efeito-ativo').textContent = this.textContent.trim();

        // Envia para cada fita
        for (var i = 0; i < fitas.length; i++) {
            await enviarComando(fitas[i], efeito, brilho, vel);
        }

        mostrarToast('Efeito enviado para ' + fitas.length + ' fita(s)');
    });
});
