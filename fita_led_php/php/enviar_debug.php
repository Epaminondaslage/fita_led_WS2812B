<?php
$fita_ips = [
  "1" => "10.0.2.240",
  "2" => "10.0.2.241",
  "3" => "10.0.2.242",
];

$fita = $_GET['fita'] ?? '3';
$efeito = $_GET['efeito'] ?? '2';
$brilho = $_GET['brilho'] ?? '150';
$vel = $_GET['vel'] ?? '50';

if (!isset($fita_ips[$fita])) {
  echo "Fita inválida.";
  exit;
}

$url = "http://{$fita_ips[$fita]}/config?efeito=$efeito&brilho=$brilho&vel=$vel";
echo "Testando URL: $url<br>";

$ch = curl_init($url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_TIMEOUT, 5);

$response = curl_exec($ch);

if ($response === false) {
  echo "Erro cURL: " . curl_error($ch);
} else {
  echo "Resposta OK:<br><pre>" . htmlspecialchars($response) . "</pre>";
}

curl_close($ch);
?>
