
document.querySelectorAll(".efeitos button").forEach(button => {
  button.addEventListener("click", () => {
    const efeito = button.getAttribute("data-efeito");
    const fitasSelecionadas = Array.from(document.querySelectorAll(".fita:checked"))
      .map(cb => cb.value);

    document.querySelectorAll(".efeitos button").forEach(b => b.classList.remove("selected"));
    button.classList.add("selected");

    fitasSelecionadas.forEach(fita => {
      fetch(`php/enviar.php?fita=${fita}&efeito=${efeito}`);
    });
  });
});

document.getElementById("brilho").addEventListener("change", e => {
  const brilho = e.target.value;
  const fitasSelecionadas = Array.from(document.querySelectorAll(".fita:checked"))
    .map(cb => cb.value);

  fitasSelecionadas.forEach(fita => {
    fetch(`php/enviar.php?fita=${fita}&brilho=${brilho}`);
  });
});

document.getElementById("velocidade").addEventListener("change", e => {
  const vel = e.target.value;
  const fitasSelecionadas = Array.from(document.querySelectorAll(".fita:checked"))
    .map(cb => cb.value);

  fitasSelecionadas.forEach(fita => {
    fetch(`php/enviar.php?fita=${fita}&vel=${vel}`);
  });
});
