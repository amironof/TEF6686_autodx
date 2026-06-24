<?php

// Скрипт принимает JSON от esp32 и записывает данные сразу в таблицу imported_json. Этот скрипт вызывается из esp32, когда она отправляет JSON. 
// Раньше мы сохраняли JSON в файл, а потом другой скрипт его читал и импортировал в базу, но теперь сразу пишем в базу, без промежуточного файла.

date_default_timezone_set('Europe/Moscow');
require_once __DIR__ . '/../config.php'; 

$json = file_get_contents('php://input');

if (!$json) {
    http_response_code(400);
    echo "Empty body";
    exit;
}

$data = json_decode($json, true);

if (!$data) {
    http_response_code(400);
    echo "Invalid JSON";
    exit;
}

$datetime     = date('Y-m-d H:i:s');
$input_att    = $data['input_att'];
$feedback_att = $data['feedback_att'];

$measurements = [];
foreach ($data['data'] as $row) {
    $freq = number_format($row[0] / 100, 2);

    $rds_pi = isset($row[8]) ? (string)$row[8] : null;
    $rds_ps = isset($row[9]) ? (string)$row[9] : null;

    $measurements[$freq] = [
        'level'      => $row[1],
        'usn'        => $row[2],
        'wam'        => $row[3],
        'offset'     => $row[4],
        'bandwidth'  => $row[5],
        'modulation' => $row[6],
        'stereo'     => $row[7],
        'rds_pi'     => $rds_pi,
        'rds_ps'     => $rds_ps,
    ];
}

$mysqli = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
if ($mysqli->connect_error) {
    http_response_code(500);
    echo "DB connection error";
    exit;
}

$mysqli->query("TRUNCATE TABLE imported_json");

$result = $mysqli->query("SELECT id, frequency FROM freq_config");
if (!$result) {
    http_response_code(500);
    echo "DB query error";
    exit;
}

$stmt = $mysqli->prepare("INSERT INTO imported_json 
    (station_id, datetime, level, usn, wam, offset, bandwidth, modulation, stereo, agc_input_att, agc_feedback_att, rds_pi, rds_ps)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
if (!$stmt) {
    http_response_code(500);
    echo "DB prepare error";
    exit;
}

$inserted = 0;
$errors = [];

while ($station = $result->fetch_assoc()) {
    $freq = number_format($station['frequency'], 2);

    if (!isset($measurements[$freq])) {
        continue;
    }

    $m = $measurements[$freq];
    $station_id = $station['id'];

    $rds_pi = $m['rds_pi'] !== null ? substr($m['rds_pi'], 0, 4) : null;
    $rds_ps = $m['rds_ps'] !== null ? substr($m['rds_ps'], 0, 8) : null;

    $stmt->bind_param("isiiiiiiiiiss",
        $station_id, $datetime,
        $m['level'], $m['usn'], $m['wam'], $m['offset'], $m['bandwidth'], $m['modulation'], $m['stereo'],
        $input_att, $feedback_att,
        $rds_pi, $rds_ps
    );

    if (!$stmt->execute()) {
        $errors[] = $stmt->error;
    } else {
        $inserted++;
    }
}

$stmt->close();
$mysqli->close();

if (!empty($errors)) {
    http_response_code(500);
    echo "Errors: " . implode(", ", array_unique($errors));
    exit;
}

echo "OK. Inserted: {$inserted}";