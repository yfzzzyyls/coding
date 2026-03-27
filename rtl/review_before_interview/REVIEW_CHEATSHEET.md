# RTL Interview Review Cheatsheet

10 representative problems covering the most commonly asked RTL interview topics for embedded/firmware roles.

---

## Category 1: FIFO (Extremely Popular in Interviews!)

### 01 — Synchronous FIFO
**Why it matters**: FIFOs are asked in almost every RTL interview. Know the pointer-based design cold.

```systemverilog
module sync_fifo #(
    parameter DATA_WIDTH = 8,
    parameter DEPTH      = 8
)(
    input  logic                  clk, rst,
    input  logic                  wr_en, rd_en,
    input  logic [DATA_WIDTH-1:0] wr_data,
    output logic [DATA_WIDTH-1:0] rd_data,
    output logic                  full, empty
);
    localparam PTR_WIDTH = $clog2(DEPTH);

    logic [PTR_WIDTH:0] head, tail;    // extra bit for full/empty detection
    logic [DATA_WIDTH-1:0] fifo [DEPTH-1:0];
    logic [DATA_WIDTH-1:0] data;

    always_ff @(posedge clk) begin
        if (rst) begin
            head <= '0;
            tail <= '0;
        end else begin
            if (wr_en && !full) begin
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
    assign full  = (head[PTR_WIDTH-1:0] == tail[PTR_WIDTH-1:0]) &&
                   (head[PTR_WIDTH] != tail[PTR_WIDTH]);
    assign empty = (head == tail);
endmodule
```

**Key concepts to explain in interview**:
- **Extra-bit trick**: Pointers are `PTR_WIDTH+1` bits wide. Lower bits index the array, MSB tracks wrap-around
- **Full**: Lower bits match but MSBs differ (tail has wrapped once more than head)
- **Empty**: Entire pointers are equal (same position, same wrap count)
- **Simultaneous R/W**: Separate `if` blocks (not `if/else if`) allow read and write in the same cycle
- **`$clog2(DEPTH)`**: Computes the number of address bits needed
- **Guard conditions**: `wr_en && !full`, `rd_en && !empty` prevent overflow/underflow

**Common follow-up questions**:
- "What happens if DEPTH is not a power of 2?" — Extra-bit trick still works, but you waste some address space
- "How would you make this an async FIFO?" — Use Gray code pointers + 2-stage synchronizers for CDC
- "Can you do simultaneous read and write when full?" — Need special handling, some designs allow it

---

## Category 2: FSM (Classic Interview Topic)

### 02 — Sequence Detector (110, Overlapping)
**Why it matters**: FSM sequence detectors are the #1 most asked RTL interview question.

```systemverilog
typedef enum logic[1:0] { IDLE, S1, S11 } state_t;
state_t state, next_state;

// Block 1: Next-state combinational logic
always_comb begin
    next_state = state;
    case (state)
        IDLE: next_state = (bit_in) ? S1   : IDLE;
        S1:   next_state = (bit_in) ? S11  : IDLE;
        S11:  next_state = (bit_in) ? S11  : IDLE;  // stay in S11 if another 1
        default: next_state = IDLE;
    endcase
end

// Block 2: State register + output (Mealy-style registered output)
always_ff @(posedge clk) begin
    if (rst) begin
        state  <= IDLE;
        detect <= 1'b0;
    end else begin
        state  <= next_state;
        detect <= (state == S11) && !bit_in;  // Mealy: output depends on input
    end
end
```

**Key concepts**:
- **3-block FSM style**: (1) next-state comb, (2) output comb, (3) state register FF — industry standard
- **Mealy vs Moore**: Mealy output depends on current state AND input (reacts same cycle). Moore output depends only on state (one cycle delay)
- **Overlapping**: From S11, if input is 1, go to S11 (not back to S1). The `11` at the end can start a new `110`
- **Why 3 states, not 4**: Mealy output lets us detect `110` in S11 without needing an S110 state
- **Default case**: Always include for safety — prevents latches in synthesis

**Common follow-up**: "Make it non-overlapping" — After detecting, go to IDLE instead of staying in S11.

---

### 03 — Traffic Light Controller (FSM + Counter)
**Why it matters**: Shows you can combine FSM with counters — common in real embedded designs.

```systemverilog
typedef enum logic [1:0] { GREEN, YELLOW, RED } state_t;
state_t state, next_state;
logic [2:0] counter;

// Next state: transition when counter hits 0
always_comb begin
    next_state = state;
    case (state)
        GREEN:  if (counter == 0) next_state = YELLOW;
        YELLOW: if (counter == 0) next_state = RED;
        RED:    if (counter == 0) next_state = GREEN;
        default: next_state = GREEN;
    endcase
end

// State register + counter management
always_ff @(posedge clk) begin
    if (rst) begin
        state   <= GREEN;
        counter <= GREEN_TICKS - 1;   // load TICKS-1 for correct count
    end else begin
        state <= next_state;
        // Load new counter on state transition, else decrement
        if (state != next_state)
            case (next_state)
                GREEN:  counter <= GREEN_TICKS - 1;
                YELLOW: counter <= YELLOW_TICKS - 1;
                RED:    counter <= RED_TICKS - 1;
            endcase
        else
            counter <= counter - 1;
    end
end

assign light = state;   // enum encoding matches output encoding
```

**Key concepts**:
- **Counter loaded with TICKS-1**: Counts down to 0, so 5 ticks = load 4, count 4→3→2→1→0
- **`assign light = state`**: Enum values (0,1,2) match the desired output encoding — no extra logic needed
- **Parameterized timing**: `GREEN_TICKS`, `YELLOW_TICKS`, `RED_TICKS` make it reusable

---

## Category 3: Sequential Basics

### 04 — Rising Edge Detector
**Why it matters**: Edge detectors appear in almost every digital design. Simple but commonly asked.

```systemverilog
logic previous_sig;

always_ff @(posedge clk) begin
    if (rst) begin
        pulse        <= 1'b0;
        previous_sig <= 1'b0;
    end else begin
        pulse <= (sig && !previous_sig);  // 0→1 transition
        previous_sig <= sig;
    end
end
```

**Key concept**: Register the signal, compare current vs previous. Rising edge = current is 1, previous is 0.

**Variations**: Falling edge = `!sig && previous_sig`. Both edges = `sig ^ previous_sig` (XOR).

---

### 08 — DFF with Synchronous Reset
**Why it matters**: Know the difference between sync and async reset — interviewers always ask.

```systemverilog
// Synchronous reset — reset only checked on clock edge
always_ff @(posedge clk) begin
    if (rst)
        q <= '0;
    else
        q <= d;
end
```

### 09 — DFF with Asynchronous Reset

```systemverilog
// Asynchronous reset — reset acts IMMEDIATELY, not on clock edge
always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n)          // active-low reset
        q <= '0;
    else
        q <= d;
end
```

**Key differences**:

| | Sync Reset | Async Reset |
|---|-----------|-------------|
| Sensitivity list | `@(posedge clk)` | `@(posedge clk or negedge rst_n)` |
| When reset acts | Next clock edge | Immediately |
| Reset polarity | Usually active-high (`rst`) | Usually active-low (`rst_n`) |
| Timing | Easier to meet timing | Can cause metastability if released near clock edge |
| Use case | Most internal logic | Power-on reset, safety-critical |

**Interview question**: "When would you use async reset?" — Power-on reset when clock might not be running yet. But always **synchronously release** an async reset (reset synchronizer) to avoid metastability.

---

## Category 4: Counters & Shift Registers

### 05 — Mod-N Counter
**Why it matters**: Fundamental building block. Used in timers, dividers, UART baud rate generators.

```systemverilog
always_ff @(posedge clk) begin
    if (rst)
        count <= '0;
    else if (count == MOD_N - 1)
        count <= '0;           // wrap to 0
    else
        count <= count + 1'b1;
end
```

**Key concept**: Count from 0 to MOD_N-1, then wrap. For a mod-10 counter: 0,1,2,...,9,0,1,...

**Common use**: Clock divider — output toggles every N cycles.

---

### 06 — PISO Serializer (Parallel-In Serial-Out)
**Why it matters**: Directly relevant to SPI, UART, and other serial protocols in embedded systems.

```systemverilog
logic [WIDTH-1:0] internal;
logic [WIDTH-1:0] count;

always_ff @(posedge clk) begin
    if (rst) begin
        internal <= '0;
        busy     <= 1'b0;
        count    <= 0;
    end
    else if (load) begin
        internal <= parallel_in;    // load the data
        count    <= WIDTH;
        busy     <= 1'b1;
    end
    else if (shift_en && busy) begin
        internal <= internal >> 1;  // shift right, LSB first
        count    <= count - 1;
        if (count == 1) busy <= 1'b0;
    end
end

assign serial_out = internal[0];    // always output LSB
```

**Key concepts**:
- **Load**: Captures parallel data, sets busy, initializes count
- **Shift**: Right-shift by 1, decrement count, LSB comes out first
- **Busy flag**: Tells the controller when serialization is complete
- **serial_out = internal[0]**: Always the current LSB — shifts out one bit per cycle

**Interview relevance**: "How would you implement SPI transmit?" — This is essentially the answer.

---

## Category 5: Combinational Logic

### 07 — Priority Encoder
**Why it matters**: Common combinational building block. Tests `casez` and loop-based approaches.

```systemverilog
// Approach 1: Loop (scalable, parameterizable)
always_comb begin
    valid = 1'b0;
    idx   = 3'b0;
    for (int i = 0; i < WIDTH; i++) begin
        if (req[i]) begin
            valid = 1'b1;
            idx   = i[2:0];    // last match wins = highest priority
        end
    end
end

// Approach 2: casez (explicit, easy to read)
always_comb begin
    casez (req)
        8'b1???????: begin valid=1; idx=7; end
        8'b01??????: begin valid=1; idx=6; end
        // ... down to ...
        8'b00000001: begin valid=1; idx=0; end
        default:     begin valid=0; idx=0; end
    endcase
end
```

**Key concepts**:
- Loop approach: later iterations overwrite earlier ones, so highest bit wins
- `casez`: `?` matches don't-care bits — natural for priority encoding
- Both synthesize to the same hardware

---

### 10 — Bit Field Insert
**Why it matters**: Register field manipulation — directly relevant to firmware/embedded work.

```systemverilog
assign out = { in[WIDTH-1:MSB+1], field, in[LSB-1:0] };
```

**Key concept**: Concatenation `{}` to splice a field into the middle of a word. Upper bits pass through, field replaces the target range, lower bits pass through.

**Interview relevance**: "How would you write to a specific register field without disturbing other bits?" — This is the hardware equivalent of `(reg & ~mask) | (value << shift)` in firmware.

---

## Quick Reference Table

| # | Problem | Category | Key Concept |
|---|---------|----------|-------------|
| 1 | Sync FIFO | Memory | Extra-bit pointer trick, full/empty detection |
| 2 | Seq Detect 110 | FSM | 3-block FSM, Mealy output, overlapping |
| 3 | Traffic Light | FSM+Counter | FSM + timer, counter loaded with TICKS-1 |
| 4 | Rising Edge Detector | Sequential | Compare current vs registered previous |
| 5 | Mod-N Counter | Counter | Count to N-1, wrap to 0 |
| 6 | PISO Serializer | Shift Reg | Load, shift right, LSB-first serial out |
| 7 | Priority Encoder | Combinational | casez or loop, highest bit wins |
| 8 | DFF Sync Reset | Sequential | `@(posedge clk)` only |
| 9 | DFF Async Reset | Sequential | `@(posedge clk or negedge rst_n)` |
| 10 | Bit Field Insert | Bit Manip | Concatenation to splice fields |

---

## Common Interview Questions to Prepare

1. **"Design a sync FIFO"** — Draw the block diagram, explain pointers, full/empty logic
2. **"Design a sequence detector for XYZ"** — Draw the state diagram, write 3-block FSM
3. **"What's the difference between sync and async reset?"** — See table above
4. **"What's Mealy vs Moore?"** — Mealy: output depends on state+input. Moore: output depends on state only
5. **"How do you cross clock domains?"** — 2-stage synchronizer for single bit, async FIFO for data bus
6. **"What causes metastability?"** — Sampling a signal during its setup/hold window
7. **"blocking vs non-blocking assignment?"** — `=` for combinational (`always_comb`), `<=` for sequential (`always_ff`)
8. **"What is a latch and how do you avoid it?"** — Incomplete assignment in combinational logic. Fix: always assign default values

---

## SystemVerilog Style Tips for Interview

```systemverilog
// Always use these for synthesizable RTL:
always_comb    // combinational logic (not always @*)
always_ff      // sequential logic (not always @(posedge clk))
logic          // instead of reg/wire
typedef enum   // for FSM states

// Always include:
default: next_state = IDLE;   // in case statements — prevents latches
```
