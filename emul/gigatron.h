#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>

static const uint8_t ZERO = 0x80;
static const uint8_t VSYNC = 0x80;
static const uint8_t HSYNC = 0x40;
static const uint32_t SAMPLES_PER_SECOND = 44100;

// AudioBuffer
static int au_data_buffer[441*32];
static int au_sdl_frame;
static int au_cpu_frame;

enum Buttons {

    BUTTON_A        = 0x80,
    BUTTON_B        = 0x40,
    BUTTON_SELECT   = 0x20,
    BUTTON_START    = 0x10,
    BUTTON_UP       = 0x08,
    BUTTON_DOWN     = 0x04,
    BUTTON_LEFT     = 0x02,
    BUTTON_RIGHT    = 0x01
};

static const char* s_opcodes[7]  = {"LD ", "AND", "OR ", "XOR", "ADD", "SUB", "ST "};
static const char* s_branches[8] = {"JMP", "BGT", "BLT", "BNE", "BEQ", "BGE", "BLE", "BRA"};

class Gigatron {

protected:

    int width, height;

    SDL_Event       event;
    SDL_Surface*    sdl_screen;
    SDL_AudioSpec   audio_device;

    // CPU
    uint8_t  started;
    uint16_t rom[65536]; // 64k x 16
    uint8_t  ram[65536]; // 64k x 8
    uint16_t pc, nextpc;
    uint16_t ramMask;
    uint8_t  ac, x, y, out, outx, inReg, ctrl;

    // VGA
    int row, col,
        minRow, maxRow,
        minCol, maxCol,
        vga_out;

    uint32_t vga_buffer[525][800];

    // Gamepad
    uint8_t  press_shift, press_ctrl, press_alt;

    // Audio
    int au_cycle, au_cpu_shift, au_sample_shift;

    // Disasm
    char     require_stop;
    uint16_t disasm_cursor;
    uint16_t disasm_start;
    char     disasm_row[50];
    char     disasm_opcode[8];
    char     disasm_op1[16];
    char     disasm_op2[16];

public:

    Gigatron(int, int, const char*);

    // Инициализация
    void    start();
    void    reset();
    void    procstart(int, char**);

    // CPU
    void     run();
    void     stop();
    void     all_tick();
    void     tick();
    void     aluOp   (uint8_t op,   uint8_t mode, uint8_t bus, uint8_t d);
    void     storeOp (uint8_t mode, uint8_t bus,  uint8_t d);
    void     branchOp(uint8_t mode, uint8_t bus,  uint8_t d);
    uint16_t address (uint8_t mode, uint8_t d);
    uint16_t offset  (uint8_t bus,  uint8_t d);

    // VGA
    void     vga_init();
    void     vga_tick();
    void     pset(int x, int y, uint32_t color);

    // Gamepad
    void     gamepad_press(SDL_Event event);
    void     gamepad_up(SDL_Event event);
    int      get_key(SDL_Event event, int keydown);

    // Audio
    void    audio_init();
    void    audio_tick();

    // Дизассемблер
    char*   disasm(uint16_t address);
    void    list();
    void    print_char_16(int col, int row, unsigned char ch, uint cl);
    void    print(int col, int row, const char* s, uint cl);
    void    debugger_press(SDL_Event event);
};
