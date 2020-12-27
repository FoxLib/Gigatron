/**
 * @desc Процессор, основанный на анализе и повторе js-эмулятора Gigatron
 * @url  https://gigatron.io/
 */

module gigatron
(
    input   wire        clock,
    input   wire        rst_n,
    output  reg  [15:0] pc,
    input   wire [15:0] rom_i,

    // Чтение и запись
    output  reg  [15:0] addr_r,
    output  reg  [15:0] addr_w,
    input   wire [ 7:0] data_i,
    output  reg  [ 7:0] data_o,
    output  reg         we,

    // Порты ввода-вывода
    input   wire [ 7:0] inreg,
    output  reg  [ 7:0] out,
    output  reg  [ 7:0] outx,

    // 76543210
    // ^^^^^^^^
    // |||||||`-- SCLK
    // ||||||`--- Not connected
    // |||||`---- /SS0
    // ||||`----- /SS1
    // |||`------ /SS2
    // ||`------- /SS3
    // |`-------- B0
    // `--------- B1 (Memory Bank)
    output  reg  [ 7:0] ctrl
);

// Регистры
reg  [ 7:0] ac  = 0;
reg  [ 7:0] x   = 0;
reg  [ 7:0] y   = 0;
reg  [15:0] ir  = 16'h0200; /* NOP: LD AC, AC */

initial begin

    pc      = 0;
    we      = 0;
    addr_r  = 0;
    addr_w  = 0;
    data_o  = 0;
    out     = 0;
    outx    = 0;
    ctrl    = 0;

end

// Декодирование IR
// ---------------------------------------------------------------------
wire [ 2:0] op      = ir[15:13]; // 3
wire [ 2:0] mode    = ir[12:10]; // 3
wire [ 1:0] bus     = ir[ 9:8];  // 2
wire [ 7:0] d       = ir[ 7:0];  // 8
wire [ 7:0] zac     = {~ac[7], ac[6:0]};
wire [15:0] pcinc   = pc + 1;

// Вычисления
// ---------------------------------------------------------------------
reg  [ 7:0] b;
reg  [ 7:0] alu;
reg  [ 7:0] base;
reg         cond;

// Комбинационная логика
// ---------------------------------------------------------------------
always @* begin

    base = pc[15:8];

    // Режим ZeroPage для branchOp
    if (op == 7) addr_r = d;
    else case (mode)

        0, 4, 5, 6:
              addr_r = d;
        1:    addr_r = x;
        2:    addr_r = {y, d};
        3, 7: addr_r = {y, x}; // bus=1, mode=7 => X++

    endcase

    // Выборка шины
    case (bus)

        /* IMM */ 2'b00: b = d;
        /* MEM */ 2'b01: b = data_i;
        /* ACC */ 2'b10: b = ac;
        /* INP */ 2'b11: b = inreg;

    endcase

    // Результат вычисления АЛУ
    case (op)

        /* AND  */ 1: alu = ac & b;
        /* OR   */ 2: alu = ac | b;
        /* EOR  */ 3: alu = ac ^ b;
        /* ADD  */ 4: alu = ac + b;
        /* SUB  */ 5: alu = ac - b;
        /* LOAD */ default: alu = b;

    endcase

    // Вычисление условия
    case (mode)

        /* JMP */ 0: begin cond = 1; base = y; end
        /* BGT */ 1: begin cond = zac  > 8'h80; end
        /* BLT */ 2: begin cond = zac  < 8'h80; end
        /* BNE */ 3: begin cond = zac != 8'h80; end
        /* BEQ */ 4: begin cond = zac == 8'h80; end
        /* BGE */ 5: begin cond = zac >= 8'h80; end
        /* BLE */ 6: begin cond = zac <= 8'h80; end
        /* BRA */ 7: begin cond = 1; end

    endcase

end

// Основная логика
// ---------------------------------------------------------------------
always @(posedge clock) begin

    ir <= rom_i;
    pc <= pcinc;
    we <= 0;

    case (op)

        // storeOp
        6: begin

            addr_w <= addr_r;
            data_o <= b;
            we     <= (bus != 1);

            case (mode)

                4: x <= b;
                5: y <= b;
                7: x <= x + 1;

            endcase

            // Дополнительные конфигурации
            if (bus == 1) ctrl <= d;

        end

        // brancOp
        7: begin if (cond) pc <= {base, b}; end

        // aluOp (0-5)
        default: case (mode)

            0, 1, 2, 3: ac <= alu;
            4: x <= alu;
            5: y <= alu;
            6, 7:
            begin

                // Инкремент X при особых условиях
                if (mode == 7 && bus == 1)
                    x <= x + 1;

                // Запись содержимого AC в OUTX @(posedge VGA_HS)
                if (!out[6] && alu[6])
                    outx <= ac;

                // Запись в порт значения АЛУ
                out <= alu;

            end

        endcase

    endcase

end

endmodule
