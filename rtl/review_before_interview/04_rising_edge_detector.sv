module rising_edge_detector (
    input  logic clk,
    input  logic rst,
    input  logic sig,
    output logic pulse
);
    // Rising-edge detector.
    // - On each posedge clk, detect a 0->1 transition on sig.
    // - pulse should assert for one clock cycle when a rising edge is seen.
    // - If rst is 1, internal state and pulse reset to 0.
    logic previous_sig;

    always_ff @(posedge clk) begin
        if(rst) begin 
            pulse <= 1'b0;
            previous_sig <= 1'b0;
        end
        else begin
            pulse <= 1'b0;
            if (sig==1'b1 && previous_sig==1'b0) begin
                pulse <= 1'b1;
            end
            previous_sig <= sig;
        end
    end
endmodule
