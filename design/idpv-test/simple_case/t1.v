module f(input clk, input [31:0] a, input [31:0] b, output reg [31:0] ret);

always @(posedge clk)
	ret <= a + b;

endmodule : f