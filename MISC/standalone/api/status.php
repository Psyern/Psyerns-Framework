<?php
require_once __DIR__ . '/config.php';

$data = pf_read_json('server-status.json', null);
if ($data === null) {
    http_response_code(404);
    echo json_encode(['error' => 'No status data available']);
    exit;
}

echo json_encode($data);
