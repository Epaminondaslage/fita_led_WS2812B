<?php
$ips = [
  "1" => "10.0.0.241",
  "2" => "10.0.0.242",
  "3" => "10.0.0.243"
];

$fita = $_GET['fita'] ?? '';
$efeito = $_GET['efeito'] ?? '';
$brilho = $_GET['brilho'] ?? '';
$vel = $_GET['vel'] ?? '';

if (!isset($ips[$fita])) exit;

$url = "http://{$ips[$fita]}/config?";
$params = [];

if ($efeito !== '') $params[] = "efeito={$efeito}";
if ($brilho !== '') $params[] = "brilho={$brilho}";
if ($vel !== '') $params[] = "vel={$vel}";

$url .= implode("&", $params);

// Envia a requisição
file_get_contents($url);
?>
