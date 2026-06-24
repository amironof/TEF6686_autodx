<?php

date_default_timezone_set('Europe/Moscow');
require_once 'config.php';

$mysqli = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
if ($mysqli->connect_error) {
    die("Ошибка подключения: " . $mysqli->connect_error);
}

$mysqli->query("DELETE FROM freq_recent WHERE datetime < NOW() - INTERVAL 72 HOUR");

$datetime = date('Y-m-d H:i:s');

$result = $mysqli->query("SELECT station_id, level, bandwidth, agc_input_att, rds_pi, rds_ps FROM imported_json");

$stmt = $mysqli->prepare("INSERT INTO freq_recent 
    (station_id, datetime, level, bandwidth, agc_input_att, rds_pi, rds_ps)
    VALUES (?, ?, ?, ?, ?, ?, ?)");

while ($row = $result->fetch_assoc()) {
    $rds_pi = $row['rds_pi'] ?? '';
    $rds_ps = $row['rds_ps'] ?? '';

    $stmt->bind_param("isiiiss",
        $row['station_id'], $datetime,
        $row['level'], $row['bandwidth'], $row['agc_input_att'],
        $rds_pi, $rds_ps
    );

    if (!$stmt->execute()) {
        echo "Ошибка: " . $stmt->error . "\n";
    }
}

$stmt->close();
$mysqli->close();
echo "OK";