module mod_n_counter #(
    parameter int WIDTH = 8,
    parameter int MOD_N = 10
) (
    input  logic             clk,
    input  logic             rst,
    output logic [WIDTH-1:0] count
);

    // Modulo-N counter with synchronous active-high reset.
    // - On each posedge clk:
    //   - if rst is 1, count resets to 0
    //   - else if count has reached MOD_N-1, count wraps to 0
    //   - else count increments by 1
    // - WIDTH controls the width of count.
    // - MOD_N controls the wrap value.

    always_ff @(posedge clk) begin
        if(rst) begin
            count <= '0;
        end
        else if (count == (MOD_N - 1)) begin
            count <= '0;
        end
        else begin
            count <= count + 1'b1;
        end
    end

endmodule
