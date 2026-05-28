<?php
require_once __DIR__ . '/config.php';

$steam_id = $_GET['steam_id'] ?? '';
if (empty($steam_id)) {
    echo json_encode(['avatar_url' => '']);
    exit;
}

echo json_encode(['avatar_url' => pf_get_avatar_url($steam_id)]);
