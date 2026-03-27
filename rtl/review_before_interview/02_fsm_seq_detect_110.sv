module seq_detect_110 (
    input  logic clk,
    input  logic rst,
    input  logic bit_in,
    output logic detect
);

    // Sequence detector for the bit pattern 1-1-0 using an explicit FSM.
    // - On each posedge clk:
    //   - if rst is 1, the FSM returns to idle
    //   - else it consumes bit_in and transitions to the next state
    // - detect should pulse high for one cycle when 1-1-0 is seen.
    // - Overlapping matches are allowed.
    // - Use 3-block FSM style with registered output.

    typedef enum logic[1:0] {
        IDLE,
        S1,
        S11
     } state_t;

    state_t state, next_state;

    always_comb begin
        next_state = state;
        case (state)
            IDLE: begin
                next_state = (bit_in) ? S1 : IDLE;
            end
            S1: begin
                next_state = (bit_in) ? S11 : IDLE;
            end
            S11: begin
                if(bit_in) begin
                    next_state = S11;
                end
                else next_state = IDLE;
            end
            default: begin
                next_state = IDLE;
            end
        endcase
    end

    always_ff @(posedge clk) begin
        if (rst) begin
            detect <= 1'b0;
            state <= IDLE;
        end
        else begin
            state <= next_state;
            detect <= ((state == S11) && !bit_in);
        end
    end
endmodule
