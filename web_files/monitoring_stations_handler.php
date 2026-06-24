<?php

// Этот скрипт читает данные из таблицы imported_json и переносит их в monitoring_data для станций, которые помечены как is_monitoring = 1. Запускается по крону.

date_default_timezone_set('Europe/Moscow');
require_once 'config.php';

$mysqli = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
if ($mysqli->connect_error) {
    die("Ошибка подключения: " . $mysqli->connect_error);
}

$result = $mysqli->query("
    SELECT i.station_id, i.datetime, i.level, i.usn, i.wam, i.offset, i.bandwidth, i.modulation
    FROM imported_json i
    JOIN freq_config f ON f.id = i.station_id
    WHERE f.is_monitoring = 1
");

$stmt = $mysqli->prepare("INSERT INTO monitoring_data 
    (station_id, datetime, level, usn, wam, offset, bandwidth, modulation)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

while ($row = $result->fetch_assoc()) {
    $stmt->bind_param("isiiiiii",
        $row['station_id'], $row['datetime'],
        $row['level'], $row['usn'], $row['wam'], $row['offset'], $row['bandwidth'], $row['modulation']
    );

    if (!$stmt->execute()) {
        echo "Ошибка: " . $stmt->error . "\n";
    }
}

$stmt->close();
$mysqli->close();
echo "OK";