function exibirErro(msg) {
  let rodape = document.getElementById("mensagem-erro");
  if (!rodape) {
    rodape = document.createElement("div");
    rodape.id = "mensagem-erro";
    rodape.style.position = "fixed";
    rodape.style.bottom = "0";
    rodape.style.left = "0";
    rodape.style.width = "100%";
    rodape.style.backgroundColor = "#ff5555";
    rodape.style.color = "#fff";
    rodape.style.textAlign = "center";
    rodape.style.padding = "10px";
    rodape.style.zIndex = "1000";
    document.body.appendChild(rodape);
  }
  rodape.textContent = msg;
  setTimeout(() => {
    if (rodape) rodape.remove();
  }, 5000);
}

async function ipAcessivel(ip) {
  try {
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), 1000); // timeout em 1s

    const response = await fetch(`http://${ip}/config`, {
      method: "HEAD",
      mode: "no-cors",
      signal: controller.signal
    });

    clearTimeout(timeoutId);
    return true;
  } catch (err) {
    return false;
  }
}

function obterIP(fita) {
  const mapa = {
    "1": "10.0.2.240",
    "2": "10.0.2.241",
    "3": "10.0.2.242"
  };
  return mapa[fita];
}

async function enviarComando(fita, efeito, brilho, vel) {
  const ip = obterIP(fita);
  const acessivel = await ipAcessivel(ip);

  if (!acessivel) {
    exibirErro(`Fita ${fita} (IP ${ip}) está inacessível`);
    return;
  }

  const url = `php/enviar.php?fita=${fita}&efeito=${efeito}&brilho=${brilho}&vel=${vel}`;
  fetch(url);
}

// Evento clique nos efeitos
document.querySelectorAll(".efeitos button").forEach(button => {
  button.addEventListener("click", async () => {
    const efeito = button.getAttribute("data-efeito");
    const brilho = document.getElementById("brilho").value;
    const vel = document.getElementById("velocidade").value;

    document.querySelectorAll(".efeitos button").forEach(b => b.classList.remove("selected"));
    button.classList.add("selected");

    const fitasSelecionadas = Array.from(document.querySelectorAll(".fita:checked")).map(cb => cb.value);
    for (const fita of fitasSelecionadas) {
      await enviarComando(fita, efeito, brilho, vel);
    }
  });
});

// Evento mudança de brilho
document.getElementById("brilho").addEventListener("change", async e => {
  const brilho = e.target.value;
  const vel = document.getElementById("velocidade").value;
  const efeito = document.querySelector(".efeitos button.selected")?.getAttribute("data-efeito") ?? 0;

  const fitasSelecionadas = Array.from(document.querySelectorAll(".fita:checked")).map(cb => cb.value);
  for (const fita of fitasSelecionadas) {
    await enviarComando(fita, efeito, brilho, vel);
  }
});

// Evento mudança de velocidade
document.getElementById("velocidade").addEventListener("change", async e => {
  const vel = e.target.value;
  const brilho = document.getElementById("brilho").value;
  const efeito = document.querySelector(".efeitos button.selected")?.getAttribute("data-efeito") ?? 0;

  const fitasSelecionadas = Array.from(document.querySelectorAll(".fita:checked")).map(cb => cb.value);
  for (const fita of fitasSelecionadas) {
    await enviarComando(fita, efeito, brilho, vel);
  }
});
