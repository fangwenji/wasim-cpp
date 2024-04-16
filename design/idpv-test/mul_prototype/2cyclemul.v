module singleModuleFPM #(parameter N = 32) (
    input clk, reset, enable,
    input signed [N-1:0] A, B,
    output reg signed [N-1:0] result,
    output reg overflow
);
    reg signed [31:0] regA,regB;
    wire signed [31:0] regOut;
    wire regOverflow;

always @(reset) begin
      if (enable) begin
            regA <= 0;
            regB <= 0;
        end
    end
    always @(posedge clk) begin
      if (enable) begin
            regA <= A;
            regB <= B;
        end
    end

    wire SR, zeroFlag;
    wire [8:0] ER;
    wire [23:0] MA, MB;
    wire [47:0] mantissasProduct, mantissaProductNormalized;
    wire [22:0] MR;

    assign zeroFlag = (regA == 32'b0 || regB == 32'b0);
    assign SR = zeroFlag ? 0 : (regA[31] ^ regB[31]);
    assign MA = (regA[30:23] == 8'b0) ? {1'b0, regA[22:0]} : {1'b1, regA[22:0]};
    assign MB = (regB[30:23] == 8'b0) ? {1'b0, regB[22:0]} : {1'b1, regB[22:0]};
    assign mantissasProduct = MA * MB;
    assign mantissaProductNormalized = mantissasProduct[47] ?  mantissasProduct : (mantissasProduct << 1);
    assign MR = zeroFlag ? 0 : mantissaProductNormalized[46:24];
    assign ER = zeroFlag ? 0 : (regA[30:23] + regB[30:23] - 8'b01111111 + mantissasProduct[47]);
    assign regOverflow = (ER[8]) & !zeroFlag;
    assign regOut = {SR, ER[7:0], MR};

    always @(posedge clk) begin
        if (enable) begin
            result <= regOut;
            overflow <=regOverflow;
        end
    end

endmodule