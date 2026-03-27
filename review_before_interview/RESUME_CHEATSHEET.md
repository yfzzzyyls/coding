# Resume Review Cheatsheet

Prepare to explain every bullet point on your resume. For each item, know:
- What you did (1-2 sentence summary)
- Why it mattered (impact)
- Technical depth (be ready for follow-ups)
- Challenges you faced

---

## RISC-V SoC Design with Custom Accelerator (11/2025 - 01/2026)

### Frontend Design

**"Designed a quaternion-based pose estimation accelerator integrated with a RISC-V core, achieving 1.8x speedup"**

- What: Custom hardware block that computes quaternion rotations (used in IMU/robotics for 3D orientation). Replaced slow software trig functions (sin/cos) with dedicated hardware.
- Be ready to explain:
  - What is a quaternion? — 4-component number (w, x, y, z) representing 3D rotation. Avoids gimbal lock vs Euler angles.
  - Why is hardware CORDIC faster? — CORDIC computes trig via 16 iterations of shift+add (binary search for rotation angle). CPU does them one at a time (~80 cycles). Hardware pipelines all 16 stages — one result per cycle after pipeline fills. Dedicated parallel hardware vs. single shared ALU.
  - Why can't the compiler do this? — Compiler can only emit instructions the CPU supports. PicoRV32 (RV32IM) has no FPU, one ALU, no parallelism. Compiler optimizes instruction order, but can't create new hardware.
  - How did you measure the speedup? — VCS cycle count comparison: firmware-only vs firmware+accelerator.

**"Integrated processor, accelerator, and memory mapped interconnect with custom bus address decode"**

- What: Connected the accelerator to the RISC-V core via memory-mapped I/O. CPU writes operands to specific addresses, accelerator computes, CPU reads result.
- Be ready to explain:
  - Memory-mapped I/O: Peripherals appear as addresses. CPU uses normal load/store — no special instructions.
  - Bus: Single-master (PicoRV32 native interface). Address decoder routes by upper bits → peripheral. Read mux returns data back to CPU.
  - CPU-accelerator flow: Write operands → poll status register → read result. Polling (not interrupts) because single-task, no OS, fast hardware compute.

**"Implemented the frontend flow spanning C firmware/toolchain integration, SystemVerilog RTL development, and Synopsys Design Compiler synthesis"**

- What: Full flow from writing C code that runs on the RISC-V core, to the RTL that implements the hardware, to synthesizing it into gates.
- Be ready to explain:
  - What is toolchain integration? — Set up RISC-V GCC cross-compiler (riscv32-unknown-elf-gcc) targeting RV32IM. Write linker script to map code/data to the correct SRAM addresses. Compile C → .elf → .hex.
  - How does RTL run the firmware? — `$readmemh("firmware.hex", sram_array)` preloads the hex into SRAM at simulation start. VCS simulates the CPU cycle by cycle — PicoRV32 fetches instructions from SRAM and executes like real silicon. That's also how cycle counts are measured.
  - How did firmware interact with the accelerator? — Write operands via volatile pointers, poll status register, read result. All compile to RISC-V load/store instructions.
  - What is Design Compiler? — Synopsys tool that converts RTL to gate-level netlist using a technology library.
  - What is the frontend flow? — RTL coding → functional simulation → synthesis → STA.

**"Developed SDC constraints and drove STA-based setup and hold timing closure"**

- What: Wrote timing constraints (clock period, input/output delays) and fixed timing violations.
- Be ready to explain:
  - What is SDC? — Synopsys Design Constraints. You tell the tool: clock period, clock uncertainty (jitter/skew), and I/O delays.
  - Setup violation: Data arrives too late before clock edge. Fix: shorten path, add pipeline stage, or slow the clock.
  - Hold violation: Data changes too soon after clock edge. Fix: add delay buffers. Hold is independent of clock period.
  - What is slack? — `Slack = required time - arrival time`. Positive = met, negative = violation. Worst slack = critical path = bottleneck limiting max frequency.
  - I/O delays vs setup/hold: I/O delays are what YOU set in SDC — describe the external world outside your chip (when data arrives/leaves the pins). Setup/hold are flip-flop properties from the .lib — the tool reads them automatically. Inside the chip, no I/O delays needed — tool computes all gate delays from the netlist.
  - Know your numbers: what clock period you targeted, worst slack, and where the critical path was.

### Physical Implementation

**"Implemented entire backend flow using Cadence Innovus in TSMC 16nm"**

- What: Took the gate-level netlist from synthesis and produced the physical layout (GDSII).
- Be ready to explain:
  - Backend flow: Floorplan → Power planning → Placement → CTS (Clock Tree Synthesis) → Routing → Sign-off
  - What is floorplanning? — Defining die size, placing macros (SRAM, I/O pads), defining power rings.
  - What is CTS? — Building a balanced clock distribution tree so all flip-flops see the clock at roughly the same time (minimize skew).
  - TSMC 16nm — FinFET technology. What's a FinFET? 3D transistor structure with better leakage control vs planar.

#### Your Step-by-Step Innovus Flow (know this cold)

```
Step 1: Setup & Design Import
  - Inputs: gate-level netlist (soc_top.v), LEF files (tech + stdcell + SRAM), timing libs (.lib), SDC constraints
  - MMMC (Multi-Mode Multi-Corner): set up library sets, RC corners, delay corners, analysis views
  - Commands: init_design, set_analysis_view -setup {view_typ} -hold {view_typ}
  - Global net connections: VDD/VSS power nets

Step 2: Floorplanning
  - floorPlan -site core -r 1.0 0.30 50 50 50 50
    → aspect ratio 1.0, utilization 30%, core margins 50µm on all sides
  - Low utilization (30%) gives maximum routing space → easier to close DRC
  - SRAM macro placement: placeInstance, then set .place_status fixed
  - Macro halos: keep-out regions around macros to prevent standard cell crowding

Step 3: Power Planning
  - Power rings around core (typically on top metal layers, e.g., Metal 9/10)
  - Power stripes across the design (e.g., on Metal 6) for uniform IR drop
  - Connect global nets: VDD, VSS to all standard cells and macros
  - sroute: special route for power/ground connections to cell rails

Step 4: Placement
  - Well taps: prevent latch-up by tying N-well/P-well to proper potential
  - End caps: boundary cells at row ends to satisfy DRC rules
  - place_design: place standard cells
  - optDesign -preCTS: optimize timing before clock tree (setup only)
  - timeDesign -preCTS: generate pre-CTS timing report

Step 5: Clock Tree Synthesis (CTS)
  - create_ccopt_clock_tree_spec: auto-generate clock tree spec from SDC
  - ccopt_design: build the clock tree (inserts buffers/inverters for balanced skew)
  - optDesign -postCTS: optimize with real clock tree delays (setup + hold)
  - timeDesign -postCTS: verify timing with actual clock skew

Step 6: Routing
  - routeDesign: global routing then detailed routing of all signal nets
  - ecoRoute -fix_drc: auto-fix DRC violations from routing
  - Manual DRC fixes if needed: adjust via placement, widen wires, add spacing
  - setExtractRCMode: parasitic extraction for accurate timing

Step 7: Signoff & Export
  - addMetalFill: fill empty metal space (density rules, antenna rules)
  - verify_drc: run DRC check → target 0 violations
  - LVS: verify layout matches netlist
  - timeDesign -signoff: final STA with extracted parasitics
  - report_timing: check setup/hold slack
  - streamOut: export GDSII for tape-out
```

**Interview talking points for your flow**:
- "Why 30% utilization?" — Course project setting for maximum DRC cleanness. Production designs are typically 70-80%.
- "What metal stack?" — TSMC 16nm 11-metal stack (N16ADFP_APR_Innovus_11M)
- "How did you handle SRAM?" — Placed as hard macro with fixed placement status and halo keep-out
- "What timing corner?" — Typical corner (tt0p8v25c) — 0.8V, 25°C. Production would add worst-case (ss) and best-case (ff).
- "SRAM debugging story" — Initial SRAM placement caused via connection failures in power planning. The auto-placer doesn't optimize for power pin alignment — it optimizes timing/congestion/area. Stripped SRAM out, verified clean baseline, re-integrated with correct placement so power pins aligned with grid. Got to 0 DRC/LVS.

**Metal layer stack (11 metals in TSMC 16nm)**:
- Transistors at bottom (below all metal). All metal layers are for wiring.
- Lower metals (M1-M3): local routing — thin, dense, fine pitch
- Middle metals (M4-M6): signal routing — longer connections
- Upper metals (M7-M10): power grid (VDD/VSS rings/stripes), clock
- Layers alternate direction: M1 horizontal, M2 vertical, M3 horizontal...

**"Delivered with 0 DRC and 0 LVS violations"**

- What: Clean design — no design rule violations and layout matches the schematic exactly.
- Be ready to explain:
  - DRC: Design Rule Check — verifies metal spacing, width, via enclosure, etc. per foundry rules.
  - LVS: Layout vs Schematic — verifies the physical layout produces the same circuit as the netlist.
  - What tool? Innovus built-in verify_drc, or Calibre (Siemens/Mentor) for signoff.
  - Metal fill: Required for density rules — addMetalFill on M1-M6, timing-aware to avoid coupling.
  - Common DRC violations: metal spacing (wires too close), min width, via enclosure (not enough metal around via), shorts (two nets touching), antenna (long wire damages gate oxide).
  - Manual DRC fix flow: delete bad wire segment (editDelete) → redraw with editRoute on same or different layer → tool auto-inserts vias when you switch layers → GUI shows legal/illegal via placement in real time → re-run verify_drc to confirm fix didn't create new violations.

### Verification

**"Ran lint, CDC, and low power UPF checks using Synopsys VC Static and Cadence JasperGold"**

- Note: These were course verification scripts — ran them and reviewed reports, didn't write from scratch.
- Be ready to explain at high level:
  - Lint: catches RTL coding mistakes — undriven signals, width mismatches, missing resets. Like compiler warnings.
  - CDC: finds signals crossing clock domains without proper synchronizers. My design was single clock domain so no CDC issues. But know the concept: single bit → 2-stage synchronizer, multi-bit data → async FIFO.
  - Async FIFO for CDC: uses Gray code pointers (only 1 bit changes at a time → safe to synchronize, never get corrupted value). Used anywhere two clock domains meet — inside same chip or between chips.
  - UPF: specifies power domains — which blocks can be powered on/off independently. Power domain = on/off switch (e.g., shut off GPU when idle). Needs isolation cells (clamp outputs when off) and retention registers (save state before power-off). Different from voltage domains (different voltages, need level shifters).
  - LEC: proves synthesis didn't break logic — compares RTL vs netlist mathematically for ALL inputs, not just simulated cases.
  - JasperGold: formal verification — proves properties mathematically, not by simulation.

---

## Digital Design Projects (02/2025 - 12/2025)

**"MBIST: Implemented a modular MBIST architecture for a 64x8 SRAM"**

- What: Memory Built-In Self-Test — hardware that tests SRAM for manufacturing defects without external equipment.
- How it works: A mux switches SRAM access from CPU (normal mode) to MBIST controller (test mode). Controller is just an FSM + address counter + data generator + comparator. Address counter visits every cell (0,1,2,...255). Data generator outputs simple patterns (all 0s, all 1s). Comparator checks: read back == expected? If mismatch → defective cell.
- March C- flow: write 0 to all → read 0/write 1 ascending → read 1/write 0 ascending → same descending → read all 0s. Tests that every cell can hold 0 and 1. Ascending/descending order catches coupling faults (writing one cell accidentally flips a neighbor).
- Why MBIST? — SRAM is buried inside chip, no pin access to individual cells. BIST wraps test logic around it for production testing.

**"SPI: Implemented a 44-bit SPI interface with FSM-based receive/access/response control"**

- What: Serial Peripheral Interface — synchronous serial protocol (master-slave, 4 wires).
- How it works: CS goes low (select slave) → SCLK toggles (one bit per edge) → MOSI shifts data out from master, MISO shifts data back from slave simultaneously (full-duplex) → CS goes high (done). It's basically a shift register controlled by the clock.
- 4 signals: SCLK (clock), MOSI (master out), MISO (master in), CS (chip select, active low)
- SPI modes: Mode 0 (most common) — clock idles low, sample on rising edge. Other 3 modes just change idle polarity and sample edge.
- Your 44-bit FSM: Receive (shift in 44 bits = opcode + address + data) → Access (decode command, read/write memory) → Response (shift result back out on MISO).
- SPI vs I2C: SPI is faster, full-duplex, but needs more wires (one CS per slave). I2C is 2 wires only, addressed, but slower and half-duplex.

**"SRAM Decoder: Implemented an 8x256 hierarchical decoder"**

- What: Converts an 8-bit address into 1-of-256 wordline select — picks which SRAM row to access.
- How it works: 8 input bits → only 1 of 256 output lines goes high (the selected row).
- Why hierarchical? Flat decoder = 256 AND gates with 8 inputs each — huge and slow. Hierarchical splits address into groups (2+3+3 bits), decodes each group with a small decoder **in parallel**, then ANDs results together. Three small fast decoders running simultaneously instead of one big slow one.
- Transistor sizing: Larger transistors = faster but more area/power. Optimized the critical path transistors.

---

## EDIVA: Differential Egocentric Video Processing for AR (05/2025 - 11/2025)

**Joint research with Meta Reality Labs, submitted to ISCA 2026**

- What: Framework that makes AR video processing faster by reusing results from previous frames instead of recomputing everything.
- Be ready to explain:
  - Differential execution: Only process what CHANGED between frames. If 80% of the scene is the same, skip re-segmenting it.
  - 2.67x lower latency, 3.65x energy reduction — compared to processing every frame from scratch.
  - Gaze-based depth predictor: Uses where the user is looking + IMU data to predict depth, avoiding expensive computation.
  - Accelerator offloading: SLAM and projection on custom SoC (lightweight, fast), heavy segmentation on GPU.

**Likely questions**:
- "What was your specific contribution vs the team?" — Be clear about what YOU did.
- "How did you measure the speedup?" — Simulation? Real hardware? What baselines?
- "What is SLAM?" — Simultaneous Localization and Mapping. Tracking camera position in 3D space.

---

## Distributed Speculative Decoding for LLMs (04/2025 - 11/2025)

**Submitted to ICML 2026**

- What: Built a simulator to optimize how LLMs run across edge devices and cloud servers using speculative decoding.
- Be ready to explain:
  - Speculative decoding: Small "draft" model guesses multiple tokens, large model verifies in one pass. If guesses are right, you get multiple tokens for the cost of one large-model inference.
  - DSD-Sim: Discrete event simulator in C++/Python that models network latency, device compute, batching.
  - AWC (Adaptive Window Control): ML-based scheduler that dynamically adjusts how many tokens the draft model speculates based on workload.
  - 1.1x speedup, 9.7% higher throughput vs fixed speculation length.

**Likely questions**:
- "Why is speculative decoding beneficial?" — Large models are memory-bandwidth bound, not compute bound. Verifying multiple tokens in parallel is nearly free.
- "What benchmarks did you use?" — GSM8K (math), HumanEval (code), CNN/DailyMail (summarization).

---

## Work Experience — RiVAI Technologies (04/2020 - 02/2024)

### Architect Role (10/2023 - 02/2024)

**"Defined microarchitecture for a decoupled frontend and load store unit for custom RISC-V CPU"**

- Be ready to explain:
  - Decoupled frontend: Fetch and decode operate independently from execute. Uses instruction buffer to hide fetch latency.
  - Load Store Unit (LSU): Handles all memory operations. Includes load queue, store queue/buffer, address calculation, cache interface.
  - What decisions did you make? — Pipeline depth, buffer sizes, forwarding paths, miss handling.

#### Decoupled Frontend Deep Dive
- **Concept**: Buffer (queue) between fetch and decode so they run independently. Fetch keeps filling buffer ahead of time even if decode stalls.
- **Your design**: 4-wide superscalar (decode up to 4 instructions/cycle). Instruction buffer sized to hold several cache lines of instructions.
- **Fetch width**: One cache line (32 bytes) per fetch = up to 16 compressed (16-bit) or 8 normal (32-bit) instructions. One fetch gives 2-4 cycles of work for 4-wide decode.
- **Branch misprediction**: Flush instruction buffer (speculatively fetched instructions are wrong), redirect fetch to correct target.
- **Why decouple?**: Hides fetch latency (I-cache misses, branch redirects). Keeps backend fed with instructions → higher IPC.

#### L1 I-Cache Design
- **Your design**: 32KB, 8-way set-associative, 32-byte cache lines, 128 sets.
- **Why 8-way?**: Forced by VIPT (Virtually Indexed, Physically Tagged). With 4KB pages and 32-byte lines: index + offset must fit within 12-bit page offset. 7-bit index + 5-bit offset = 12. Fewer ways → index spills into VPN bits → aliasing.
- **VIPT**: Index cache with virtual address bits (from page offset) while TLB translates in parallel. Compare tags using physical PPN from TLB. Works because page offset is same in VA and PA.
- **Aliasing**: If index includes VPN bits, same physical address can land in different cache sets via different virtual addresses (shared libraries, fork). Two stale copies = broken. 8-way keeps index within page offset → prevents aliasing.
- **Formula**: Max cache size without aliasing = Ways × Page Size. 8 × 4KB = 32KB.
- **No banks**: L1I doesn't need banks — fetch reads one cache line from one PC per cycle. (L1D may need banks for multiple loads/stores per cycle.)
- **Miss penalty**: Stall fetch → request L2 (~10 cycles) → if L2 miss, DRAM (~100 cycles). Instruction buffer helps absorb short misses.

#### Fetch Pipeline Stages
- **F1**: Send PC → index I-cache (pick set) + ITLB (start VPN→PPN) + TAGE branch predictor. All three start in parallel.
- **F2**: ITLB resolves PPN (1 cycle CAM). I-cache reads all 8 ways' tags AND data speculatively from the indexed set.
- **F3**: Compare PPN from ITLB against 8 tags → hit/miss. Mux-select hit way's data (already read in F2, no extra cycle). TAGE prediction also ready → redirect fetch if predicted taken.
- **F4**: Deliver cache line to instruction buffer. Decode can start pulling instructions.
- **Latency vs throughput**: 3-4 cycle latency per fetch, but pipelined → one new cache line per cycle throughput.
- **Speculative data read**: All 8 ways read in F2 before knowing which hit. Costs power but saves a cycle vs reading only the hit way after tag compare. Standard for high-performance cores.
- **Tag and data are separate SRAMs**: Tag SRAM (small, ~20 bits/entry) and Data SRAM (large, 256 bits/entry) accessed with same index in same cycle. Separate so you can read tags only for snoops without touching data array.

#### Branch Prediction in Fetch (BTB + TAGE)
- **BTB (Branch Target Buffer)**: Answers "WHERE does this branch go?" — cache of branch targets, indexed by cache line address. One entry stores multiple branches per cache line (position, target, type).
- **TAGE**: Answers "TAKEN or NOT-TAKEN?" — replaces simple BHT. Multiple tables with geometric history lengths (4,8,16,32,64,128). Longer history match wins. Base table (PC-only, no history) acts as fallback BHT.
- **BTB + TAGE together**: TAGE says taken + BTB gives target → redirect fetch. TAGE says not-taken → fetch next sequential PC.
- **Multiple branches per cache line**: One fetch (32 bytes) can contain several branches. Pre-decoder scans opcode bits to identify branches. TAGE predicts each one. First taken branch → redirect fetch, discard everything after it.

#### Pre-decode on Cache Fill
- **Concept**: When line fills from L2 → L1I, pre-decode it once. Store "is-branch" bits, branch type, and instruction length alongside data in a separate small SRAM.
- **Why**: Pre-decode on fill is off critical path (L2 fill already ~10 cycles). Every subsequent fetch reads pre-decode bits for free — no scanning needed.
- **What it stores**: Is-branch flag, branch type (conditional/unconditional/call/return), instruction length (16-bit or 32-bit for RISC-V C extension). Instruction length helps identify boundaries for 4-wide decode.

#### TAGE Predictor Internals
- **Input**: PC + Global Branch History Register (GHR — shift register of last N branch outcomes, taken=1/not-taken=0).
- **Structure**: Base table (hash of PC only, always matches, fallback) + 4-6 tagged tables with geometric history lengths (e.g., 4,8,16,32,64,128). Each tagged entry has: tag + 2-bit counter + useful bit.
- **Lookup**: Hash(PC, GHR) into ALL tables in parallel. Check tag matches. Longest history match wins → use its 2-bit counter for prediction. If no tagged match → fallback to base table.
- **Update on correct**: Strengthen counter + increment useful bit in matching table.
- **Update on wrong**: Weaken counter + decrement useful bit. Allocate new entry in a longer history table (if short history got it wrong, maybe longer history captures the pattern). Evict entries with useful=0.
- **Why it works**: Simple branches use base table. Complex patterns automatically escalate to longer history tables. Geometric spacing covers all scales efficiently.

#### I-Cache Prefetch
- **Your design**: Next-line prefetch + branch-target prefetch + fetch buffer.
- **Next-line**: Every fetch automatically requests next sequential cache line from L2. Nearly free, covers sequential code.
- **Branch-target**: When TAGE predicts taken, prefetch the target cache line from L2 alongside the next-line prefetch.
- **Fetch buffer**: Prefetched lines go into a small separate buffer, NOT directly into L1I. On L1I miss, check fetch buffer first. If hit → move to L1I (prefetch saved a stall). If never used → silently discard. Avoids polluting L1I with speculative data.

#### ITLB
- **Your design**: ~8-16 entries, fully associative (CAM).
- **Must hit in 1 cycle**: Cache needs PPN from TLB for tag comparison. If ITLB misses, cache tags sit waiting → pipeline stalls.
- **Why fully associative?**: Small structure, can't afford conflict misses on something accessed every fetch. CAM compares all entries in parallel — input is VPN, output is matching PPN. Expensive (comparator per entry) but fine for 8-16 entries.
- **CAM vs RAM**: RAM uses a decoder (address in → data out). CAM uses comparators per entry (data in → which entry matched?). CAM is expensive but enables fully associative lookup.
- **Miss path**: ITLB miss → L2 TLB (~4 cycles, set-associative, 512-1024 entries) → page table walk (~50-100 cycles, up to 3 memory accesses for RISC-V Sv39) → page fault (OS loads from disk, millions of cycles).

#### Load Store Unit (LSU) Deep Dive
- **Structure**: Load Queue (LQ) + Store Queue (SQ) + Address Generation Unit (AGU) + DTLB + L1D cache. ~8-16 entries each queue.
- **Store-to-load forwarding**: Load checks store queue in parallel with L1D. If older store to same address exists in SQ, forward data from SQ instead of cache. Critical for performance.
- **Memory model (TSO)**: Stores drain to cache in program order (FIFO). Loads can execute out of order but must check for ordering violations. TSO is simpler hardware than RVWMO (weak model), store buffer is just a FIFO.
- **Memory disambiguation**: Speculative — loads execute immediately without waiting for older store addresses to resolve. If older store later resolves to same address → memory order violation → replay load and everything younger.
- **Store Queue vs Store Buffer**: SQ holds all in-flight stores (speculative + committed). Store buffer holds only committed stores waiting to drain to L1D. Some designs combine them with a "committed" bit. Speculative stores can be flushed on misprediction; committed stores in STB cannot.
- **Flush/replay**: Not full pipeline flush — flush from offending instruction onward using age tags (ROB index). Broadcast "kill everything younger than X". Redirect fetch to re-execute from that point.
- **Replay ripple effect**: When a load replays, ALL younger instructions that consumed the load's value are also flushed and re-executed. The whole dependency chain rebuilds from scratch. That's why disambiguation misses are costly (10-20+ cycles to refill pipeline).
- **Commit vs drain**: Commit = ROB marks store as non-speculative, frees ROB entry. Drain = store buffer later writes to L1D when cache port available. Two independent events — core moves on after commit, store buffer handles cache timing.
- **Store response from cache**: No data response needed (unlike load), but cache must signal accepted/retry (bank conflict). On write miss with write-allocate, store buffer entry stays occupied until line is fetched from L2 and written — entry not freed until refill completes.
- **Store buffer drain rate**: 1 store per cycle max (1 write port). Not every cycle — blocked by bank conflicts, cache misses, empty buffer.
- **Combined SQ vs separate STB**: Combined (committed bit) is simpler but committed stores block entries needed by new speculative stores. Separate STB frees SQ entries at commit, decouples speculation from retirement. Separate STB requires forwarding to check both SQ and STB.

#### Flush and Fence
- **Branch misprediction**: Immediately kill everything after the branch in ROB. Don't wait. Instructions before branch continue and commit normally.
- **Memory order violation**: Flush from offending load onward. Same mechanism — kill younger instructions by ROB age tag.
- **FENCE (data fence)**: Orders loads/stores. All older stores must drain to cache before younger loads/stores execute. Pipeline drains behind fence — expensive, used sparingly.
- **FENCE.I (instruction fence)**: Syncs I-cache with data writes. After writing new instructions to memory via stores, FENCE.I ensures I-cache sees them (typically invalidates entire I-cache). Needed because I-cache and D-cache are separate — stores go to D-cache, I-cache doesn't know instructions changed.

#### Memory Model (TSO vs RVWMO)
- **TSO (your design)**: Stores seen by other cores in program order. Store buffer drains in FIFO order. Loads can reorder with each other but not past older stores. Simpler hardware.
- **RVWMO (RISC-V weak model)**: Almost everything can reorder. Need explicit FENCE instructions. More aggressive performance but harder to program. Store buffer could drain out of order.

#### L1 D-Cache Design
- **Your design**: 32KB, 8-way (VIPT, same aliasing constraint as L1I), write-back + write-allocate. 1 read port + 1 write port (8T SRAM cells), 4 or 8 banks.
- **Banks**: 32-byte line / 4 banks = 8 bytes per bank. addr[2:0] = byte within bank, addr[4:3] = which bank. Multiple loads/stores access different banks in parallel. Same-bank conflict → one stalls. Cheaper than true multi-port SRAM.
- **Write-back**: Store writes to cache only, sets dirty bit. Writes to memory only on eviction. Fast writes, less bus traffic.
- **Write-allocate**: On write miss, fetch line from L2 into cache first, then write to it. Good for repeated writes to same line.
- **Dirty bit**: Clean (0) when loaded from L2. Set to 1 on store. On eviction: dirty=0 → discard. Dirty=1 → write back to L2 first.
- **L1D vs L1I**: L1D has random access patterns, needs banks/multi-port, handles writes, needs store buffer and forwarding, full coherence protocol.

#### SRAM Port Design
- **6T cell**: 1 port (read OR write). 2 inverters (4T) + 2 access transistors. Standard, most dense.
- **8T cell**: 1R + 1W (read AND write simultaneously). +2T = 1 access transistor + 1 buffer transistor for isolated read port. Buffer isolates storage node so read doesn't disturb concurrent write. ~30% more area than 6T.
- **Each additional port**: +2T per cell + new bitline(s) running full column + new wordline running full row. Wiring congestion is the real scaling problem, not transistor count.
- **Your design**: L1I = 6T (read-only, writes only on fill). L1D = 8T (1R+1W, simultaneous load + store drain). Banks provide additional parallelism without going to 10T+.

#### Vector Renaming — Why Not Practical
- **Problem**: Vector registers are huge. Scalar = 64 bits. Vector (VLEN=256) = 2048 bytes per register. Renaming needs physical register file much larger than architectural: 128 physical × 256 bytes = 32KB per register file — enormous.
- **Also**: Bypass network for forwarding 256-byte results = massive wiring. RAT/ROB/free-list all must handle huge entries.
- **Solution**: Vector unit typically runs in-order (no renaming needed). Scalar core is OoO with renaming, vector unit has its own simpler scheduling or chaining.

#### Cache Coherence vs Memory Consistency
- **Coherence**: Same address — all cores see the same value. Hardware protocol (MESI). Per-address correctness.
- **Consistency**: Different addresses — in what order do other cores see writes. Architecture spec (TSO/RVWMO). System-wide ordering rules.
- **Consistency only matters for multi-core**: Single core always sees its own writes in order. Multi-threaded programmers usually don't worry directly — pthread_mutex/atomic insert the right fences. OS kernel and lock-free library writers care most.

#### MESI Cache Coherence Protocol
- **4 states per cache line**: Modified (only I have it, dirty), Exclusive (only I have it, clean), Shared (multiple caches, clean), Invalid (no valid copy).
- **Key transitions**:
  - Core reads, nobody has it: Invalid → Exclusive
  - Another core also reads: Exclusive → Shared (both cores)
  - Core writes to Shared line: Shared → Modified (writer), Shared → Invalid (all others). Writer sends "invalidate" on bus.
  - Core reads a Modified line in another cache: Modified → Shared (owner writes back dirty data first), Invalid → Shared (requester gets fresh data).
- **Snooping**: All caches watch a shared bus. Every write/read-miss broadcasts on bus, other caches check "do I have this address?" and react. Why tag and data SRAMs are separate — snoop checks tags only, saves power.
- **Snooping vs Directory**: Snooping = broadcast to all (simple, fast, scales to 4-8 cores). Directory = central tracker sends targeted invalidates (scales to 64+ cores, more latency).
- **MOESI**: Adds Owned state — dirty data can be shared without writing back to memory. Owner supplies data on requests. Reduces memory traffic.

#### Memory Consistency Models
- **TSO (your design)**: Stores seen by other cores in program order. If Core 1 sees flag=1, it must see all stores before flag. Store buffer = FIFO. Simpler hardware, fewer fences in software.
- **RVWMO (weak)**: Stores can be reordered. Core 1 might see flag=1 but miss earlier stores. Must insert FENCE explicitly. More hardware flexibility, higher throughput potential. Software must be careful.
- **The flag pattern**: Core 0: store data, FENCE, store flag. Core 1: load flag, FENCE, load data. Without fences under weak model, Core 1 can see flag=1 but data=old.

#### Register File — Not SRAM
- **Difference**: SRAM optimized for density (6T, tiny cells, sense amps). Register file optimized for speed + ports (custom ~20-30T flip-flop cells, direct read, no sense amp).
- **Similar concept**: Both have wordlines (select entry) and bitlines (carry data). But register file is hand-optimized, flip-flop based for robustness.
- **Multi-port scaling problem**: Area grows as ports². Each port adds bitlines (full column) + wordlines (full row). All wires cross each other in 2D layout. More ports = more capacitance = slower reads = can't meet 1-cycle timing.
- **Your core**: ~8-12 read ports + ~4-6 write ports (4-wide, 2 source regs per instruction). Already near practical limit.
- **Workarounds instead of more ports**: Banking (split into banks, stall on conflict), duplication (2 copies with half the read ports each, writes go to both), clustering (each cluster has own register file copy). This is why core width maxes out around 4-6 wide.

**"Collaborated with architecture, design, and performance teams to analyze bottlenecks"**

- Be ready to give a specific example of a bottleneck you found and how you fixed it.

### Performance Modeling Team Lead (05/2021 - 10/2023)

**"Led development of a cycle-accurate C++ performance model for our custom RISC-V core"**

- What: Software model that simulates the CPU cycle-by-cycle, matching RTL behavior but running 100-1000x faster.
- Be ready to explain:
  - What is cycle-accurate? — Every pipeline stage, queue, cache access modeled per cycle. Stalls, bubbles, forwarding all tracked. Instruction enters fetch at cycle N, decode at N+1, etc.
  - Why not just simulate RTL? — RTL simulation is too slow (~1K cycles/sec) for real workloads (Linux, SPEC). Performance model runs ~1M cycles/sec (100-1000x faster).
  - How is this different from Gem5? — Gem5 is general-purpose (ARM, x86, RISC-V). Your model was tuned to your exact microarchitecture — exact pipeline depth, cache config, branch predictor. Faster and more accurate for your specific core.
  - YAML configurable: Architects explore design space quickly — "what if L1 is 64KB? What if TAGE has 8 tables?" Change YAML, rerun, compare IPC. No recompilation.
  - How accurate was it vs RTL? — Know your CPI correlation number (aim for <5% error).

**"Modeled branch prediction, fetch/decode, LSU (including L1 cache), MMU, and vector path"**

- How do you model hardware in C++? — Model tracks timing and metadata only, not actual data values. Example: cache model stores tags, valid/dirty bits, LRU counters per set. Returns hit/miss + latency. Doesn't store the actual cached data.
- Be ready to explain any of these in depth:
  - Branch predictor: TAGE in C++. Hash tables, counters, update logic. Run instruction stream through it, track misprediction rate.
  - L1 cache model: Array of sets, each with N ways. Access = compute set index + tag, check all ways for match, update LRU. Return hit/miss + cycle count.
  - MMU: TLB as small array, page table walker as state machine. Track TLB hit/miss, page walk latency.
  - Vector path: RISC-V V extension. Vector unit modeled in-order (no renaming), track VLEN, element-wise execution latency.

**"Built RTL-model correlation flow using VCS instruction/cycle traces"**

- What: Compared cycle-by-cycle traces from RTL simulation vs performance model to find mismatches.
- Be ready to explain:
  - Flow: Run same program on both RTL (VCS) and perf model. Both produce instruction traces (cycle number + instruction). Diff the traces. Find first divergence point → debug why cycle counts differ.
  - How did you debug mismatches? — Compare instruction retirement order, cycle counts, state transitions at the divergence point.
  - Common causes: Cache miss latency off by 1 cycle, store forwarding edge case not modeled, branch predictor warm-up difference, TLB miss penalty wrong, spec ambiguity.
  - Be ready with a specific example of a mismatch you found and fixed.

**"Ran Linux workloads and SPEC benchmarks to track regressions"**

- SPEC CPU: Industry-standard benchmark suite. SPEC2006/2017. Integer (gcc, mcf) and floating point.
- Metrics tracked: IPC, branch misprediction rate, L1/L2 cache miss rate, TLB miss rate. Any regression between RTL versions → flag it.
- Linux on perf model: Boot from checkpoint (skip billions of boot cycles). Start from representative point in workload. Connects to your embedded SW role — you built the checkpoint tools.

### Embedded Software Engineer (10/2020 - 05/2021)

**"Developed bare-metal RISC-V BSP components (drivers, interrupts, tohost/fromhost)"**

- What: Board Support Package — lowest level software that initializes and controls hardware.

#### Volatile Keyword
- **Purpose**: Tells compiler "this variable can change at any time without C code touching it" — must re-read from memory every access, never cache in register.
- **Without volatile**: Compiler optimizes away repeated reads → polling loops become infinite loops, hardware register changes are missed.
- **Three cases**: (1) Memory-mapped hardware registers, (2) Variables shared with ISR, (3) Variables shared between threads (but volatile alone is NOT enough for threads — need atomics/locks).
- **Interview trap**: "Is volatile enough for multi-threading?" — No. Prevents compiler optimization but doesn't prevent CPU reordering or guarantee atomicity.

#### Drivers (UART example)
- **Init**: Set baud rate (divisor = sys_clock / (baud × 16)), enable TX/RX in control register.
- **Transmit**: Poll TX_FULL status bit → write byte to TX_DATA register. All through volatile pointers.
- **Receive**: Poll RX_READY status bit → read byte from RX_DATA register.
- **Polling vs interrupt-driven**: Polling = simple, CPU stuck in loop. Interrupt-driven = TX/RX interrupts fire, ISR reads/writes to ring buffer, main loop processes at its own pace. Interrupt-driven needs ring buffer + ISR + enable/disable management.

#### RISC-V Interrupt System — 3 Types
- **External interrupt**: Through PLIC (peripherals: UART, SPI, GPIO). mcause = 0x800B.
- **Timer interrupt**: Through CLINT (mtime >= mtimecmp). mcause = 0x8007.
- **Software interrupt**: Through CLINT (IPI — one core signals another via msip register). mcause = 0x8003.

#### PLIC (Platform-Level Interrupt Controller)
- **Purpose**: Collects ALL peripheral interrupts, prioritizes them, presents highest priority to CPU on a single interrupt line.
- **All memory-mapped registers**: Priority per IRQ, enable bits, priority threshold, claim/complete register — CPU controls PLIC through normal load/store, same as any peripheral.
- **Flow**: Peripheral fires → PLIC asserts interrupt line → CPU enters ISR → CPU reads PLIC claim register ("who fired?") → handle it → write PLIC complete register ("I'm done") → PLIC moves to next pending.

#### CLINT (Core-Local Interruptor)
- **Timer**: mtime (always-counting 64-bit counter) + mtimecmp (compare value). When mtime >= mtimecmp → timer interrupt fires. Clear by writing new future value to mtimecmp. Used for RTOS tick, periodic tasks.
- **Software IPI**: msip register (1 bit per core). Core 0 writes 1 to Core 1's msip → Core 1 gets software interrupt. Used for inter-processor communication in multi-core.

#### Interrupt Handling Flow (RISC-V)
- **Setup**: Write ISR address to mtvec CSR. Enable specific interrupts in mie CSR (bit 3=software, bit 7=timer, bit 11=external). Set global enable in mstatus.MIE.
- **When interrupt fires**: CPU hardware saves PC to mepc, cause to mcause, disables interrupts (mstatus.MIE←0), jumps to mtvec.
- **ISR**: Read mcause to determine type. If external → plic_claim() to find which peripheral → handle → plic_complete(). Keep ISR SHORT — read/write hardware, copy data to buffer, set flag. Heavy processing in main loop.
- **Return**: mret instruction restores PC from mepc, re-enables interrupts (mstatus.MIE ← mstatus.MPIE).
- **Key CSRs**: mtvec (handler addr), mie (interrupt enables), mstatus (global enable), mepc (saved PC), mcause (interrupt cause), mtval (trap value).
- **Nested interrupts**: By default disabled (MIE cleared on entry). To allow: save mepc/mcause to stack, re-enable MIE. Must save/restore carefully.

#### tohost/fromhost (Simulation Interface)
- **What**: Memory addresses for firmware ↔ simulator communication. Write 1 to tohost → simulator terminates "pass". Write >1 → "fail". Not real hardware — simulation convention only.
- **How address is assigned**: `volatile uint64_t tohost __attribute__((section(".tohost")))` tells compiler to put variable in custom section ".tohost". Linker script places .tohost section at a specific address (e.g., page-aligned). Simulator reads ELF symbol table to find tohost's address and watches for writes to it.
- **`__attribute__((section(...)))`**: Compiler directive — "put this variable in named section X." Linker script then maps section X to a physical address. At link time, all references to tohost are filled with the real address (relocation).

#### Startup Sequence (start.S / crt0.S)
- **Reset vector → main()**: (1) Set stack pointer (la sp, _stack_top). (2) Set global pointer (RISC-V gp optimization). (3) Clear .bss (memset to 0 — uninitialized globals). (4) Copy .data from ROM to RAM (initialized globals stored in ROM, copied to RAM at boot). (5) Set up trap handler (csrw mtvec, trap_handler). (6) Call main(). (7) If main returns, write to tohost or loop forever.
- **Linker script**: Defines memory layout (ROM/RAM regions), maps sections (.text, .data, .bss, .tohost) to physical addresses. Provides symbols (_stack_top, _bss_start, _bss_end, _data_load) that start.S uses.
- **All symbols** (_stack_top, _bss_start, etc.) come from linker script, resolved at link time.

#### Memory Layout (Embedded)
```
Low addr                                              High addr
┌───────┬────────┬───────┬──────┬───────────────────┬───────┐
│ .text │.rodata │ .data │ .bss │  heap →    ← stack│       │
│ code  │ const  │ init  │ zero │  malloc    locals  │       │
│ FIXED │ FIXED  │ FIXED │FIXED │  GROWS UP  GROWS DN│      │
└───────┴────────┴───────┴──────┴───────────────────┴───────┘
```
- **.text/.rodata/.data/.bss**: Fixed size at link time. Never change during execution. More globals → bigger .data/.bss. More code → bigger .text.
- **Stack**: Grows DOWN. Automatic — function call pushes, return pops. Local variables, return addresses, saved registers.
- **Heap**: Grows UP. Manual — malloc/free. Many bare-metal systems avoid heap entirely (fragmentation risk, non-deterministic timing).
- **Stack overflow on bare-metal**: No MMU guard page → stack silently overwrites heap or .bss → hard-to-debug corruption. Detect with stack canary or MPU.

**"Built Linux snapshot/checkpoint tools"**

- What: Save/restore full system state (CPU registers, memory) to speed up simulation.
- Why: Booting Linux takes billions of cycles. Checkpoint after boot, restore to skip it for every test.

**"Optimized performance-sensitive C kernels using compiler intrinsics"**

- What: C functions that map directly to specific CPU instructions (e.g., RISC-V vector intrinsics).
- Why not assembly? — Intrinsics give instruction-level control while keeping code portable and compiler-optimizable. Compiler still handles register allocation and scheduling.
- Example — vector add with RISC-V V extension: vsetvl sets vector length, vle32 loads vector, vadd adds element-wise, vse32 stores. Processes VLEN/32 elements per iteration instead of 1.

**"Implemented ResNet-50 in C"**

- What: Convolutional neural network, 50 layers deep. Implemented inference (not training) in pure C.
- Be ready to explain:
  - Convolution: Sliding window of weights across input feature map, multiply-accumulate.
  - ResNet's key innovation: Skip/residual connections — add input to output of conv layers to avoid vanishing gradient.
  - How did you handle memory? — Layer-by-layer computation to minimize memory footprint? Tiling?

### Test Engineer (04/2020 - 09/2020)

**"Maintained Python-based C/assembly test generation"**

- What: Scripts that auto-generate test programs to verify CPU correctness.
- Types of tests: Instruction-level (does ADD work?), random (fuzzing), directed (corner cases), compliance (RISC-V spec conformance).

---

## Technical Skills — Be Ready to Discuss

### Tools You Listed
| Tool | What It Does | Your Experience Level |
|------|-------------|----------------------|
| VCS | Synopsys Verilog simulator | Used extensively for RTL sim |
| Verdi | Synopsys waveform debugger | Debug RTL with waveforms |
| Design Compiler | RTL → gate-level synthesis | Frontend flow |
| Cadence Innovus | Place and route (backend) | Full backend flow |
| Cadence Virtuoso | Schematic/layout for analog | Analog projects |
| Calibre | DRC/LVS signoff | Signoff checks |
| Gem5 | CPU architecture simulator | Performance studies |
| QEMU | Fast CPU emulator | Software development |

### Common Technical Questions About Your Skills
- "Difference between Gem5 and your C++ performance model?" — Gem5 is general-purpose, configurable but slow. Custom model is tuned to your specific microarchitecture, faster, more accurate.
- "What RISC-V extensions did your core support?" — RV64IMAFDC? V extension?
- "What was your core's pipeline depth?" — Know the answer for your specific design.

---

## Behavioral Questions — STAR Format

For each, prepare a story using: **Situation → Task → Action → Result**

1. **"Tell me about a challenging technical problem you solved"**
   - Suggestion: RTL-model correlation debugging, or a tricky timing closure issue.

2. **"Tell me about a time you led a team"**
   - Your performance modeling team lead role (2.5 years).

3. **"Tell me about a disagreement with a colleague"**
   - Architecture decisions at RiVAI — microarchitecture tradeoffs.

4. **"Why are you leaving your current role / Why this company?"**
   - Prepare a genuine, positive answer.

5. **"Walk me through your most impactful project"**
   - RISC-V SoC or the performance model — quantifiable impact.

---

## Quick Resume Stats to Remember

- 4 years at RiVAI: Test → Embedded SW → Perf Modeling Lead → Architect
- 2 papers submitted: ISCA 2026 (EDIVA/Meta), ICML 2026 (DSD-Sim)
- RISC-V SoC: Full RTL-to-GDSII in TSMC 16nm, 0 DRC/LVS violations
- 1.8x accelerator speedup, 2.67x AR latency reduction, 1.1x LLM throughput gain
- GPA: 4.0/4.0 at NYU
