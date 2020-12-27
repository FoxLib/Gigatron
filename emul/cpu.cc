#include "gigatron.h"

/** reset registers to power-on state */
void Gigatron::reset() {

    pc      = 0;
    nextpc  = (pc + 1) & 0xFFFF;
    ac      = 0;
    x       = 0;
    y       = 0;
    out     = 0;
    outx    = 0;
    ctrl    = 0;
    inReg   = 0xff; // active low!
}

/** advance simulation by one tick */
void Gigatron::tick() {

    uint16_t let_pc = pc;

    pc     = nextpc;
    nextpc = (nextpc + 1) & 0xFFFF;

    uint16_t ir   = rom[ let_pc ];
    uint8_t  op   = (ir >> 13) & 0x0007;
    uint8_t  mode = (ir >> 10) & 0x0007;
    uint8_t  bus  = (ir >>  8) & 0x0003;
    uint8_t  d    = (ir >>  0) & 0x00ff;

    switch (op) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            aluOp(op, mode, bus, d);
            break;
        case 6:
            storeOp(mode, bus, d);
            break;
        case 7:
            branchOp(mode, bus, d);
            break;
    }
}

/** perform an alu op
 * @param {number} op
 * @param {number} mode
 * @param {number} bus
 * @param {number} d
 */
void Gigatron::aluOp(uint8_t op, uint8_t mode, uint8_t bus, uint8_t d) {

    uint8_t  b = 0, rising;
    uint16_t addr;

    switch (bus & 3) {

        case 0:
            b = d;
            break;
        case 1:
            b = ram[ address(mode, d) & ramMask ];
            break;
        case 2:
            b = ac;
            break;
        case 3:
            b = inReg;
            break;
    }

    switch (op & 7) {
        case 1:
            b = ac & b;
            break;
        case 2:
            b = ac | b;
            break;
        case 3:
            b = ac ^ b;
            break;
        case 4:
            b = (ac + b) & 0xff;
            break;
        case 5:
            b = (ac - b) & 0xff;
            break;
    }

    switch (mode & 7) {
        case 0:
        case 1:
        case 2:
        case 3:
            ac = b;
            break;
        case 4:
            x = b;
            break;
        case 5:
            y = b;
            break;
        case 6:
        case 7:
            rising = ~out & b;
            out = b;

            // rising edge of out[6] registers outx from ac
            if (rising & 0x40) {
                outx = ac;
            }

            break;
    }
}

/** perform a store op
 * @param {number} mode
 * @param {number} bus
 * @param {number} d
 */
void Gigatron::storeOp(uint8_t mode, uint8_t bus, uint8_t d) {

    uint8_t b = 0;

    switch (bus & 3) {
        case 0:
            b = d;
            break;
        case 1:
            ctrl = d;
            break;
        case 2:
            b = ac;
            break;
        case 3:
            b = inReg;
            break;
    }

    if (bus == 1)
        return;

    ram[ address(mode, d) & ramMask ] = b;

    switch (mode) {
        case 4: // XXX not clear whether x++ mode takes priority
            x = b;
            break;
        case 5:
            y = b;
            break;
    }
}

/** perform a branch op
 * @param {number} mode
 * @param {number} bus
 * @param {number} d
 */
void Gigatron::branchOp(uint8_t mode, uint8_t bus, uint8_t d) {

    uint8_t  c = 0;
    uint8_t  let_ac = ac ^ ZERO;        // ~ac[7], ac[6:0]
    uint16_t base   = pc & 0xff00;

    switch (mode) {
        case 0: // jmp [y,<offset>]
            c = 1;
            base = y << 8;
            break;
        case 1: // bgt
            c = let_ac > ZERO;  // (ac^0x80) > 0x80
            break;
        case 2: // blt
            c = let_ac < ZERO;  // (ac^0x80) < 0x80
            break;
        case 3: // bne
            c = let_ac != ZERO; // (ac^0x80) != 0x80
            break;
        case 4: // beq
            c = let_ac == ZERO; // (ac^0x80) == 0x80
            break;
        case 5: // bge
            c = let_ac >= ZERO; // (ac^0x80) >= 0x80
            break;
        case 6: // ble
            c = let_ac <= ZERO; // (ac^0x80) <= 0x80
            break;
        case 7: // bra
            c = 1;
            break;
    }

    if (c) {
        nextpc = base | (offset(bus, d));
    }
}

/** calculate a ram address
 * @param {number} mode
 * @param {number} d
 * @return {number} the address
 */
uint16_t Gigatron::address(uint8_t mode, uint8_t d) {

    uint16_t addr;

    switch (mode & 7) {
        case 0:
        case 4:
        case 5:
        case 6:
            return d;
        case 1:
            return x;
        case 2:
            return (y << 8) | d;
        case 3:
            return (y << 8) | x;
        case 7:
            addr = (y << 8) | x;
            x = (x + 1) & 0xff;
            return addr;
    }
    return 0;
}

/** calculate a branch page offset
 * @param {number} bus
 * @param {number} d
 * @return {number} page offset
 */
uint16_t Gigatron::offset(uint8_t bus, uint8_t d) {

    switch (bus & 3) {
        case 0:
            return d;
        case 1:
            // RAM always has at least 1 page, so no need to mask address
            return ram[d];
        case 2:
            return ac;
        case 3:
            return inReg;
    }
    return 0;
}

void Gigatron::run()  { started = 1; }
void Gigatron::stop() { started = 0; }
