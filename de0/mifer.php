<?php

$rows = [];
$file = file_get_contents(isset($argv[1]) ? $argv[1] : "gigatron.rom");

for ($cursor = 0; $cursor < strlen($file); $cursor += 2) {

    $ir = 256*ord($file[$cursor]) + ord($file[$cursor+1]); // Big Endian
    $rows[] = "  " . str_pad( dechex($cursor >> 1), 4, '0', STR_PAD_LEFT) . " :  " . str_pad( dechex($ir), 4, '0', STR_PAD_LEFT) . ";"; 
}

if ($cursor < 0x20000)
$rows[] = "  [" . str_pad( dechex(strlen($file) >> 1), 4, '0', STR_PAD_LEFT) . " .. ffff]: 00;";

ob_start();
?>
WIDTH=16;
DEPTH=65536;

ADDRESS_RADIX=HEX;
DATA_RADIX=HEX;
CONTENT BEGIN
<? echo join("\n", $rows) . "\n"; ?>
END;
<?
$data = ob_get_clean();
if (isset($argv[1])) echo $data; else file_put_contents("rom.mif", $data);
?>
