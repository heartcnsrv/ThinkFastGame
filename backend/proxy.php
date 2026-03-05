<?php
// ============================================================
//  ThinkFast  |  backend/proxy.php
//
//  This file does ONE thing: forward the browser's request
//  to the C++ backend server running on localhost:8080,
//  then return the response.
//
//  NO game logic here. The C++ server handles everything:
//    /auth        → AuthManager  (login/register/guest/stats)
//    /validate    → WordValidator (real dictionary check)
//    /leaderboard → AuthManager::leaderboard()
//    /room        → RoomManager  (all multiplayer game rules)
// ============================================================

define('CPP_SERVER', 'http://127.0.0.1:8080');

header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');
header('Content-Type: application/json');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

// Which C++ endpoint to call — from the URL path segment after /api/
// e.g.  /api/auth        → POST http://127.0.0.1:8080/auth
//        /api/room        → POST http://127.0.0.1:8080/room
//        /api/leaderboard → POST http://127.0.0.1:8080/leaderboard
//        /api/validate    → POST http://127.0.0.1:8080/validate

$uri      = parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH);
$segments = explode('/', trim($uri, '/'));
// segments: ['api', 'auth'] or ['src','gui','...'] etc.
// The endpoint is the last non-empty segment.
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

// Read the raw JSON body the browser sent
$body = file_get_contents('php://input');

// Forward to C++ server
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
