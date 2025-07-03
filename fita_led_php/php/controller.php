<?php
$ips = [
  1 => '10.0.2.240',
  2 => '10.0.2.241',
  3 => '10.0.2.242'
];

$fita = $_GET['fita'] ?? 1;
$efeito = $_GET['efeito'] ?? null;
$brilho = $_GET['brilho'] ?? null;
$vel = $_GET['vel'] ?? null;

$ip = $ips[$fita] ?? null;
if (!$ip) exit;

$params = [];
if ($efeito !== null) $params[] = "efeito=$efeito";
if ($brilho !== null) $params[] = "brilho=$brilho";
if ($vel !== null) $params[] = "vel=$vel";

$url = "http://$ip/config?" . implode('&', $params);
file_get_contents($url);
?>
