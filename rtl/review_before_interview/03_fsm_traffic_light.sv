module traffic_light #(
    parameter GREEN_TICKS  = 5,
    parameter YELLOW_TICKS = 2,
    parameter RED_TICKS    = 5
)(
    input  logic       clk,
    input  logic       rst,
    output logic [1:0] light
);

    // Traffic light controller FSM.
    // - Cycles through GREEN -> YELLOW -> RED -> GREEN ...
    // - Stays in each state for the parameterized number of clock cycles.
    // - light encoding: 00 = GREEN, 01 = YELLOW, 10 = RED.
    // - On rst, go to GREEN and reset the internal counter.
    // - Use 3-block FSM style with registered output.

    typedef enum logic [1:0] {
        GREEN,
        YELLOW,
        RED
    } state_t;

    state_t state, next_state;

    logic [2:0] counter;

    always_comb begin
        next_state = state;
        case (state)
            GREEN: begin
                if(counter == 0) next_state = YELLOW;
                else next_state = GREEN;
            end
            YELLOW: begin
                if(counter == 0) next_state = RED;
                else next_state = YELLOW;
            end
            RED: begin
                if(counter == 0) next_state = GREEN;
                else next_state = RED;
            end
            default: begin
                next_state = GREEN;
            end
        endcase
    end

    always_ff @(posedge clk) begin
        if(rst) begin
            state <= GREEN;
            counter <= GREEN_TICKS - 1;
        end
        else begin
            if(state == GREEN) begin
                if(next_state == YELLOW) counter <= YELLOW_TICKS - 1;
                else counter <= counter - 1;
            end
            else if(state == YELLOW) begin
                if(next_state == RED) counter <= RED_TICKS - 1;
                else counter <= counter - 1;
            end
            else if(state == RED) begin
                if(next_state == GREEN) counter <= GREEN_TICKS - 1;
                else counter <= counter - 1;
            end

            state <= next_state;
        end
    end

    assign light = state;

endmodule
