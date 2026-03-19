<?php

require_once __DIR__ . '/config.php';

// PHP auth fallback that reads/writes the CSV database directly.
$body   = input();
$action = $body['action'] ?? '';

switch ($action) {

    case 'login': {
        $username = trim($body['username'] ?? '');
        $password = trim($body['password'] ?? '');

        if (!$username || !$password) {
            jsonErr('Username and password required.');
        }

        $users = loadUsers();
        $idx   = findUser($users, $username);

        if ($idx === -1 || $users[$idx]['password'] !== $password) {
            jsonErr('Invalid username or password.', 401);
        }

        $u = $users[$idx];
        jsonOk([
            'username'     => $u['username'],
            'wins'         => $u['wins'],
            'losses'       => $u['losses'],
            'games_played' => $u['games_played'],
            'joined_date'  => $u['joined_date'],
            'guest'        => false,
        ]);
    }

    case 'register': {
        $username = trim($body['username'] ?? '');
        $password = trim($body['password'] ?? '');

        if (strlen($username) < 2) jsonErr('Username must be at least 2 characters.');
        if (strlen($password) < 3) jsonErr('Password must be at least 3 characters.');
        if (!preg_match('/^[a-zA-Z0-9_]+$/', $username)) {
            jsonErr('Username may only contain letters, numbers, and underscores.');
        }

        $users = loadUsers();
        if (findUser($users, $username) !== -1) {
            jsonErr('Username already taken.', 409);
        }

        $users[] = [
            'username'     => $username,
            'password'     => $password,
            'wins'         => 0,
            'losses'       => 0,
            'games_played' => 0,
            'joined_date'  => date('Y-m-d'),
        ];
        saveUsers($users);

        jsonOk([
            'username'     => $username,
            'wins'         => 0,
            'losses'       => 0,
            'games_played' => 0,
            'joined_date'  => date('Y-m-d'),
            'guest'        => false,
        ]);
    }

    case 'guest': {
        $name = trim($body['name'] ?? 'Guest');
        if (empty($name)) $name = 'Guest';

        jsonOk([
            'username' => $name,
            'guest'    => true,
            'wins'     => 0,
            'losses'   => 0,
            'games_played' => 0,
        ]);
    }

    case 'stats': {
        $username = trim($body['username'] ?? '');
        $wins     = (int)($body['wins']    ?? 0);
        $losses   = (int)($body['losses']  ?? 0);
        $games    = (int)($body['games']   ?? 0);

        if (!$username) jsonErr('Username required.');

        $users = loadUsers();
        $idx   = findUser($users, $username);
        if ($idx === -1) jsonErr('User not found.', 404);

        $users[$idx]['wins']         = $wins;
        $users[$idx]['losses']       = $losses;
        $users[$idx]['games_played'] = $games;
        saveUsers($users);

        jsonOk(['message' => 'Stats updated.']);
    }

    default:
        jsonErr('Unknown action.', 400);
}
