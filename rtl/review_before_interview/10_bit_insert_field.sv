// Question 6: Insert Field
//
// Write synthesizable combinational RTL.
//
// Requirement:
// Replace bits `in[MSB:LSB]` with `field`.
// All other bits of `in` must pass through unchanged.
//
// Assumptions:
// - `0 <= LSB <= MSB < WIDTH`
// - `field` width is exactly `MSB-LSB+1`
//
// Example with default parameters WIDTH=8, MSB=5, LSB=2:
// in    = 8'b1101_0010
// field = 4'b1010
// out   = 8'b1110_1010
//
// in    = 8'h3C
// field = 4'h0
// out   = 8'h00

module insert_field #(
    parameter int WIDTH = 8,
    parameter int MSB   = 5,
    parameter int LSB   = 2
) (
    input  logic [WIDTH-1:0] in,
    input  logic [MSB-LSB:0] field,
    output logic [WIDTH-1:0] out
);
    
    assign out = { in[WIDTH-1:MSB+1], field, in[LSB-1:0] };

endmodule
