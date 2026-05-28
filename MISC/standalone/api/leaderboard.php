<?php
require_once __DIR__ . '/config.php';

$type = $_GET['type'] ?? 'pve';
$limit = intval($_GET['limit'] ?? 20);
$limit = min(max($limit, 1), 100);

$data = pf_read_json('leaderboard.json', []);
if (empty($data)) {
    echo json_encode(['generatedAt' => '', 'playerOnlineCounter' => 0, 'totalPlayers' => 0, 'players' => []]);
    exit;
}

$key = $type === 'pvp' ? 'topPVPPlayers' : 'topPVEPlayers';
$raw = $data[$key] ?? [];
$raw = array_slice($raw, 0, $limit);

$players = [];
foreach ($raw as $p) {
    $steam_id = $p['playerID'] ?? $p['odolozId'] ?? '';
    $players[] = [
        'steam_id' => $steam_id,
        'player_name' => $p['playerName'] ?? '',
        'pve_points' => intval($p['pvePoints'] ?? 0),
        'pvp_points' => intval($p['pvpPoints'] ?? 0),
        'kills' => intval($p['kills'] ?? 0),
        'deaths' => intval($p['deaths'] ?? 0),
        'ai_kills' => intval($p['aiKills'] ?? 0),
        'pve_deaths' => intval($p['pveDeaths'] ?? 0),
        'pvp_deaths' => intval($p['pvpDeaths'] ?? 0),
        'longest_shot' => floatval($p['longestShot'] ?? 0),
        'playtime' => floatval($p['playtime'] ?? 0),
        'is_online' => intval($p['isOnline'] ?? 0),
        'last_login' => $p['lastLoginDate'] ?? '',
        'category_kills' => $p['categoryKills'] ?? new stdClass(),
        'category_deaths' => $p['categoryDeaths'] ?? new stdClass(),
        'category_longest_ranges' => $p['categoryLongestRanges'] ?? new stdClass(),
        'avatar_url' => pf_get_avatar_url($steam_id),
    ];
}

echo json_encode([
    'generatedAt' => $data['generatedAt'] ?? '',
    'playerOnlineCounter' => intval($data['playerOnlineCounter'] ?? 0),
    'totalPlayers' => intval($data['totalPlayers'] ?? 0),
    'players' => $players,
]);
