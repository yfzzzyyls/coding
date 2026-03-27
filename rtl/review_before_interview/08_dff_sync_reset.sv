module dff_sync_reset #(
    parameter int WIDTH = 8
) (
    input  logic             clk,
    input  logic             rst,
    input  logic [WIDTH-1:0] d,
    output logic [WIDTH-1:0] q
);

    always_ff @(posedge clk) begin
        if(rst) begin
            q <='0;
        end
        else begin
            q <= d;
        end
    end

endmodule
