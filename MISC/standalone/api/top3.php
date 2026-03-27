<?php
require_once __DIR__ . '/config.php';

$type = $_GET['type'] ?? 'monthly';
$data = pf_read_json('leaderboard.json', []);

if (empty($data)) {
    echo json_encode([]);
    exit;
}

$all = array_merge($data['topPVEPlayers'] ?? [], $data['topPVPPlayers'] ?? []);

// Deduplicate by steam ID, keep highest combined score
$byId = [];
foreach ($all as $p) {
    $sid = $p['playerID'] ?? $p['odolozId'] ?? '';
    if (empty($sid)) continue;
    if (!isset($byId[$sid])) {
        $byId[$sid] = $p;
    } else {
        // Merge: keep higher values
        $existing = $byId[$sid];
        $byId[$sid]['kills'] = max($existing['kills'] ?? 0, $p['kills'] ?? 0);
        $byId[$sid]['deaths'] = max($existing['deaths'] ?? 0, $p['deaths'] ?? 0);
        $byId[$sid]['pvePoints'] = max($existing['pvePoints'] ?? 0, $p['pvePoints'] ?? 0);
        $byId[$sid]['pvpPoints'] = max($existing['pvpPoints'] ?? 0, $p['pvpPoints'] ?? 0);
    }
}

$players = array_values($byId);

if ($type === 'deadliest') {
    usort($players, function($a, $b) {
        return ($b['kills'] ?? 0) - ($a['kills'] ?? 0);
    });
} else {
    // monthly: sort by combined points
    usort($players, function($a, $b) {
        $aScore = ($a['pvePoints'] ?? 0) + ($a['pvpPoints'] ?? 0);
        $bScore = ($b['pvePoints'] ?? 0) + ($b['pvpPoints'] ?? 0);
        return $bScore - $aScore;
    });
}

$top3 = array_slice($players, 0, 3);

$result = [];
foreach ($top3 as $p) {
    $sid = $p['playerID'] ?? $p['odolozId'] ?? '';
    $result[] = [
        'steam_id'                => $sid,
        'player_name'             => $p['playerName'] ?? '',
        'pve_points'              => intval($p['pvePoints'] ?? 0),
        'pvp_points'              => intval($p['pvpPoints'] ?? 0),
        'kills'                   => intval($p['kills'] ?? 0),
        'deaths'                  => intval($p['deaths'] ?? 0),
        'ai_kills'                => intval($p['aiKills'] ?? 0),
        'longest_shot'            => floatval($p['longestShot'] ?? 0),
        'playtime'                => floatval($p['playtime'] ?? 0),
        'is_online'               => intval($p['isOnline'] ?? 0),
        'category_kills'          => $p['categoryKills'] ?? new stdClass(),
        'category_longest_ranges' => $p['categoryLongestRanges'] ?? new stdClass(),
        'avatar_url'              => pf_get_avatar_url($sid),
    ];
}

echo json_encode($result);
