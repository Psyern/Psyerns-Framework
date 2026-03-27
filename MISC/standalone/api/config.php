<?php
define('PF_API_KEY', 'CHANGE_ME_TO_A_SECRET_KEY');
define('PF_STEAM_API_KEY', '');
define('PF_DATA_DIR', __DIR__ . '/../data/');
define('PF_CACHE_DIR', __DIR__ . '/../cache/');
define('PF_DEFAULT_THEME', 'dark');

header('Access-Control-Allow-Origin: *');
header('Content-Type: application/json; charset=utf-8');

function pf_validate_key() {
    $key = $_GET['api_key'] ?? $_POST['api_key'] ?? '';
    if ($key !== PF_API_KEY) {
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
    if (empty($steam_id)) return '';
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
