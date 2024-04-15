module top(A,B,M,N,O,clk,result);
    input [15:0] A,B;
    input [3:0] M,N;
    output [62:0] O;
    input clk;
    output reg [62:0]result;
    wire [31:0] C;
    wire [4:0] P;
    assign C = A * B;
    assign P = M + N;
    assign O = C << P;
    always@(posedge clk)begin
        result <= O;
    end
endmodule