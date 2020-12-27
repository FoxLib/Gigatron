`timescale 10ns / 1ns
module tb;
// ---------------------------------------------------------------------
reg clock;
reg clock_25;
reg clock_50;

always #0.5 clock    = ~clock;
always #1.0 clock_50 = ~clock_50;
always #1.5 clock_25 = ~clock_25;

initial begin clock = 1; clock_25 = 0; clock_50 = 0; #2000 $finish; end
initial begin $dumpfile("tb.vcd"); $dumpvars(0, tb); end
// ---------------------------------------------------------------------

wire [15:0] program_a;
reg  [15:0] program_m[65536];
reg  [15:0] memory[65536];
wire [15:0] addr_r;
wire [15:0] addr_w;
wire [ 7:0] data_o;
wire [ 7:0] data_i = memory[addr_r];
wire [15:0] rom_i = program_m[program_a];
wire        we;
wire [ 7:0] out;
wire [ 7:0] outx;

always @(posedge clock) if (we) memory[addr_w] <= data_o;

initial $readmemh("asm/program.hex", program_m, 16'h0000);
initial $readmemh("memory.hex",  memory, 16'h0000);
// ---------------------------------------------------------------------

gigatron GCPU
(
    .clock  (clock_25),
    .rst_n  (1'b1),
    .pc     (program_a),
    .rom_i  (rom_i),

    // Интерфейс памяти
    .addr_r (addr_r),
    .addr_w (addr_w),
    .data_i (data_i),
    .data_o (data_o),
    .we     (we),
    .out    (out),
    .outx   (outx)
);

endmodule
