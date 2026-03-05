<?php
// ============================================================
//  ThinkFast  |  backend/config.php
//  Configuration constants for the multiplayer backend
// ============================================================

define('DATA_DIR',   __DIR__ . '/../data/');
define('USERS_CSV',  DATA_DIR . 'users.csv');
define('ROOMS_DIR',  DATA_DIR . 'rooms/');

// Make sure rooms directory exists
if (!is_dir(ROOMS_DIR)) {
    mkdir(ROOMS_DIR, 0755, true);
}

// CORS headers (allow requests from the GUI frontend)
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Always respond as JSON
header('Content-Type: application/json');

// Handle pre-flight
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

// ── Helpers ───────────────────────────────────────────────────

function jsonOk(array $data): void {
    echo json_encode(['ok' => true] + $data);
    exit;
}

function jsonErr(string $message, int $code = 400): void {
    http_response_code($code);
    echo json_encode(['ok' => false, 'error' => $message]);
    exit;
}

function input(): array {
    $raw = file_get_contents('php://input');
    $data = json_decode($raw, true);
    return is_array($data) ? $data : [];
}

// ── CSV helpers ───────────────────────────────────────────────

function loadUsers(): array {
    $users = [];
    if (!file_exists(USERS_CSV)) return $users;
    $fh = fopen(USERS_CSV, 'r');
    $header = fgetcsv($fh); // skip header
    while (($row = fgetcsv($fh)) !== false) {
        if (count($row) < 6) continue;
        $users[] = [
            'username'     => $row[0],
            'password'     => $row[1],
            'wins'         => (int)$row[2],
            'losses'       => (int)$row[3],
            'games_played' => (int)$row[4],
            'joined_date'  => $row[5],
        ];
    }
    fclose($fh);
    return $users;
}

function saveUsers(array $users): void {
    $fh = fopen(USERS_CSV, 'w');
    fputcsv($fh, ['username','password','wins','losses','games_played','joined_date']);
    foreach ($users as $u) {
        fputcsv($fh, [
            $u['username'], $u['password'],
            $u['wins'], $u['losses'], $u['games_played'], $u['joined_date'],
        ]);
    }
    fclose($fh);
}

function findUser(array $users, string $username): int {
    foreach ($users as $i => $u) {
        if ($u['username'] === $username) return $i;
    }
    return -1;
}

// ── Room helpers ──────────────────────────────────────────────

function roomPath(string $code): string {
    return ROOMS_DIR . preg_replace('/[^A-Z0-9]/', '', strtoupper($code)) . '.json';
}

function loadRoom(string $code): ?array {
    $path = roomPath($code);
    if (!file_exists($path)) return null;
    $data = json_decode(file_get_contents($path), true);
    return is_array($data) ? $data : null;
}

function saveRoom(string $code, array $room): void {
    file_put_contents(roomPath($code), json_encode($room, JSON_PRETTY_PRINT));
}

function generateRoomCode(): string {
    $chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
    $code  = '';
    for ($i = 0; $i < 6; $i++) {
        $code .= $chars[random_int(0, strlen($chars) - 1)];
    }
    return $code;
}
