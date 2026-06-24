<?php

date_default_timezone_set('Europe/Moscow');
require_once 'config.php';

$mysqli = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
if ($mysqli->connect_error) {
    die("Ошибка подключения: " . $mysqli->connect_error);
}

$mysqli->query("DELETE FROM freq_history WHERE datetime < NOW() - INTERVAL 30 DAY");

$datetime = date('Y-m-d H:i:s');

$result = $mysqli->query("SELECT station_id, level, bandwidth, agc_input_att FROM imported_json");

$stmt = $mysqli->prepare("INSERT INTO freq_history 
    (station_id, datetime, level, bandwidth, agc_input_att)
    VALUES (?, ?, ?, ?, ?)");

while ($row = $result->fetch_assoc()) {
    $stmt->bind_param("isiii",
        $row['station_id'], $datetime,
        $row['level'], $row['bandwidth'], $row['agc_input_att']
    );

    if (!$stmt->execute()) {
        echo "Ошибка: " . $stmt->error . "\n";
    }
}

$stmt->close();
$mysqli->close();
echo "OK";