document.querySelectorAll(".efeitos button").forEach(button => {
  button.addEventListener("click", () => {
    const efeito = button.getAttribute("data-efeito");
    const brilho = document.getElementById("brilho").value;
    const vel = document.getElementById("velocidade").value;

    const fitasSelecionadas = Array.from(document.querySelectorAll(".fita:checked"))
      .map(cb => cb.value);

    document.querySelectorAll(".efeitos button").forEach(b => b.classList.remove("selected"));
    button.classList.add("selected");

    fitasSelecionadas.forEach(fita => {
      fetch(`php/enviar.php?fita=${fita}&efeito=${efeito}&brilho=${brilho}&vel=${vel}`);
    });
  });
});

document.getElementById("brilho").addEventListener("change", e => {
  const brilho = e.target.value;
  const vel = document.getElementById("velocidade").value;
  const efeito = document.querySelector(".efeitos button.selected")?.getAttribute("data-efeito") ?? 0;
  const fitasSelecionadas = Array.from(document.querySelectorAll(".fita:checked"))
    .map(cb => cb.value);

  fitasSelecionadas.forEach(fita => {
    fetch(`php/enviar.php?fita=${fita}&efeito=${efeito}&brilho=${brilho}&vel=${vel}`);
  });
});

document.getElementById("velocidade").addEventListener("change", e => {
  const vel = e.target.value;
  const brilho = document.getElementById("brilho").value;
  const efeito = document.querySelector(".efeitos button.selected")?.getAttribute("data-efeito") ?? 0;
  const fitasSelecionadas = Array.from(document.querySelectorAll(".fita:checked"))
    .map(cb => cb.value);

  fitasSelecionadas.forEach(fita => {
    fetch(`php/enviar.php?fita=${fita}&efeito=${efeito}&brilho=${brilho}&vel=${vel}`);
  });
});
