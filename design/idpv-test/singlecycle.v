module singlecycle(input_a, input_b, product, underflow, overflow, result);
  input[31:0] input_a, input_b;
  output underflow, overflow;
  output reg [31:0] product;
  output reg [31:0] result;
  wire[7:0] exponent_a, exponent_b, exponent_sum, final_exponent, error_check_exponent;
  wire sign_a, sign_b, sign_final, zero_a, zero_b;
  wire[23:0] mantissa_a, mantissa_b; 
  wire[22:0] final_mantissa;
  wire[47:0] mantissa_prod;
  assign zero_a = ~(|input_a);
  assign zero_b = ~(|input_b);
  assign mantissa_a[23] = 1'b1;
  assign mantissa_b[23] = 1'b1;
  assign {sign_a, exponent_a, mantissa_a[22:0]} = input_a;
  assign {sign_b, exponent_b, mantissa_b[22:0]} = input_b;
  assign sign_final = sign_a ^ sign_b;
  assign mantissa_prod = (mantissa_a * mantissa_b) + 1'b1;
  assign exponent_sum = exponent_a + exponent_b - 8'd127;
  assign final_mantissa = mantissa_prod[47] ? mantissa_prod[47:24] : mantissa_prod[46:23] + 1'b1;
  assign final_exponent = mantissa_prod[47] ? exponent_sum + 1 : exponent_sum;
  assign overflow = (exponent_a[7] & exponent_b[7]) & ~final_exponent[7];
  assign underflow = (~exponent_b[7] & ~exponent_a[7] & final_exponent[7]);
  assign error_check_exponent = underflow ? (overflow ? final_exponent : 8'b0) : (overflow ? 8'b11111111 : final_exponent);
  assign product = (zero_a | zero_b) ? 32'b0 : {sign_final, error_check_exponent, final_mantissa};
  always @(*) begin
      result <= product;
  end
    
endmodule