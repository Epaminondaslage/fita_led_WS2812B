<?php
// Recebe os dados JSON enviados pelo script.js
$data = json_decode(file_get_contents('php://input'), true);

// Extrai os valores com segurança
$efeito = $data['efeito'] ?? '';
$brilho = $data['brilho'] ?? '';
$velocidade = $data['velocidade'] ?? '';
$fitas = $data['fitas'] ?? [];

// Verifica se veio pelo menos uma fita
if (empty($fitas)) {
    http_response_code(400);
    echo json_encode(["erro" => "Nenhuma fita selecionada."]);
    exit;
}

// Envia o comando para cada fita marcada
foreach ($fitas as $fita) {
    // Chama enviar.php com os parâmetros (assíncrono)
    exec("php enviar.php $fita $efeito $brilho $velocidade > /dev/null 2>&1 &");
}

// Retorna resposta para o JavaScript
echo json_encode([
    "status" => "ok",
    "fitas" => $fitas,
    "efeito" => $efeito,
    "brilho" => $brilho,
    "velocidade" => $velocidade
]);
