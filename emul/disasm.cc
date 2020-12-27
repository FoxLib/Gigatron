#include "gigatron.h"
#include <string.h>
#include "ansi16.h"

// Дизассемблировать строку
char* Gigatron::disasm(uint16_t address) {

    strcpy(disasm_row,    "");
    strcpy(disasm_opcode, "");
    strcpy(disasm_op1,    "");
    strcpy(disasm_op2,    "");

    uint16_t ir   = rom[address];
    uint8_t  op   = (ir >> 13) & 0x0007;
    uint8_t  mode = (ir >> 10) & 0x0007;
    uint8_t  bus  = (ir >> 8) & 0x0003;
    uint8_t  d    = (ir >> 0) & 0x00ff;

    // Мнемоника
    if (op < 7) strcpy(disasm_opcode, s_opcodes[op]);
    else        strcpy(disasm_opcode, s_branches[mode]);

    // ALU
    if (op < 6) {

        // Операнд dst
        switch (mode) {

            case 0:
            case 1:
            case 2:
            case 3: sprintf(disasm_op1, "AC"); break;
            case 4: sprintf(disasm_op1, "X"); break;
            case 5: sprintf(disasm_op1, "Y"); break;
            case 6:
            case 7: sprintf(disasm_op1, "OUT"); break;
        }

        // Операнд src
        switch (bus) {

            case 0: sprintf(disasm_op2, "$%02X", d); break;
            case 1: switch (mode) {

                case 0:
                case 4:
                case 5:
                case 6: sprintf(disasm_op2, "[$%02X]", d); break;
                case 1: sprintf(disasm_op2, "[X]"); break;
                case 2: sprintf(disasm_op2, "[Y,$%02X]", d); break;
                case 3: sprintf(disasm_op2, "[Y,X]"); break;
                case 7: sprintf(disasm_op2, "[Y,X++]"); break;
            }
            break;

            case 2: sprintf(disasm_op2, "AC"); break;
            case 3: sprintf(disasm_op2, "IN"); break;
        }

        sprintf(disasm_row, "%s  %s, %s", disasm_opcode, disasm_op1, disasm_op2);
    }
    // STORE
    else if (op == 6) {

        if (bus == 1) sprintf(disasm_op1, "CTRL");
        else switch (mode) {

            case 0:
            case 6: sprintf(disasm_op1, "[$%02X]", d); break;
            case 1: sprintf(disasm_op1, "[X]"); break;
            case 2: sprintf(disasm_op1, "[Y,$%02X]", d); break;
            case 3: sprintf(disasm_op1, "[Y,X]"); break;
            case 4: sprintf(disasm_op1, "[$%02X],X", d); break;
            case 5: sprintf(disasm_op1, "[$%02X],Y", d); break;
            case 7: sprintf(disasm_op1, "[Y,X++]"); break;
        }

        // Операнд src
        switch (bus) {

            case 0:
            case 1: sprintf(disasm_op2, "$%02X", d); break;
            case 2: sprintf(disasm_op2, "AC"); break;
            case 3: sprintf(disasm_op2, "IN"); break;
        }

        sprintf(disasm_row, "%s  %s, %s", disasm_opcode, disasm_op2, disasm_op1);
    }
    // BRANCH
    else if (op == 7) {

        switch (bus) {

            case 0: sprintf(disasm_op1, "$%02X", d); break;
            case 1: sprintf(disasm_op1, "[$%02X]", d); break;
            case 2: sprintf(disasm_op1, "AC"); break;
            case 3: sprintf(disasm_op1, "IN"); break;

        }
        sprintf(disasm_row, "%s  %s%s", disasm_opcode, mode == 0 ? "Y," : "", disasm_op1);

    }

    return disasm_row;
}

// Печать на экране Char
void Gigatron::print_char_16(int col, int row, unsigned char ch, uint cl) {

    col *= 8;
    row *= 16;

    for (int i = 0; i < 16; i++) {

        unsigned char hl = ansi16[ch][i];
        for (int j = 0; j < 8; j++) {
            if (hl & (1<<(7-j)))
                pset(j + col, i + row, cl);
        }
    }
}

// Печать строки
void Gigatron::print(int col, int row, const char* s, uint cl) {

    int i = 0;
    while (s[i]) {

        print_char_16(col, row, s[i], cl);
        col++;
        i++;
    }
}

// Выдать листинг гигатрона
void Gigatron::list() {

    unsigned int i, j, found_pc = 0;

    int  dstart = disasm_start;
    char buf[50];

    // Очистка экрана
    for (int y = 0; y < 480; y++)
    for (int x = 0; x < 640; x++)
        pset(x, y, 0);

    // Выдать строки
    for (i = 0; i < 30; i++) {

        int   digit = 0;
        char* dz    = disasm(dstart);

        // Подсветка текущей линии
        for (int x = 0; x < 250; x++)
        for (int y = 0; y < 16; y++)
            pset(x, y + 16*i, disasm_cursor == dstart ? 0xC0 : 0x303030);

        // Адрес
        sprintf(buf, "%04X\xB3    \xB3", dstart);
        print(0, i, (const char*)buf, 0xa0a0a0);

        // Код
        sprintf(buf, "%04X", rom[dstart]);
        print(5, i, (const char*)buf, 0xffffc0);

        // Указывает текущее положение PC
        if (pc == dstart) { print_char_16(10, i, 0x10, 0xffffff); found_pc = 1; }

        // Пропечатать цветной вывод
        for (j = 0; j < strlen(dz); j++) {

            uint32_t color = 0xcccccc;
            char     ch = dz[j];

            // Мнемоника
            if (j < 4) {
                color = 0x00ff00;
            }
            // Есть цифра
            else if (ch == '$') {
                color = 0xff88ff;
                digit = 1;
            }
            // Обнаружение цифры
            else if (digit) {

                if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
                    color = 0xff88ff;
                } else {
                    digit = 0;
                    color = 0xcccccc;
                }
            }

            print_char_16(11 + j, i, dz[j], color);
        }

        dstart++;
    }

    // Рекурсия 1 шаг
    if (found_pc == 0) {

        disasm_cursor = pc;
        disasm_start  = pc;
        list();
    }

    sprintf(buf, "AC: %02X   X:   %02X   Y:    %02X", ac, x, y); print(33, 1, buf, 0xc0c0c0);
    sprintf(buf, "IN: %02X   OUT: %02X   OUTX: %02X  CTRL: %02X", inReg, out, outx, ctrl); print(33, 2, buf, 0xc0c0c0);
    sprintf(buf, "PC: %04X NEXT %04X", pc, nextpc); print(33, 3, buf, 0xc0c0c0);

    // VGA
    sprintf(buf, "COL %d", col);  print(33, 4, buf, 0xc0c0c0);
    sprintf(buf, "ROW  %d", row); print(42, 4, buf, 0xc0c0c0);

    // Отрисовка буфера VGA
    for (int y = 0; y < 480; y += 2)
    for (int x = 0; x < 640; x += 2) {

        // x &~3 срезает нижние 2 бита, т.к. 1 точка = 4 пикселя
        uint32_t color = vga_buffer[y][x&~3] ^ (x == 0 || y == 0 || y == 480-2 || x == 640-2 ? 0x808080 : 0);
        pset((x>>1) + 285, (y>>1) + 200, color);
    }
}

// Нажата клавиша в режиме ожидания
void Gigatron::debugger_press(SDL_Event event) {

    int key = get_key(event, 1);

    // F7: Выполнить один такт
    if (key == 65) {

        all_tick();
        disasm_cursor = pc;
        list();
    }
    // ESC: Старт процессора
    else if (key == ~27) {
        run();
    }
}
