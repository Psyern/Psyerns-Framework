<?php
/**
 * Psyerns_Framework
 * Copyright (c) 2026 Psyern / Deadmans Echo
 * SPDX-License-Identifier: AGPL-3.0-only
 */
define('PF_API_KEY', 'CHANGE_ME_TO_A_SECRET_KEY');
define('PF_STEAM_API_KEY', '');
define('PF_DATA_DIR', __DIR__ . '/../data/');
define('PF_CACHE_DIR', __DIR__ . '/../cache/');
define('PF_DEFAULT_THEME', 'dark');

header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json; charset=utf-8');

function pf_extract_api_key() {
    // Prefer the X-API-Key request header so the secret does not end up in
    // access/proxy logs; fall back to the legacy query/body param for the
    // DayZ-server uploader that cannot set arbitrary headers.
    if (!empty($_SERVER['HTTP_X_API_KEY'])) {
        return trim($_SERVER['HTTP_X_API_KEY']);
    }
    return $_GET['api_key'] ?? $_POST['api_key'] ?? '';
}

function pf_validate_key() {
    // F3 deploy guard: refuse (and loudly log) if the key was never changed
    // from its placeholder, otherwise the write endpoints are wide open.
    if (PF_API_KEY === 'CHANGE_ME_TO_A_SECRET_KEY' || PF_API_KEY === '') {
        error_log('[Psyerns_Framework] Standalone API refused a request: PF_API_KEY is still the placeholder or empty. Set a secret key in api/config.php before deploying.');
        http_response_code(503);
        echo json_encode(['error' => 'Service not configured: set PF_API_KEY in config.php']);
        exit;
    }
    $key = pf_extract_api_key();
    if (!hash_equals(PF_API_KEY, $key)) {
        http_response_code(401);
        echo json_encode(['error' => 'Unauthorized']);
        exit;
    }
}

function pf_read_json($filename, $default = []) {
    $path = PF_DATA_DIR . basename($filename);
    if (!file_exists($path)) return $default;
    $json = file_get_contents($path);
    return json_decode($json, true) ?: $default;
}

function pf_write_json($filename, $data) {
    if (!is_dir(PF_DATA_DIR)) mkdir(PF_DATA_DIR, 0755, true);
    $path = PF_DATA_DIR . basename($filename);
    $fp = fopen($path, 'w');
    if ($fp && flock($fp, LOCK_EX)) {
        fwrite($fp, json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE));
        flock($fp, LOCK_UN);
        fclose($fp);
        return true;
    }
    if ($fp) fclose($fp);
    return false;
}

function pf_get_avatar_url($steam_id) {
    // F1: only accept a canonical 17-digit SteamID64. This blocks path
    // traversal into the cache dir AND unsafe interpolation into the Steam URL.
    if (!preg_match('/^\d{17}$/', $steam_id)) return '';
    if (empty(PF_STEAM_API_KEY)) return '';

    if (!is_dir(PF_CACHE_DIR)) mkdir(PF_CACHE_DIR, 0755, true);
    $cache_file = PF_CACHE_DIR . $steam_id . '.json';

    if (file_exists($cache_file) && (time() - filemtime($cache_file)) < 86400) {
        $cached = json_decode(file_get_contents($cache_file), true);
        if (isset($cached['avatar_url'])) return $cached['avatar_url'];
    }

    $url = 'https://api.steampowered.com/ISteamUser/GetPlayerSummaries/v0002/?key=' . PF_STEAM_API_KEY . '&steamids=' . $steam_id;
    $ctx = stream_context_create(['http' => ['timeout' => 5]]);
    $resp = @file_get_contents($url, false, $ctx);
    if (!$resp) return '';

    $data = json_decode($resp, true);
    $players = $data['response']['players'] ?? [];
    $avatar = !empty($players) ? ($players[0]['avatarfull'] ?? '') : '';

    file_put_contents($cache_file, json_encode(['avatar_url' => $avatar]));
    return $avatar;
}
