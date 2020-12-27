#include "gigatron.h"

/**
 * horizontal:
 * {
 *      frontPorch: 16,
 *      pulse: 96,
 *      backPorch: 48,
 *      visible: 640,
 * },
 * vertical:
 * {
 *      frontPorch: 6,
 *      pulse: 8,
 *      backPorch: 27,
 *      visible: 480,
 * },
 **/

void Gigatron::vga_init() {

    row = 0;
    col = 0;

    minRow = 27 + 8;        // v.backPorch + v.pulse
    maxRow = minRow + 480;  // v.visible

    minCol = 48 + 96;       // h.backPorch + h.pulse
    maxCol = minCol + 640;  // h.visible

    vga_out = 0;
}

// Обработчик VGA эмулятора
void Gigatron::vga_tick() {

    uint8_t let_out = out;                  // cpu.out
    uint8_t falling = vga_out & ~let_out;   // vga.out

    // После 4 и более CPU циклов HSYNC увеличит row до 0
    if (falling & VSYNC) {
        row = -1;
    }

    if (falling & HSYNC) {
        col = 0;
        row++;
    }

    vga_out = out;

    if ((row >= minRow && row < maxRow) &&
        (col >= minCol && col < maxCol)) {

        uint8_t let_r = (let_out     ) & 3;
        uint8_t let_g = (let_out >> 2) & 3;
        uint8_t let_b = (let_out >> 4) & 3;

        for (int i = 0; i < 4; i++) {

            uint32_t color = (85*let_r)*65536 + (85*let_g)*256 + (85*let_b);

            // Сохранение для отладчика
            vga_buffer[row - minRow][col - minCol] = color;

            // Процессор запущен
            if (started) pset((col - minCol) + i, (row - minRow), color);
        }
    }

    col += 4;
}
