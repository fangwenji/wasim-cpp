module Mul(
  input         clock,
  input         reset,
  input         io_mul_valid,
  input         io_mulw,
  input  [63:0] io_multiplicand,
  input  [63:0] io_multiplier,
  output        io_out_valid,
  input         io_out_ready,
  output [31:0] io_result_hi,
  output [31:0] io_result_lo
);
`ifdef RANDOMIZE_REG_INIT
  reg [63:0] _RAND_0;
  reg [31:0] _RAND_1;
  reg [127:0] _RAND_2;
  reg [95:0] _RAND_3;
`endif // RANDOMIZE_REG_INIT
  wire [2:0] booth_partial_io_y; // @[Mul.scala 72:31]
  wire [63:0] booth_partial_io_x; // @[Mul.scala 72:31]
  wire  booth_partial_io_c; // @[Mul.scala 72:31]
  wire [63:0] booth_partial_io_p; // @[Mul.scala 72:31]
  wire  sign = io_multiplicand[63] ^ io_multiplier[63]; // @[Mul.scala 52:33]
  reg [63:0] res; // @[Mul.scala 54:22]
  reg [1:0] state; // @[Mul.scala 56:24]
  reg [127:0] src1; // @[Mul.scala 58:23]
  reg [64:0] src2; // @[Mul.scala 59:23]
  wire [31:0] _src1_32_T_2 = io_multiplicand[31] ? 32'hffffffff : 32'h0; // @[Bitwise.scala 74:12]
  wire [63:0] _src1_32_T_4 = {_src1_32_T_2,io_multiplicand[31:0]}; // @[Cat.scala 31:58]
  wire [63:0] src1_32 = io_mulw ? _src1_32_T_4 : io_multiplicand; // @[Mul.scala 66:19]
  wire [31:0] _src2_32_T_2 = io_multiplier[31] ? 32'hffffffff : 32'h0; // @[Bitwise.scala 74:12]
  wire [63:0] _src2_32_T_4 = {_src2_32_T_2,io_multiplier[31:0]}; // @[Cat.scala 31:58]
  wire [63:0] src2_32 = io_mulw ? _src2_32_T_4 : io_multiplier; // @[Mul.scala 67:19]
  wire [63:0] _real_cand_T_5 = src1_32[63] ? 64'hffffffffffffffff : 64'h0; // @[Bitwise.scala 74:12]
  wire [127:0] real_cand = {_real_cand_T_5,src1_32}; // @[Cat.scala 31:58]
  wire [64:0] _real_er_T = {src2_32,1'h0}; // @[Cat.scala 31:58]
  wire [127:0] _GEN_1 = io_mul_valid ? real_cand : src1; // @[Mul.scala 78:44 80:22 58:23]
  wire [63:0] real_er = _real_er_T[63:0]; // @[Mul.scala 62:23 70:13]
  wire  _T_4 = src2 != 65'h0; // @[Mul.scala 89:26]
  wire [63:0] _res_T_1 = res + booth_partial_io_p; // @[Mul.scala 90:32]
  wire [63:0] _GEN_21 = {{63'd0}, booth_partial_io_c}; // @[Mul.scala 90:53]
  wire [63:0] _res_T_3 = _res_T_1 + _GEN_21; // @[Mul.scala 90:53]
  wire [129:0] _src1_T = {src1, 2'h0}; // @[Mul.scala 92:34]
  wire [1:0] _GEN_4 = io_out_ready ? 2'h0 : state; // @[Mul.scala 56:24 94:39 95:31]
  wire [129:0] _GEN_7 = src2 != 65'h0 ? _src1_T : {{2'd0}, src1}; // @[Mul.scala 58:23 89:33 92:26]
  wire [129:0] _GEN_16 = 2'h1 == state ? _GEN_7 : {{2'd0}, src1}; // @[Mul.scala 76:18 58:23]
  wire [129:0] _GEN_18 = 2'h0 == state ? {{2'd0}, _GEN_1} : _GEN_16; // @[Mul.scala 76:18]
  wire [31:0] _io_result_hi_T_1 = {sign,res[62:32]}; // @[Cat.scala 31:58]
  wire  _GEN_22 = _T_4 ? 1'h0 : 1'h1; // @[Mul.scala 108:35 110:26 116:26]
  wire [31:0] _GEN_23 = _T_4 ? 32'h0 : _io_result_hi_T_1; // @[Mul.scala 108:35 111:26 114:26]
  wire [31:0] _GEN_24 = _T_4 ? 32'h0 : res[31:0]; // @[Mul.scala 108:35 112:26 115:26]
  wire  _GEN_26 = state == 2'h1 & _GEN_22; // @[Mul.scala 107:33 121:22]
  wire [31:0] _GEN_27 = state == 2'h1 ? _GEN_23 : 32'h0; // @[Mul.scala 107:33 122:22]
  wire [31:0] _GEN_28 = state == 2'h1 ? _GEN_24 : 32'h0; // @[Mul.scala 107:33 123:22]
  wire [129:0] _GEN_25 = reset ? 130'h0 : _GEN_18; // @[Mul.scala 58:{23,23}]
  partial_product booth_partial ( // @[Mul.scala 72:31]
    .io_y(booth_partial_io_y),
    .io_x(booth_partial_io_x),
    .io_c(booth_partial_io_c),
    .io_p(booth_partial_io_p)
  );
  assign io_out_valid = state == 2'h0 ? 1'h0 : _GEN_26; // @[Mul.scala 102:23 104:22]
  assign io_result_hi = state == 2'h0 ? 32'h0 : _GEN_27; // @[Mul.scala 102:23 105:22]
  assign io_result_lo = state == 2'h0 ? 32'h0 : _GEN_28; // @[Mul.scala 102:23 106:22]
  assign booth_partial_io_y = src2[2:0]; // @[Mul.scala 73:31]
  assign booth_partial_io_x = src1[63:0]; // @[Mul.scala 74:24]
  always @(posedge clock) begin
    if (reset) begin // @[Mul.scala 54:22]
      res <= 64'h0; // @[Mul.scala 54:22]
    end else if (2'h0 == state) begin // @[Mul.scala 76:18]
      if (io_mul_valid) begin // @[Mul.scala 78:44]
        res <= 64'h0; // @[Mul.scala 82:21]
      end
    end else if (2'h1 == state) begin // @[Mul.scala 76:18]
      if (src2 != 65'h0) begin // @[Mul.scala 89:33]
        res <= _res_T_3; // @[Mul.scala 90:25]
      end
    end
    if (reset) begin // @[Mul.scala 56:24]
      state <= 2'h0; // @[Mul.scala 56:24]
    end else if (2'h0 == state) begin // @[Mul.scala 76:18]
      if (io_mul_valid) begin // @[Mul.scala 78:44]
        state <= 2'h1; // @[Mul.scala 79:23]
      end
    end else if (2'h1 == state) begin // @[Mul.scala 76:18]
      if (!(src2 != 65'h0)) begin // @[Mul.scala 89:33]
        state <= _GEN_4;
      end
    end
    src1 <= _GEN_25[127:0]; // @[Mul.scala 58:{23,23}]
    if (reset) begin // @[Mul.scala 59:23]
      src2 <= 65'h0; // @[Mul.scala 59:23]
    end else if (2'h0 == state) begin // @[Mul.scala 76:18]
      if (io_mul_valid) begin // @[Mul.scala 78:44]
        src2 <= {{1'd0}, real_er}; // @[Mul.scala 81:22]
      end
    end else if (2'h1 == state) begin // @[Mul.scala 76:18]
      if (src2 != 65'h0) begin // @[Mul.scala 89:33]
        src2 <= {{2'd0}, src2[64:2]}; // @[Mul.scala 91:26]
      end
    end
  end
// Register and memory initialization
`ifdef RANDOMIZE_GARBAGE_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_INVALID_ASSIGN
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_REG_INIT
`define RANDOMIZE
`endif
`ifdef RANDOMIZE_MEM_INIT
`define RANDOMIZE
`endif
`ifndef RANDOM
`define RANDOM $random
`endif
`ifdef RANDOMIZE_MEM_INIT
  integer initvar;
`endif
`ifndef SYNTHESIS
`ifdef FIRRTL_BEFORE_INITIAL
`FIRRTL_BEFORE_INITIAL
`endif
initial begin
  `ifdef RANDOMIZE
    `ifdef INIT_RANDOM
      `INIT_RANDOM
    `endif
    `ifndef VERILATOR
      `ifdef RANDOMIZE_DELAY
        #`RANDOMIZE_DELAY begin end
      `else
        #0.002 begin end
      `endif
    `endif
`ifdef RANDOMIZE_REG_INIT
  _RAND_0 = {2{`RANDOM}};
  res = _RAND_0[63:0];
  _RAND_1 = {1{`RANDOM}};
  state = _RAND_1[1:0];
  _RAND_2 = {4{`RANDOM}};
  src1 = _RAND_2[127:0];
  _RAND_3 = {3{`RANDOM}};
  src2 = _RAND_3[64:0];
`endif // RANDOMIZE_REG_INIT
  `endif // RANDOMIZE
end // initial
`ifdef FIRRTL_AFTER_INITIAL
`FIRRTL_AFTER_INITIAL
`endif
`endif // SYNTHESIS
endmodule