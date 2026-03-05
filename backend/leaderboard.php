<?php
// ============================================================
//  ThinkFast  |  backend/leaderboard.php
//  Returns top players sorted by wins
// ============================================================

require_once __DIR__ . '/config.php';

$users = loadUsers();

// Sort by wins descending
usort($users, fn($a, $b) => $b['wins'] - $a['wins']);

// Return top 50
$top = array_slice($users, 0, 50);

// Strip passwords
foreach ($top as &$u) unset($u['password']);

jsonOk(['leaderboard' => $top]);
