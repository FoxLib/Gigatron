<?php

$file = $argv[1];
$rows = array_map('trim', file($file));

$t1   = ['ac' => 0, 'x'   => 4, 'y'  => 5,  'out' => 6];

$org    = 0;
$labels = [];
$hex    = '';
$bin    = '';
$equ    = [];

function address($a) {

    global $labels, $equ;

    $a = trim($a);

    // Трансляция адреса
    if (isset($equ[$a])) $a = $equ[$a];

    if (isset($labels[$a])) {
        return $labels[$a] & 255;
    }

    if (preg_match('~\$([0-9a-f]+)~i', $a)) {
        return hexdec($a) & 255;
    }

    return -1;
}

// Поиск меток
foreach ($rows as $i => $row) {

    // Удаление комментария
    $row = trim(preg_replace('~;.*$~', '', $row));
    $rows[$i] = $row;

    // Эквивалентная метка
    if (preg_match('~(.+)=(.+)~', $row, $c)) {

        $equ[trim($c[1])] = trim($c[2]);
        unset($rows[$i]);
    }
    // На линии обнаружена метка
    else if (preg_match('~^([a-z0-9\._]+):~i', $row, $c)) $labels[$c[1]] = $org;

    // На линии обнаружена инструкция
    if (preg_match('~\b(st|ld|and|or|xor|add|sub|jmp|bra|bne|beq|bgt|blt|bge|ble)\b~i', $row)) $org++;
}

$org = 0;

foreach ($rows as $line => $row) {

    $lnum = $line + 1;

    // Удалить метку вначале
    $row = preg_replace('~^\s*[a-z0-9\._]+:~i', '', $row);
    $row = trim($row);

    if ($row == '') continue;

    $ir   = -1;
    $mode = -1;
    $bus  = -1;
    $data =  0;

    // АЛУ
    if (preg_match('~\b(ld|and|or|xor|add|sub)\b\s+(.+)$~i', $row, $c)) {

        $op   = strtolower($c[2]); // 0..5
        $ir   = ['ld' => 0, 'and' => 1, 'or' => 2,  'xor' => 3,'add' => 4,'sub' => 5][$c[1]];

        // bus=1 Используется шина данных, здесь операнды фиксированы
             if (preg_match('~ac,\s*\[x\]~', $op))              { $mode = 1; $bus = 1; }
        else if (preg_match('~ac,\s*\[y,\s*x\]~', $op))         { $mode = 3; $bus = 1; }
        else if (preg_match('~out,\s*\[y,\s*x\+\+]~', $op))     { $mode = 7; $bus = 1; }
        else if (preg_match('~x,\s*\[(.+)]~', $op, $m))         { $mode = 4; $bus = 1; $data = address($m[1]); }
        else if (preg_match('~y,\s*\[(.+)]~', $op, $m))         { $mode = 5; $bus = 1; $data = address($m[1]); }
        else if (preg_match('~out,\s*\[(.+)]~', $op, $m))       { $mode = 6; $bus = 1; $data = address($m[1]); }
        else if (preg_match('~ac,\s*\[y,\s*(.+)\]~', $op, $m))  { $mode = 2; $bus = 1; $data = address($m[1]); }
        else if (preg_match('~ac,\s*\[(.+)\]~', $op, $m))       { $mode = 0; $bus = 1; $data = address($m[1]); }
        // bus=0,2,3 Источник D,AC,IN
        else if (preg_match('~(ac|x|y|out),\s*ac~', $op, $m))   { $mode = $t1[$m[1]]; $bus = 2; }
        else if (preg_match('~(ac|x|y|out),\s*in~', $op, $m))   { $mode = $t1[$m[1]]; $bus = 3; }
        else if (preg_match('~(ac|x|y|out),\s*(.+)~', $op, $m)) { $mode = $t1[$m[1]]; $bus = 0; $data = address($m[2]); }

        if ($data < 0) { echo "Label not found ($lnum) `$row`\n"; exit(1); }
    }
    // STORE
    else if (preg_match('~\bst\b\s+(.+)$~', $row, $c)) {

        $op = $c[1];
        $ir = 6;

        // @todo CTRL

        // Поиск режима работы
        if      (preg_match('~,\s*\[x]~i', $op))             { $mode = 1; }
        else if (preg_match('~,\s*\[y,\s*x]~i', $op))        { $mode = 3; }
        else if (preg_match('~,\s*\[y,\s*x\+\+]~i', $op))    { $mode = 7; }
        else if (preg_match('~,\s*\[y,(.+)]~i', $op, $m))    { $mode = 2; $data = address($m[1]); }
        else if (preg_match('~,\s*\[(.+)],\s*x~i', $op, $m)) { $mode = 4; $data = address($m[1]); }
        else if (preg_match('~,\s*\[(.+)],\s*y~i', $op, $m)) { $mode = 5; $data = address($m[1]); }
        else if (preg_match('~,\s*\[(.+)]~i', $op, $m))      { $mode = 0; $data = address($m[1]); }

        // BUS
        if      (preg_match('~ac\s*,~i', $op)) $bus = 2;
        else if (preg_match('~in\s*,~i', $op)) $bus = 3;
        else if (preg_match('~(.+),~i', $op, $m)) { $bus = 0; $data = address($m[1]); }
    }
    // Переходы
    else if (preg_match('~(jmp|bgt|blt|bne|beq|bge|ble|bra)\s*(.+)$~i', $row, $c)) {

        $ir   = 7;
        $data = 0;
        $mode = [
            "jmp"=>0, "bgt"=>1,
            "blt"=>2, "bne"=>3,
            "beq"=>4, "bge"=>5,
            "ble"=>6, "bra"=>7
        ][ strtolower($c[1]) ];

        // bus=1
        if (preg_match('~\[(.+)\]~i', $c[2], $m)) {
            $data = address($m[1]);
            $bus = 1;
        }
        // bus=2,3
        else if (preg_match('~ac~i', $c[2])) $bus = 2;
        else if (preg_match('~in~i', $c[2])) $bus = 3;
        // bus=0
        else {

            $regex = '~\by\s*,~i';

            // JMP должен быть только с Y,<data>
            if ($mode == 0 && !preg_match($regex, $c[2])) {
                $mode = -1;
            }

            $data = address(preg_replace($regex, '', $c[2]));
            $bus  = $data >= 0 ? 0 : -1;
        }

        if ($data < 0) { echo "Label not found ($lnum) `$row`\n"; exit(1); }
    }

    if ($mode < 0 || $bus < 0) {
        echo "Error in line: $lnum `$row`\n";
        exit(1);
    }

    $opcode = ($ir<<13) + ($mode << 10) + ($bus << 8) + ($data & 255);
    $hex .= sprintf("%04x\n", $opcode);
    $bin .= chr($opcode>>8) . chr($opcode & 255);

    $org++;
}

// Сохранение в .hex-формате
file_put_contents(preg_replace('~\.asm~i' , '.bin', $file), $bin);
file_put_contents(preg_replace('~\.asm~i' , '.hex', $file), $hex);

