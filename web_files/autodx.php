<?php

/*
Скрипт детекции дальних станций.
Логика такая: берём медианные данные за последние 15 минут и сравниваем их с историческими медианными данными для этой же станции и такого же уровня аттенюатора.
Если медианный уровень сигнала за последние 15 минут выше исторической медианной нормы на 6 дБ, а медианная ширина полосы выше нормы на 20 кГц, то считаем, что это может быть проходом.
Проходы сохраняются в отдельную таблицу dxinfo_save, а также отправляются в телеграм. Если для станции уже есть активный проход (мы его уже отправили в телеграм),
то просто обновляем время последнего обнаружения, чтобы не спамить телеграм повторными сообщениями.
*/

date_default_timezone_set('Europe/Moscow');
require_once 'config.php';
require_once 'tg_send.php';

function median(array $arr): float {
    if (empty($arr)) return 0;
    sort($arr);
    $count = count($arr);
    $mid = (int)($count / 2);
    if ($count % 2 === 0) {
        return ($arr[$mid - 1] + $arr[$mid]) / 2;
    }
    return $arr[$mid];
}

$mysqli = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
if ($mysqli->connect_error) {
    die("Ошибка подключения: " . $mysqli->connect_error);
}

$mysqli->query("DELETE FROM dx_alerts WHERE last_seen < NOW() - INTERVAL 30 MINUTE");

// Получаем все точки за 15 минут
$raw = $mysqli->query("
    SELECT r.station_id, f.frequency, r.level, r.bandwidth, r.agc_input_att, r.rds_pi, r.rds_ps
    FROM freq_recent r
    JOIN freq_config f ON f.id = r.station_id
    WHERE r.datetime > NOW() - INTERVAL 15 MINUTE
    AND r.agc_input_att = (SELECT agc_input_att FROM imported_json LIMIT 1)
    ORDER BY r.station_id, f.frequency + 0
");

// Группируем по станции
$stations = [];
while ($row = $raw->fetch_assoc()) {
    $sid = $row['station_id'];
    if (!isset($stations[$sid])) {
        $stations[$sid] = [
            'frequency' => $row['frequency'],
            'levels'    => [],
            'bws'       => [],
            'agc'       => $row['agc_input_att'],
            'rds_pi'    => null,
            'rds_ps'    => null,
        ];
    }
    $stations[$sid]['levels'][] = $row['level'];
    $stations[$sid]['bws'][]    = $row['bandwidth'];

    // Берём последнее непустое значение
    if (!empty($row['rds_pi'])) $stations[$sid]['rds_pi'] = $row['rds_pi'];
    if (!empty($row['rds_ps'])) $stations[$sid]['rds_ps'] = $row['rds_ps'];
}

// Получаем исторические нормы
$hist_result = $mysqli->query("
    SELECT station_id, level, bandwidth, agc_input_att
    FROM freq_history
");

$hist_raw = [];
while ($row = $hist_result->fetch_assoc()) {
    $sid = $row['station_id'];
    $agc = $row['agc_input_att'];
    $hist_raw[$sid][$agc]['levels'][] = $row['level'];
    $hist_raw[$sid][$agc]['bws'][]    = $row['bandwidth'];
}

$history = [];
foreach ($hist_raw as $sid => $agc_data) {
    foreach ($agc_data as $agc => $data) {
        $history[$sid][$agc] = [
            'norm_level' => median($data['levels']),
            'norm_bw'    => median($data['bws']),
        ];
    }
}

// Считаем медианы один раз
$medians = [];
foreach ($stations as $sid => $data) {
    $medians[$sid] = [
        'median_level' => median($data['levels']),
        'median_bw'    => median($data['bws']),
    ];
}

// Детектируем проходы
$detected = [];
foreach ($stations as $sid => $data) {
    $agc = $data['agc'];
    if (!isset($history[$sid][$agc])) continue;

    $median_level = $medians[$sid]['median_level'];
    $median_bw    = $medians[$sid]['median_bw'];
    $norm_level   = $history[$sid][$agc]['norm_level'];
    $norm_bw      = $history[$sid][$agc]['norm_bw'];

    if ($median_level > $norm_level + 6 && $median_bw > $norm_bw + 20) {
        $detected[] = [
            'station_id'   => $sid,
            'frequency'    => $data['frequency'],
            'median_level' => $median_level,
            'median_bw'    => $median_bw,
            'norm_level'   => $norm_level,
            'norm_bw'      => $norm_bw,
            'rds_pi'       => $data['rds_pi'],
            'rds_ps'       => $data['rds_ps'],
        ];
    }
}

echo "<h2>Обнаруженные частоты</h2>";

if (empty($detected)) {
    echo "<p>Проход не обнаружен</p>";
} else {
    $stmt_save = $mysqli->prepare("INSERT INTO dxinfo_save (station_id, datetime, level, bandwidth, rds_pi, rds_ps) VALUES (?, NOW(), ?, ?, ?, ?)");

    echo "<table border='1'>";
    echo "<tr><th>Частота</th><th>Level (медиана)</th><th>Level (норма)</th><th>BW (медиана)</th><th>BW (норма)</th><th>RDS PI</th><th>RDS PS</th></tr>";

    foreach ($detected as $row) {
        $station_id  = $row['station_id'];
        $freq        = $row['frequency'];
        $level       = round($row['median_level'], 1);
        $bw          = round($row['median_bw'], 1);
        $rds_pi      = $row['rds_pi'] ?? '—';
        $rds_ps      = $row['rds_ps'] ?? '—';
        $rds_pi_save = $row['rds_pi'];
        $rds_ps_save = $row['rds_ps'];

        $check = $mysqli->query("SELECT station_id FROM dx_alerts WHERE station_id = $station_id");

        if ($check->num_rows == 0) {
            tg_send("🔴 Обнаружен проход на $freq МГц!\nLevel: $level (норма: " . round($row['norm_level'], 1) . ")\nBW: $bw (норма: " . round($row['norm_bw'], 1) . ")\nRDS PI: $rds_pi\nRDS PS: $rds_ps");
            $mysqli->query("INSERT INTO dx_alerts (station_id, started_at, last_seen) VALUES ($station_id, NOW(), NOW())");
        } else {
            $mysqli->query("UPDATE dx_alerts SET last_seen = NOW() WHERE station_id = $station_id");
        }

        $stmt_save->bind_param("iddss", $station_id, $level, $bw, $rds_pi_save, $rds_ps_save);
        $stmt_save->execute();

        echo "<tr>";
        echo "<td>" . $freq . " МГц</td>";
        echo "<td>" . $level . "</td>";
        echo "<td>" . round($row['norm_level'], 1) . "</td>";
        echo "<td>" . $bw . "</td>";
        echo "<td>" . round($row['norm_bw'], 1) . "</td>";
        echo "<td>" . htmlspecialchars($rds_pi) . "</td>";
        echo "<td>" . htmlspecialchars($rds_ps) . "</td>";
        echo "</tr>";
    }

    $stmt_save->close();
    echo "</table>";
}

// Таблица всех значений
echo "<h2>Медианные значения по всем частотам</h2>";
echo "<table border='1'>";
echo "<tr><th>Частота</th><th>Level (норма)</th><th>Level (сейчас)</th><th>BW (норма)</th><th>BW (сейчас)</th></tr>";

uasort($stations, fn($a, $b) => $a['frequency'] + 0 <=> $b['frequency'] + 0);

foreach ($stations as $sid => $data) {
    $agc = $data['agc'];
    if (!isset($history[$sid][$agc])) continue;
        echo "<tr>";
        echo "<td>" . $data['frequency'] . " МГц</td>";
        echo "<td>" . round($history[$sid][$agc]['norm_level'], 1) . "</td>";
        echo "<td>" . round($medians[$sid]['median_level'], 1) . "</td>";
        echo "<td>" . round($history[$sid][$agc]['norm_bw'], 1) . "</td>";
        echo "<td>" . round($medians[$sid]['median_bw'], 1) . "</td>";
        echo "</tr>";
}

echo "</table>";

$mysqli->close();