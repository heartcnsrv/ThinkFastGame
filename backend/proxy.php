<?php

define('CPP_SERVER', 'http://127.0.0.1:8080');

// Apache/PHP-facing bridge into the custom C++ backend.
// The frontend can call PHP endpoints, and PHP forwards the raw JSON payload
// to the matching C++ route so core logic stays centralized there.
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');
header('Content-Type: application/json');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

$uri      = parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH);
$segments = explode('/', trim($uri, '/'));
$endpoint = '';
foreach (array_reverse($segments) as $seg) {
    $seg = trim($seg);
    if ($seg !== '' && $seg !== 'api') { $endpoint = $seg; break; }
}

$allowed = ['auth', 'validate', 'leaderboard', 'room'];
if (!in_array($endpoint, $allowed, true)) {
    http_response_code(400);
    echo json_encode(['ok' => false, 'error' => 'Unknown endpoint: ' . $endpoint]);
    exit;
}

$body = file_get_contents('php://input');

$ch = curl_init(CPP_SERVER . '/' . $endpoint);
curl_setopt_array($ch, [
    CURLOPT_POST           => true,
    CURLOPT_POSTFIELDS     => $body,
    CURLOPT_RETURNTRANSFER => true,
    CURLOPT_TIMEOUT        => 10,
    CURLOPT_HTTPHEADER     => ['Content-Type: application/json'],
]);

$response = curl_exec($ch);
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
$error    = curl_error($ch);
curl_close($ch);

if ($response === false || $error) {
    http_response_code(502);
    echo json_encode([
        'ok'    => false,
        'error' => 'C++ backend unreachable. Is ThinkFastServer running? (' . $error . ')'
    ]);
    exit;
}

http_response_code($httpCode ?: 200);
echo $response;
