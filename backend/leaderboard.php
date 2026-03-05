<?php

require_once __DIR__ . '/config.php';

$users = loadUsers();

usort($users, fn($a, $b) => $b['wins'] - $a['wins']);

$top = array_slice($users, 0, 50);

foreach ($top as &$u) unset($u['password']);

jsonOk(['leaderboard' => $top]);
