<?php
/**
 * Psyerns_Framework
 * Copyright (c) 2026 Psyern / Deadmans Echo
 * SPDX-License-Identifier: AGPL-3.0-only
 */
require_once __DIR__ . '/config.php';
pf_validate_key();

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['error' => 'Method not allowed']);
    exit;
}

$endpoint = $_GET['endpoint'] ?? '';
$body = json_decode(file_get_contents('php://input'), true);

if ($body === null && json_last_error() !== JSON_ERROR_NONE) {
    http_response_code(400);
    echo json_encode(['error' => 'Invalid JSON: ' . json_last_error_msg()]);
    exit;
}

switch ($endpoint) {
    case 'status':
        pf_write_json('server-status.json', $body);
        echo json_encode(['success' => true]);
        break;

    case 'upload':
        $store = [
            'generatedAt' => $body['generatedAt'] ?? '',
            'playerOnlineCounter' => intval($body['playerOnlineCounter'] ?? 0),
            'totalPlayers' => intval($body['totalPlayers'] ?? 0),
            'topPVEPlayers' => $body['topPVEPlayers'] ?? [],
            'topPVPPlayers' => $body['topPVPPlayers'] ?? [],
        ];
        pf_write_json('leaderboard.json', $store);
        echo json_encode(['success' => true]);
        break;

    case 'whitelist_add':
        $steam_id = $body['steamId'] ?? '';
        $name = $body['name'] ?? '';
        if (empty($steam_id)) {
            http_response_code(400);
            echo json_encode(['error' => 'Missing steamId']);
            exit;
        }
        $wl = pf_read_json('whitelist.json', []);
        foreach ($wl as $entry) {
            if ($entry['steam_id'] === $steam_id) {
                echo json_encode(['success' => true, 'message' => 'Already exists']);
                exit;
            }
        }
        $wl[] = ['steam_id' => $steam_id, 'name' => $name, 'added_at' => gmdate('Y-m-d H:i:s')];
        pf_write_json('whitelist.json', $wl);
        echo json_encode(['success' => true]);
        break;

    case 'whitelist_remove':
        $steam_id = $body['steamId'] ?? '';
        if (empty($steam_id)) {
            http_response_code(400);
            echo json_encode(['error' => 'Missing steamId']);
            exit;
        }
        $wl = pf_read_json('whitelist.json', []);
        $wl = array_values(array_filter($wl, function($e) use ($steam_id) {
            return $e['steam_id'] !== $steam_id;
        }));
        pf_write_json('whitelist.json', $wl);
        echo json_encode(['success' => true]);
        break;

    default:
        http_response_code(400);
        echo json_encode(['error' => 'Unknown endpoint: ' . $endpoint]);
        break;
}
