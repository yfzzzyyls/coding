module sync_fifo #(
    parameter DATA_WIDTH = 8,
    parameter DEPTH      = 8
)(
    input  logic                  clk,
    input  logic                  rst,
    input  logic                  wr_en,
    input  logic [DATA_WIDTH-1:0] wr_data,
    input  logic                  rd_en,
    output logic [DATA_WIDTH-1:0] rd_data,
    output logic                  full,
    output logic                  empty
);

    // Synchronous FIFO with parameterized data width and depth.
    // - Single clock domain for both read and write.
    // - wr_en: write wr_data into the FIFO when asserted and FIFO is not full.
    // - rd_en: read from the FIFO when asserted and FIFO is not empty.
    //   rd_data should present the read value on the same cycle rd_en is asserted.
    // - full: asserted when FIFO cannot accept more writes.
    // - empty: asserted when FIFO has no data to read.
    // - Use a register array for storage and pointer-based tracking.
    // - Think about how to distinguish full vs empty when pointers are equal.

    localparam PTR_WIDTH = $clog2(DEPTH);

    logic [PTR_WIDTH: 0] head; // 1 extra bit
    logic [PTR_WIDTH: 0] tail; // 1 extra bit

    logic [DATA_WIDTH - 1 : 0] fifo [DEPTH - 1 : 0];

    logic [DATA_WIDTH - 1 : 0] data;

    always_ff @(posedge clk) begin
        if (rst) begin
            head <= '0;
            tail <= '0;
        end
        else begin
            if(wr_en && !full) begin 
                fifo[tail[PTR_WIDTH-1:0]] <= wr_data;
                tail <= tail + 1'b1;
            end
            if (rd_en && !empty) begin
                data <= fifo[head[PTR_WIDTH-1:0]];
                head <= head + 1'b1;
            end
        end
    end

    assign rd_data = data;

    assign full = (head[PTR_WIDTH - 1 : 0] == tail[PTR_WIDTH - 1 : 0] && 
                                head[PTR_WIDTH] != tail[PTR_WIDTH]);

    // assign empty = (head[PTR_WIDTH - 1 : 0] == tail[PTR_WIDTH - 1 : 0] && 
    //                         head[PTR_WIDTH] == tail[PTR_WIDTH]);
    assign empty = head == tail;
    
endmodule
