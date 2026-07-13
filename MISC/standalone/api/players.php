<?php
/**
 * Psyerns_Framework
 * Copyright (c) 2026 Psyern / Deadmans Echo
 * SPDX-License-Identifier: AGPL-3.0-only
 */
require_once __DIR__ . '/config.php';
pf_validate_key();

$action = $_GET['action'] ?? '';
$data = pf_read_json('leaderboard.json', []);
$all_players = array_merge($data['topPVEPlayers'] ?? [], $data['topPVPPlayers'] ?? []);

switch ($action) {
    case 'lookup':
        $steam_id = $_GET['steam_id'] ?? '';
        $found = null;
        foreach ($all_players as $p) {
            if (($p['playerID'] ?? $p['odolozId'] ?? '') === $steam_id) { $found = $p; break; }
        }
        if (!$found) {
            http_response_code(404);
            echo json_encode(['error' => 'Player not found']);
            exit;
        }
        echo json_encode([
            'steamId' => $found['playerID'] ?? $found['odolozId'] ?? '',
            'name' => $found['playerName'] ?? '',
            'online' => (bool)($found['isOnline'] ?? false),
            'posX' => 0.0, 'posY' => 0.0, 'posZ' => 0.0, 'health' => 0.0,
        ]);
        break;

    case 'online':
        $online = [];
        $seen = [];
        foreach ($all_players as $p) {
            $sid = $p['playerID'] ?? $p['odolozId'] ?? '';
            if (($p['isOnline'] ?? 0) && !isset($seen[$sid])) {
                $seen[$sid] = true;
                $online[] = [
                    'steamId' => $sid,                    'name' => $p['playerName'] ?? '',
                    'online' => true,
                    'posX' => 0.0, 'posY' => 0.0, 'posZ' => 0.0, 'health' => 0.0,
                ];
            }
        }
        echo json_encode($online);
        break;

    default:
        http_response_code(400);
        echo json_encode(['error' => 'Missing or invalid action']);
        break;
}
