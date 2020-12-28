module de0(

      /* Reset */
      input              RESET_N,

      /* Clocks */
      input              CLOCK_50,
      input              CLOCK2_50,
      input              CLOCK3_50,
      inout              CLOCK4_50,

      /* DRAM */
      output             DRAM_CKE,
      output             DRAM_CLK,
      output      [1:0]  DRAM_BA,
      output      [12:0] DRAM_ADDR,
      inout       [15:0] DRAM_DQ,
      output             DRAM_CAS_N,
      output             DRAM_RAS_N,
      output             DRAM_WE_N,
      output             DRAM_CS_N,
      output             DRAM_LDQM,
      output             DRAM_UDQM,

      /* GPIO */
      inout       [35:0] GPIO_0,
      inout       [35:0] GPIO_1,

      /* 7-Segment LED */
      output      [6:0]  HEX0,
      output      [6:0]  HEX1,
      output      [6:0]  HEX2,
      output      [6:0]  HEX3,
      output      [6:0]  HEX4,
      output      [6:0]  HEX5,

      /* Keys */
      input       [3:0]  KEY,

      /* LED */
      output      [9:0]  LEDR,

      /* PS/2 */
      inout              PS2_CLK,
      inout              PS2_DAT,
      inout              PS2_CLK2,
      inout              PS2_DAT2,

      /* SD-Card */
      output             SD_CLK,
      inout              SD_CMD,
      inout       [3:0]  SD_DATA,

      /* Switch */
      input       [9:0]  SW,

      /* VGA */
      output      [3:0]  VGA_R,
      output      [3:0]  VGA_G,
      output      [3:0]  VGA_B,
      output             VGA_HS,
      output             VGA_VS
);

// Z-state
assign DRAM_DQ = 16'hzzzz;
assign GPIO_0  = 36'hzzzzzzzz;
assign GPIO_1  = {1'b0, 1'bz, outx[7], 33'hzzzzzzzz};

// LED OFF
assign HEX0 = 7'b1111111;
assign HEX1 = 7'b1111111;
assign HEX2 = 7'b1111111;
assign HEX3 = 7'b1111111;
assign HEX4 = 7'b1111111;
assign HEX5 = 7'b1111111;

// Модуль SDRAM
assign DRAM_CKE  = 0; // ChipEnable=1
assign DRAM_CS_N = 1; // ChipSelect=0

// Вывод "blinker light"
assign LEDR = pwm_light < 1024 ? outx[3:0] : 0;
assign {VGA_VS, VGA_HS, VGA_B[3:2], VGA_G[3:2], VGA_R[3:2]} = out;

// Снизить яркость светодиодов
reg [15:0] pwm_light; always @(posedge clock_625) pwm_light <= pwm_light + 1;

// ---------------------------------------------------------------------
// Тактовые генераторы
// ---------------------------------------------------------------------

wire clock_625;
wire clock_25;
wire clock_50;
wire clock_100;

pll u0(

    // Источник тактирования
    .clkin (CLOCK_50),

    // Производные частоты
    .m625   (clock_625),
    .m25    (clock_25),
    .m50    (clock_50),
    .m75    (clock_75),
    .m100   (clock_100),
    .m106   (clock_106),
    .locked (locked)
);

// -----------------------------------------------------------------------
// Процессор Gigatron
// -----------------------------------------------------------------------

wire [15:0] pc;             // Program Counter
wire [15:0] ir;             // К IR
wire [15:0] r_addr;         // Адрес памяти на чтение
wire [15:0] w_addr;         // Адрес записи
wire [ 7:0] i_data;         // Данные из памяти
wire [ 7:0] o_data;         // Данные на запись
wire        o_we;           // Сигнал записи
reg  [ 7:0] inreg = 8'hFF;  // Клавиатура (и входные данные)
wire [ 7:0] out;            // К VGA
wire [ 7:0] outx;           // К audio и blink
wire [ 7:0] ctrl;           // Управляющие сигналы

gigatron TTL
(
    // Программа
    .clock      (clock_625 & locked),
    .rst_n      (1'b1),
    .pc         (pc),
    .rom_i      (ir),

    // Интерфейс памяти
    .addr_r     (r_addr),
    .addr_w     (w_addr),
    .data_i     (i_data),
    .data_o     (o_data),
    .we         (o_we),

    // Порты ввода-вывода
    .inreg      (inreg),
    .out        (out),
    .outx       (outx),
    .ctrl       (ctrl)
);

// ---------------------------------------------------------------------
// Контроллер памяти
// ---------------------------------------------------------------------

// Коды программы
rom UnitROM
(
    .clock      (clock_100),
    .address_a  (pc),
    .q_a        (ir)
);

// Память двухпортовая
ram UnitRAM
(
    .clock      (clock_100),
    .address_a  (r_addr),
    .q_a        (i_data),
    .address_b  (w_addr),
    .data_b     (o_data),
    .wren_b     (o_we),
);

// ---------------------------------------------------------------------
// Контроллер клавиатуры
// ---------------------------------------------------------------------

wire [7:0]  ps2data;
wire        ps2hit;
reg         shift = 1'b0;
reg         released  = 1'b0;

keyboard keyb
(
    .CLOCK_50           (clock_50), // Тактовый генератор на 50 Мгц
    .PS2_CLK            (PS2_CLK),  // Таймингс PS/2
    .PS2_DAT            (PS2_DAT),  // Данные с PS/2
    .received_data      (ps2data),  // Принятые данные
    .received_data_en   (ps2hit),   // Нажата клавиша
);

// Прием символа (пример)
always @(posedge clock_50) begin

    if (ps2hit) begin

        // Клавиша отпущена
        if (ps2data == 8'hF0) begin released <= 1'b1; end

        // Другие клавиши
        else begin

            case (ps2data)

                // Левый и правый шифт равнозначны
                /* SH */ 8'h12, 8'h59: shift <= ~released;

                // Цифробуквенная клавиатура
                /* A  */ 8'h1C: inreg <= shift ? 8'h41 : 8'h61;
                /* B  */ 8'h32: inreg <= shift ? 8'h42 : 8'h62;
                /* C  */ 8'h21: inreg <= shift ? 8'h43 : 8'h63;
                /* D  */ 8'h23: inreg <= shift ? 8'h44 : 8'h64;
                /* E  */ 8'h24: inreg <= shift ? 8'h45 : 8'h65;
                /* F  */ 8'h2B: inreg <= shift ? 8'h46 : 8'h66;
                /* G  */ 8'h34: inreg <= shift ? 8'h47 : 8'h67;
                /* H  */ 8'h33: inreg <= shift ? 8'h48 : 8'h68;
                /* I  */ 8'h43: inreg <= shift ? 8'h49 : 8'h69;
                /* J  */ 8'h3B: inreg <= shift ? 8'h4A : 8'h6A;
                /* K  */ 8'h42: inreg <= shift ? 8'h4B : 8'h6B;
                /* L  */ 8'h4B: inreg <= shift ? 8'h4C : 8'h6C;
                /* M  */ 8'h3A: inreg <= shift ? 8'h4D : 8'h6D;
                /* N  */ 8'h31: inreg <= shift ? 8'h4E : 8'h6E;
                /* O  */ 8'h44: inreg <= shift ? 8'h4F : 8'h6F;
                /* P  */ 8'h4D: inreg <= shift ? 8'h50 : 8'h70;
                /* Q  */ 8'h15: inreg <= shift ? 8'h51 : 8'h71;
                /* R  */ 8'h2D: inreg <= shift ? 8'h52 : 8'h72;
                /* S  */ 8'h1B: inreg <= shift ? 8'h53 : 8'h73;
                /* T  */ 8'h2C: inreg <= shift ? 8'h54 : 8'h74;
                /* U  */ 8'h3C: inreg <= shift ? 8'h55 : 8'h75;
                /* V  */ 8'h2A: inreg <= shift ? 8'h56 : 8'h76;
                /* W  */ 8'h1D: inreg <= shift ? 8'h57 : 8'h77;
                /* X  */ 8'h22: inreg <= shift ? 8'h58 : 8'h78;
                /* Y  */ 8'h35: inreg <= shift ? 8'h59 : 8'h79;
                /* Z  */ 8'h1A: inreg <= shift ? 8'h5A : 8'h7A;
                /* 0) */ 8'h45: inreg <= shift ? 8'h29 : 8'h30;
                /* 1! */ 8'h16: inreg <= shift ? 8'h21 : 8'h31;
                /* 2@ */ 8'h1E: inreg <= shift ? 8'h40 : 8'h32;
                /* 3# */ 8'h26: inreg <= shift ? 8'h23 : 8'h33;
                /* 4$ */ 8'h25: inreg <= shift ? 8'h24 : 8'h34;
                /* 5% */ 8'h2E: inreg <= shift ? 8'h25 : 8'h35;
                /* 6^ */ 8'h36: inreg <= shift ? 8'h5E : 8'h36;
                /* 7& */ 8'h3D: inreg <= shift ? 8'h26 : 8'h37;
                /* 8* */ 8'h3E: inreg <= shift ? 8'h2A : 8'h38;
                /* 9( */ 8'h46: inreg <= shift ? 8'h28 : 8'h39;

                // Спецсимволы
                /* `~ */ 8'h0E: inreg <= shift ? 8'h7E : 8'h60;
                /* -_ */ 8'h4E: inreg <= shift ? 8'h5F : 8'h2D;
                /* =+ */ 8'h55: inreg <= shift ? 8'h2B : 8'h3D;
                /* \| */ 8'h5D: inreg <= shift ? 8'h7C : 8'h5C;
                /* [{ */ 8'h54: inreg <= shift ? 8'h7B : 8'h5B;
                /* ]} */ 8'h5B: inreg <= shift ? 8'h7D : 8'h5D;
                /* ;: */ 8'h4C: inreg <= shift ? 8'h3A : 8'h3B;
                /* '" */ 8'h52: inreg <= shift ? 8'h22 : 8'h27;
                /* ,< */ 8'h41: inreg <= shift ? 8'h3C : 8'h2C;
                /* .> */ 8'h49: inreg <= shift ? 8'h3E : 8'h2E;
                /* /? */ 8'h4A: inreg <= shift ? 8'h3F : 8'h2F;

                // Разные клавиши
                /* SP */ 8'h29: inreg <= 8'h20;
                /* TB */ 8'h0D: inreg <= 8'h09;
                /* EN */ 8'h5A: inreg <= 8'h0A;
                /* BS */ 8'h66: inreg <= 8'h7F;
                /* ES */ 8'h76: inreg <= 8'h1B;

                // Клавиши джойстика, на самом деле
                8'h74: inreg <= inreg & ~8'h01; // RIGHT
                8'h6B: inreg <= inreg & ~8'h02; // LEFT
                8'h72: inreg <= inreg & ~8'h04; // DOWN
                8'h75: inreg <= inreg & ~8'h08; // UP
                8'h69: inreg <= inreg & ~8'h10; // START    | INS
                8'h71: inreg <= inreg & ~8'h20; // SELECT   | DEL
                8'h70: inreg <= inreg & ~8'h40; // B        | INS
                8'h6C: inreg <= inreg & ~8'h80; // A        | HOME

            endcase

            // Эта клавиша была отпущена
            if (released) inreg <= 8'hFF;

            released <= 1'b0;

        end

    end

end

endmodule
