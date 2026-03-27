module dff_async_reset #(
    parameter int WIDTH = 8
) (
    input  logic             clk,
    input  logic             rst_n,
    input  logic [WIDTH-1:0] d,
    output logic [WIDTH-1:0] q
);

    // Asynchronous active-low reset D flip-flop.
    // - If rst_n is 0, q resets immediately to 0.
    // - Otherwise, q captures d on each posedge clk.
    // - WIDTH controls the width of d and q.

    always_ff @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin 
            q <= '0;
        end
        else begin
            q <= d;
        end
    end
endmodule
