module piso_serializer #(
    parameter int WIDTH = 8
) (
    input  logic             clk,
    input  logic             rst,
    input  logic             load,
    input  logic             shift_en,
    input  logic [WIDTH-1:0] parallel_in,
    output logic             serial_out,
    output logic             busy
);

    // Parallel-in serial-out serializer.
    // - On each posedge clk:
    //   - if rst is 1, internal state resets to 0 and busy deasserts
    //   - else if load is 1, load parallel_in into the internal shift register
    //     and mark the serializer busy with WIDTH bits remaining
    //   - else if shift_en is 1 and busy is 1, shift right by 1 bit and
    //     consume one serialized bit
    // - serial_out is the current LSB of the internal shift register.
    // - busy is 1 while serialized bits remain to be shifted out.

    logic [WIDTH-1:0] internal;
    logic [WIDTH-1:0] count;

    always_ff @(posedge clk) begin
        if(rst) begin
            internal <= '0;
            busy <= 1'b0;
            count <= 0;
        end
        else if (load) begin
            internal <= parallel_in;
            count <= WIDTH;
            busy <= 1'b1;
        end
        else if (shift_en && busy) begin
            internal <= internal >> 1;
            if (count == 1) begin 
                busy <= 1'b0;
            end
            count <= count - 1;
        end
    end

    assign serial_out = internal[0];

endmodule
