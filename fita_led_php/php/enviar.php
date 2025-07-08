<?php
$mapa = [
  "1" => "10.0.2.240",
  "2" => "10.0.2.241",
  "3" => "10.0.2.242"
];

$fita = $_GET['fita'] ?? null;
$efeito = $_GET['efeito'] ?? null;
$brilho = $_GET['brilho'] ?? null;
$vel = $_GET['vel'] ?? null;

if (!$fita || !isset($mapa[$fita])) {
  http_response_code(400);
  exit("Fita inválida ou não informada.");
}
if ($efeito === null || $brilho === null || $vel === null) {
  http_response_code(400);
  exit("Parâmetros incompletos.");
}

$ip = $mapa[$fita];
$url = "http://$ip/config?efeito=$efeito&brilho=$brilho&vel=$vel";

$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$resposta = curl_exec($ch);
$http_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
curl_close($ch);

echo "Comando enviado para fita $fita ($ip)\n";
echo "URL: $url\n";
echo "Resposta HTTP: $http_code\n";
?>
