<?php

// Этот скрипт читает JSON, который был сохранён в папке api/monitoring_data/, и импортирует данные в базу для дальнейшей обработки. Запускается по крону.

ini_set('display_errors', 1);
error_reporting(E_ALL);

date_default_timezone_set('Europe/Moscow');
require_once 'config.php';

$file = __DIR__ . '/api/monitoring_data/latest.json';
$file_age = time() - filemtime($file);

echo "Время сервера: " . date('Y-m-d H:i:s') . "\n";
echo "Время файла: " . date('Y-m-d H:i:s', filemtime($file)) . "\n";
echo "Возраст файла: " . $file_age . " секунд\n";

if ($file_age > 600) {
    die("Файл устарел, пропускаем");
}

$json_raw = file_get_contents($file);
if (!$json_raw) {
    die("Не удалось прочитать файл JSON");
}

$json = json_decode($json_raw, true);
if (!$json) {
    die("Ошибка парсинга JSON");
}

$datetime = date('Y-m-d H:i:s', filemtime($file));
$input_att    = $json['input_att'];
$feedback_att = $json['feedback_att'];
$data         = $json['data'];

$measurements = [];
foreach ($data as $row) {
    $freq = number_format($row[0] / 100, 2);
    $measurements[$freq] = [
        'level'      => $row[1],
        'usn'        => $row[2],
        'wam'        => $row[3],
        'offset'     => $row[4],
        'bandwidth'  => $row[5],
        'modulation' => $row[6],
        'stereo'     => $row[7],
    ];
}

$mysqli = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
if ($mysqli->connect_error) {
    die("Ошибка подключения: " . $mysqli->connect_error);
}

$mysqli->query("TRUNCATE TABLE imported_json");

$result = $mysqli->query("SELECT id, frequency FROM freq_config");

$stmt = $mysqli->prepare("INSERT INTO imported_json 
    (station_id, datetime, level, usn, wam, offset, bandwidth, modulation, stereo, agc_input_att, agc_feedback_att)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

while ($station = $result->fetch_assoc()) {
    $station_id = $station['id'];
    $freq = number_format($station['frequency'], 2);

    if (!isset($measurements[$freq])) {
        continue;
    }

    $m = $measurements[$freq];

    $stmt->bind_param("isiiiiiiiii",
        $station_id, $datetime,
        $m['level'], $m['usn'], $m['wam'], $m['offset'], $m['bandwidth'], $m['modulation'], $m['stereo'],
        $input_att, $feedback_att
    );

    if (!$stmt->execute()) {
        echo "Ошибка: " . $stmt->error . "\n";
    }
}

$stmt->close();
$mysqli->close();
echo "OK";