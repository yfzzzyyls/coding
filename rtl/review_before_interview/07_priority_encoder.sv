// Question 1: Priority Encoder
//
// Write synthesizable combinational RTL.
//
// Requirement:
// Given an 8-bit request vector `req`, output the index of the highest
// asserted bit.
// - If any bit in `req` is 1, assert `valid`
// - `idx` should be the highest asserted bit position
// - If no bits are asserted, drive `valid=0` and `idx=3'd0`
//
// Priority:
// - Bit 7 has the highest priority
// - Bit 0 has the lowest priority
//
// Examples:
// req = 8'b0000_0000 -> valid = 0, idx = 3'd0
// req = 8'b0001_0100 -> valid = 1, idx = 3'd4
// req = 8'b1001_0001 -> valid = 1, idx = 3'd7

module priority_encoder #(parameter int WIDTH = 8) (
    input  logic [7:0] req,
    output logic       valid,
    output logic [2:0] idx
);

    // always_comb begin
    //     casez(req)
    //         8'b1???????: begin
    //             valid = 1'b1;
    //             idx   = 3'd7;
    //         end
    //         8'b01??????: begin
    //             valid = 1'b1;
    //             idx   = 3'd6;
    //         end
    //         8'b001?????: begin
    //             valid = 1'b1;
    //             idx   = 3'd5;
    //         end
    //         8'b0001????: begin
    //             valid = 1'b1;
    //             idx   = 3'd4;
    //         end
    //         8'b00001???: begin
    //             valid = 1'b1;
    //             idx   = 3'd3;
    //         end
    //         8'b000001??: begin
    //             valid = 1'b1;
    //             idx   = 3'd2;
    //         end
    //         8'b0000001?: begin
    //             valid = 1'b1;
    //             idx   = 3'd1;
    //         end
    //         8'b00000001: begin
    //             valid = 1'b1;
    //             idx   = 3'd0;
    //         end
    //         default: begin
    //             valid = 1'b0;
    //             idx   = 3'd0;
    //         end 
    //     endcase
    // end


    always_comb begin
        valid = 1'b0;
        idx = 3'b0;

        for (int i = 0; i < WIDTH; i++) begin
            if(req[i] > 1'b0) begin 
                valid = 1'b1;
                idx = i[2:0];
            end
        end
    end

endmodule