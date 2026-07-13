<?php
/**
 * Psyerns_Framework
 * Copyright (c) 2026 Psyern / Deadmans Echo
 * SPDX-License-Identifier: AGPL-3.0-only
 */
require_once __DIR__ . '/config.php';

$action = $_GET['action'] ?? '';

switch ($action) {
    case 'check':
        pf_validate_key();
        $steam_id = $_GET['steam_id'] ?? '';
        $wl = pf_read_json('whitelist.json', []);
        $found = false;
        foreach ($wl as $entry) {
            if ($entry['steam_id'] === $steam_id) { $found = true; break; }
        }
        echo json_encode(['whitelisted' => $found]);
        break;

    case 'add':
        pf_validate_key();
        if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
            http_response_code(405);
            echo json_encode(['error' => 'POST required']);
            exit;
        }
        $body = json_decode(file_get_contents('php://input'), true);
        $steam_id = $body['steamId'] ?? '';
        $name = $body['name'] ?? '';
        if (empty($steam_id)) {
            http_response_code(400);
            echo json_encode(['error' => 'Missing steamId']);
            exit;
        }
        $wl = pf_read_json('whitelist.json', []);
        foreach ($wl as $e) {
            if ($e['steam_id'] === $steam_id) {
                echo json_encode(['success' => true]);
                exit;
            }
        }
        $wl[] = ['steam_id' => $steam_id, 'name' => $name, 'added_at' => gmdate('Y-m-d H:i:s')];
        pf_write_json('whitelist.json', $wl);
        echo json_encode(['success' => true]);
        break;

    case 'remove':
        pf_validate_key();
        if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
            http_response_code(405);
            echo json_encode(['error' => 'POST required']);
            exit;
        }
        $body = json_decode(file_get_contents('php://input'), true);
        $steam_id = $body['steamId'] ?? '';
        $wl = pf_read_json('whitelist.json', []);
        $wl = array_values(array_filter($wl, function($e) use ($steam_id) {
            return $e['steam_id'] !== $steam_id;
        }));
        pf_write_json('whitelist.json', $wl);
        echo json_encode(['success' => true]);
        break;

    case 'list':
        pf_validate_key();
        echo json_encode(pf_read_json('whitelist.json', []));
        break;

    default:
        http_response_code(400);
        echo json_encode(['error' => 'Missing or invalid action parameter']);
        break;
}
