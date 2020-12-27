#include "gigatron.h"

// Получение XT-кода
int Gigatron::get_key(SDL_Event event, int keydown) {

    /* Получение ссылки на структуру с данными о нажатой клавише */
    SDL_KeyboardEvent * eventkey = & event.key;

    int xt = 0;
    int k = eventkey->keysym.scancode;

    // printf("%d ", k);
    switch (k) {

        /* A */ case 0x26: xt = press_shift ? ~'A' : ~'a'; break;
        /* B */ case 0x38: xt = press_shift ? ~'B' : ~'b'; break;
        /* C */ case 0x36: xt = press_shift ? ~'C' : ~'c'; break;
        /* D */ case 0x28: xt = press_shift ? ~'D' : ~'d'; break;
        /* E */ case 0x1a: xt = press_shift ? ~'E' : ~'e'; break;
        /* F */ case 0x29: xt = press_shift ? ~'F' : ~'f'; break;
        /* G */ case 0x2a: xt = press_shift ? ~'G' : ~'g'; break;
        /* H */ case 0x2b: xt = press_shift ? ~'H' : ~'h'; break;
        /* I */ case 0x1f: xt = press_shift ? ~'I' : ~'i'; break;
        /* J */ case 0x2c: xt = press_shift ? ~'J' : ~'j'; break;
        /* K */ case 0x2d: xt = press_shift ? ~'K' : ~'k'; break;
        /* L */ case 0x2e: xt = press_shift ? ~'L' : ~'l'; break;
        /* M */ case 0x3a: xt = press_shift ? ~'M' : ~'m'; break;
        /* N */ case 0x39: xt = press_shift ? ~'N' : ~'n'; break;
        /* O */ case 0x20: xt = press_shift ? ~'O' : ~'o'; break;
        /* P */ case 0x21: xt = press_shift ? ~'P' : ~'p'; break;
        /* Q */ case 0x18: xt = press_shift ? ~'Q' : ~'q'; break;
        /* R */ case 0x1b: xt = press_shift ? ~'R' : ~'r'; break;
        /* S */ case 0x27: xt = press_shift ? ~'S' : ~'s'; break;
        /* T */ case 0x1c: xt = press_shift ? ~'T' : ~'t'; break;
        /* U */ case 0x1e: xt = press_shift ? ~'U' : ~'u'; break;
        /* V */ case 0x37: xt = press_shift ? ~'V' : ~'v'; break;
        /* W */ case 0x19: xt = press_shift ? ~'W' : ~'w'; break;
        /* X */ case 0x35: xt = press_shift ? ~'X' : ~'x'; break;
        /* Y */ case 0x1d: xt = press_shift ? ~'Y' : ~'y'; break;
        /* Z */ case 0x34: xt = press_shift ? ~'Z' : ~'z'; break;

        /* 0 */ case 0x13: xt = press_shift ? ~')' : ~'0'; break;
        /* 1 */ case 0x0A: xt = press_shift ? ~'!' : ~'1'; break;
        /* 2 */ case 0x0B: xt = press_shift ? ~'@' : ~'2'; break;
        /* 3 */ case 0x0C: xt = press_shift ? ~'#' : ~'3'; break;
        /* 4 */ case 0x0D: xt = press_shift ? ~'$' : ~'4'; break;
        /* 5 */ case 0x0E: xt = press_shift ? ~'%' : ~'5'; break;
        /* 6 */ case 0x0F: xt = press_shift ? ~'^' : ~'6'; break;
        /* 7 */ case 0x10: xt = press_shift ? ~'&' : ~'7'; break;
        /* 8 */ case 0x11: xt = press_shift ? ~'*' : ~'8'; break;
        /* 9 */ case 0x12: xt = press_shift ? ~'(' : ~'9'; break;

        // Символы
        /* ~ */ case 0x31: xt = ~'~'; break;
        /* - */ case 0x14: xt = press_shift ? ~'_' : ~'-'; break;
        /* = */ case 0x15: xt = press_shift ? ~'+' : ~'='; break;
        /* \ */ case 0x33: xt = press_shift ? ~'|' : ~'\\'; break;
        /* [ */ case 0x22: xt = press_shift ? ~'{' : ~'['; break;
        /* ] */ case 0x23: xt = press_shift ? ~'}' : ~']'; break;
        /* ; */ case 0x2f: xt = press_shift ? ~':' : ~';'; break;
        /* ' */ case 0x30: xt = press_shift ? ~'"' : ~'\''; break;
        /* , */ case 0x3b: xt = press_shift ? ~'<' : ~','; break;
        /* . */ case 0x3c: xt = press_shift ? ~'>' : ~'.'; break;
        /* / */ case 0x3d: xt = press_shift ? ~'?' : ~'/'; break;

        // Специальные клавиши
        /* bs */ case 0x16: xt = ~0x7F; break; // Back Space
        /* sp */ case 0x41: xt = ~' ';  break; // Space
        /* tb */ case 0x17: xt = ~9;    break; // Tab
        /* ls */ case 0x32: press_shift = keydown; break; // Left Shift
        /* lc */ case 0x25: press_ctrl  = keydown; break; // Left Ctrl
        /* la */ case 0x40: press_alt   = keydown; break; // Left Alt
        /* en */ case 0x24: xt = ~10;   break; // Enter
        /* es */ case 0x09: xt = ~27;   break; // Escape

        /* F1  */ case 67: xt = 0x3B; break;
        /* F2  */ case 68: xt = 0x3C; break;
        /* F3  */ case 69: xt = 0x3D; break;
        /* F4  */ case 70: xt = 0x3E; break;
        /* F5  */ case 71: xt = 0x3F; break;
        /* F6  */ case 72: xt = 0x40; break;
        /* F7  */ case 73: xt = 0x41; break;
        /* F8  */ case 74: xt = 0x42; break;
        /* F9  */ case 75: xt = 0x43; break;
        /* F10 */ case 76: xt = 0x44; break;
        /* F11 */ case 95: xt = 0x57; break; // Не проверено
        /* F12 */ case 96: xt = 0x58; break;

        // ---------------------------------------------
        // Специальные (не так же как в реальном железе)
        // ---------------------------------------------

        /* UP  */  case 0x6F: xt = BUTTON_UP; break;
        /* RT  */  case 0x72: xt = BUTTON_RIGHT; break;
        /* DN  */  case 0x74: xt = BUTTON_DOWN; break;
        /* LF  */  case 0x71: xt = BUTTON_LEFT; break;
        /* Home */ case 0x6E: xt = BUTTON_B; break;
        /* End  */ case 0x73: xt = BUTTON_START; break;
        /* PgUp */ case 0x70: xt = 0; break;
        /* PgDn */ case 0x75: xt = 0; break;
        /* Del  */ case 0x77: xt = BUTTON_SELECT; break;
        /* Ins  */ case 0x76: xt = BUTTON_A; break;
        /* NLock*/ case 0x4D: xt = 0; break;

        default: return 0;
    }

    /* Получить скан-код клавиш */
    return xt;
}

void Gigatron::gamepad_press(SDL_Event event) {

    int key = get_key(event, 1);

    // Выход к отладчику
    if (key == ~27) {
        require_stop = 1;
    } else {
        inReg &= ~key;
    }
}

void Gigatron::gamepad_up(SDL_Event event) {

    get_key(event, 0); 

    inReg = 0xFF;
}
