# RTL Design + Architecture Interview Prep Plan (3 Weeks)

## Context
Targeting **RTL Design Engineer** and **CPU/IP Architect** roles at big tech (Apple, AMD, NVIDIA, Intel, Qualcomm). Full-time study (6-8 hrs/day). Graduating May 2026.

**Strengths**: 4 years RISC-V CPU architecture + perf modeling, strong architecture conceptual knowledge, basic RTL fundamentals done (modules 01-08 in `/home/fy2243/coding/rtl/`).

**Critical gaps**: No CDC, no async FIFO, no bus protocols (AXI), no pipeline with hazards, no UART/SPI from scratch, no arbiter, no SRAM design. Architecture knowledge needs interview-level depth and fluency.

**What this plan does NOT cover (intentionally)**: UVM/verification, low power design, formal verification, generic LeetCode (DP/graphs), PCIe/DDR protocols, GPU architecture, SystemC/HLS, physical design tool flow, C/C++ coding practice.

---

## All Work Goes In: `/home/fy2243/coding/design_and_perf/`

```
design_and_perf/
├── STUDY_PLAN.md
├── 09_clock_domain_crossing/    (CDC syncs, async FIFO, glitch-free clock mux)
├── 10_bus_protocols_handshaking/ (AXI-Lite, UART, SPI, credit-based flow)
├── 11_pipeline_design/          (pipeline with hazards, skid buffer)
├── 13_arbiters/                 (fixed priority, round-robin, weighted RR)
├── 14_sram_memory/              (SRAM design, dual-port RAM, register file)
├── 15_clock_generation/         (clock divider odd divide 50% duty)
└── architecture_notes/          (Q&A pairs for architecture topics 17-28)
```

---

## Complete Topic List (28 Topics)

### RTL Modules (16) — Code + Testbench

| # | Topic | Folder | Priority |
|---|-------|--------|----------|
| 1 | CDC — 2-FF synchronizer, pulse synchronizer | `09_clock_domain_crossing/` | **Critical** |
| 2 | Async FIFO — gray code, dual-clock, full/empty | `09_clock_domain_crossing/` | **Critical** |
| 3 | Round-robin arbiter — rotating priority | `13_arbiters/` | **Critical** |
| 4 | AXI-Lite slave — valid/ready handshake, register bank | `10_bus_protocols_handshaking/` | High |
| 5 | SRAM design — cell, decoder, sense amp, banking, pipelined copy | `14_sram_memory/` | High |
| 6 | UART TX/RX — baud divider, FSM, shift register | `10_bus_protocols_handshaking/` | High |
| 7 | Pipeline stage with hazards — forwarding, stall, flush | `11_pipeline_design/` | High |
| 8 | Skid buffer — valid/ready pipe stage with backpressure | `11_pipeline_design/` | High |
| 9 | Clock divider — odd divide with 50% duty cycle | `15_clock_generation/` | High |
| 10 | SPI master — clock divider, CPOL/CPHA, shift register | `10_bus_protocols_handshaking/` | Medium |
| 11 | Glitch-free clock mux — safe clock switching | `09_clock_domain_crossing/` | Medium |
| 12 | Dual-port RAM — parameterized, synchronous | `14_sram_memory/` | Medium |
| 13 | Fixed priority arbiter | `13_arbiters/` | Medium |
| 14 | Credit-based flow control | `10_bus_protocols_handshaking/` | Medium |
| 15 | Weighted round-robin arbiter | `13_arbiters/` | Low |
| 16 | Register file — multi-read/write | `14_sram_memory/` | Low |

### Architecture Concepts (12) — Notes + Q&A Pairs

| # | Topic | Priority |
|---|-------|----------|
| 17 | Cache hierarchy — VIPT math, MSHRs, prefetch, inclusion policies | High |
| 18 | Coherence — MESI/MOESI full state transitions, snooping vs directory | High |
| 19 | OoO execution — rename, ROB, reservation stations, LSQ | High |
| 20 | Branch prediction — BTB, RAS, TAGE internals, recovery | High |
| 21 | Memory consistency — SC vs TSO vs relaxed, fences | High |
| 22 | Virtual memory / TLB — page walk, TLB hierarchy, hugepages | High |
| 23 | Performance analysis — CPI stacking, Amdahl's Law, Little's Law, napkin math | Medium |
| 24 | Timing — setup/hold, critical path, clock skew, how to fix violations | **Critical** |
| 25 | Physical design — congestion, IR drop, antenna, floorplan issues | Medium |
| 26 | Speculative execution + Spectre — concept level | Medium |
| 27 | SMT — how threads share pipeline resources | Low |
| 28 | Sync vs async reset, latch vs FF, FSM encoding tradeoffs | **Critical** |

### Dependency Order (learn fundamentals before modules that use them)
```
CDC fundamentals (#1) → Async FIFO (#2) → Glitch-free clock mux (#11)
Valid/Ready concept → AXI-Lite (#4) → Skid buffer (#8) → Credit flow (#14)
FSM + shift register (already done 05-07) → UART (#6) → SPI (#10)
Pipeline concept → Pipeline with hazards (#7) → Skid buffer (#8)
Fixed priority arbiter (#13) → Round-robin (#3) → Weighted RR (#15)
SRAM cell basics (#5) → Dual-port RAM (#12) → Register file (#16)
```

---

## Daily Structure
- **Morning (3-4 hrs)**: Primary — learn concept, then code RTL module + testbench
- **Afternoon (2-3 hrs)**: Architecture concept study OR additional RTL module
- **Evening (1 hr)**: Spaced repetition — re-code a previous module from scratch, closed-book
- **Mastery test**: Can you code it on a blank screen in 45 min with no reference? If not, it's not learned.

---

## WEEK 1: CDC, Async FIFO, Arbiters, Pipeline

### Day 1: CDC Fundamentals + Timing/Reset Concepts
- **AM**: Study metastability, MTBF, why 2-FF works. Code `sync_2ff.sv` + `pulse_sync.sv` with testbenches (two independent clocks)
- **PM**: Study gray code (binary↔gray conversion, why only 1 bit changes). Then **1 hr quick-hit concepts**: setup/hold violations (#24 partial — what happens, how to fix), sync vs async reset (#28 partial — tradeoffs, reset synchronizer), latch vs FF (when latches are OK)
- **EVE**: Re-code `sync_2ff.sv` closed-book

### Day 2: Async FIFO Part 1
- **AM**: Study async FIFO architecture (Cummings SNUG 2002). Begin coding `async_fifo.sv` — gray-coded pointers, dual-port RAM, full/empty generation
- **PM**: Continue coding + testbench (independent clk_wr/clk_rd, fill to full, drain to empty)
- **EVE**: Draw async FIFO block diagram from memory

### Day 3: Async FIFO Part 2 + FIFO Sizing
- **AM**: Harden testbench (randomized enables, assertions). **Re-code entire `async_fifo.sv` from scratch, timed 60 min**
- **PM**: Study FIFO sizing formula: `depth >= burst_length × (1 - f_read/f_write)`. When to use async FIFO vs handshake vs sync FIFO
- **EVE**: Review CDC concepts from Day 1

### Day 4: Arbiters
- **AM**: Study arbiter types. Code `arbiter_fixed_priority.sv` + `arbiter_round_robin.sv` (rotating mask approach) with testbenches
- **PM**: Architecture — Cache hierarchy intro (#17 partial): VIPT aliasing math, why VIPT is standard, inclusion policies. Write 5 cache Q&A pairs. (Reset/latch/FSM encoding covered Day 1)
- **EVE**: Re-code `arbiter_round_robin.sv` closed-book 45 min. Re-code async FIFO (spaced rep)

### Day 5: Pipeline with Hazards
- **AM**: Code `pipeline_3stage.sv` — Fetch/Execute/Writeback, 8 registers, RAW detection, EX-EX and WB-EX forwarding, load-use stall
- **PM**: Architecture — WAR/WAW hazards in OoO, branch handling, cache miss interaction with pipeline (#19 partial). Write 5 pipeline Q&A pairs
- **EVE**: Draw forwarding mux diagram from memory. Re-code arbiter (spaced rep)

### Day 6: AXI-Lite Slave + UART TX
- **AM**: Study AXI-Lite protocol (5 channels, VALID/READY rules). Code `axi_lite_slave.sv` — register bank, proper handshaking
- **PM**: Code `uart_tx.sv` — baud divider, FSM (IDLE→START→DATA→STOP), `tx_busy` flag
- **EVE**: Re-code async FIFO (spaced rep, target 50 min)

### Day 7: Week 1 Consolidation + Clock Divider
- **AM**: Code `clock_divider.sv` — odd divide with 50% duty cycle (#9). Code `dual_port_ram.sv` (#12)
- **PM**: **Re-code from scratch (pick 2, timed)**: async FIFO (45 min), arbiter (30 min)
- **EVE**: Self-assessment — which modules need more practice?

---

## WEEK 2: Architecture Depth + SRAM + SPI + More RTL

### Day 8: Cache Hierarchy Deep-Dive
- **AM**: Architecture — VIPT aliasing math, inclusion policies (inclusive/exclusive/NINE), replacement (LRU/RRIP), MSHRs, prefetching (#17). Write 10 cache Q&A pairs
- **PM**: Architecture — Virtual memory / TLB deep-dive — page table walk (Sv39, 3 levels), TLB hierarchy, hugepages (#22)
- **EVE**: Re-code UART TX (spaced rep)

### Day 9: Coherence + SPI
- **AM**: Architecture — MESI full state diagram (draw every transition), MOESI (Owned state), snooping vs directory vs snoop filter, false sharing (#18). Write 8 Q&A pairs
- **PM**: Code `spi_master.sv` — FSM-based, 8-bit transfer, clock divider, CPOL/CPHA mode 0 (#10)
- **EVE**: Re-code arbiter (spaced rep). Review cache Q&A

### Day 10: OoO Execution + SRAM Design
- **AM**: Architecture — rename/dispatch/issue/execute/complete/retire, register renaming walkthrough, ROB, reservation stations vs centralized scheduler, LSQ (#19)
- **PM**: Study SRAM design — 6T/8T cell, read/write path, decoder, sense amp, banking, column mux. Code `sram_controller.sv` (#5)
- **EVE**: Re-code AXI-Lite slave (spaced rep)

### Day 11: Branch Prediction + Memory Consistency
- **AM 2hrs**: Branch prediction — BTB, RAS, TAGE internals, recovery on mispredict (#20)
- **AM 2hrs**: Memory consistency — SC vs TSO vs relaxed, store buffer, fences, litmus tests (#21)
- **PM**: Code `credit_counter.sv` (#14) + code `glitch_free_clock_mux.sv` (#11)
- **EVE**: Re-code async FIFO (target 40 min)

### Day 12: Performance Analysis + Timing
- **AM**: Architecture — Amdahl's Law examples, CPI stacking, Little's Law in HW, bandwidth/latency napkin math (#23). Practice 5 calculation problems
- **PM**: Timing concepts — setup/hold (what happens on violation), critical path equation, clock skew, multi-cycle/false paths (#24). Physical design concepts — congestion, IR drop, antenna (#25)
- **EVE**: Re-code SPI master (spaced rep)

### Day 13: Mock Interview Day #1
- **AM RTL mocks** (50 min each, blank file, no reference):
  - Mock 1: Async FIFO
  - Mock 2: Round-robin arbiter
  - Mock 3: UART RX (new — detect start bit, sample mid-bit, shift in 8 bits). Code `uart_rx.sv`
- **PM Architecture mock** (5-10 min each, answer aloud):
  1. Walk through L1 miss → L2 hit. Coherence transactions?
  2. Estimate avg memory access time given miss rates + latencies
  3. Snooping vs directory — when to use each?
  4. Walk through ADD then MUL through OoO pipeline
  5. Store-to-load forwarding? Corner cases?
  6. Design an L1 data cache from scratch (open-ended, talk 15+ min)
- **EVE**: Score each mock. Identify weakest areas for Week 3

### Day 14: Rest + Light Review
- **AM 2hrs only**: Review architecture Q&A pairs. Re-code 1 weakest RTL module
- **PM**: **REST**
- **EVE**: Plan Week 3 focus based on Day 13 gaps

---

## WEEK 3: Polish, Advanced Topics, Mock Interviews

### Day 15: Advanced RTL + Spectre
- **AM**: Code `arbiter_weighted_rr.sv` (#15). Code `pipe_stage.sv` — skid buffer with backpressure (#8)
- **PM**: Architecture — Speculative execution + Spectre concept (#26). SMT — how threads share resources (#27)
- **EVE**: Re-code pipeline 3-stage (spaced rep)

### Day 16: Register File + Architecture Deep Cuts
- **AM**: Code `register_file.sv` — multi-read/write (#16). Study register file design tradeoffs (banking, duplication)
- **PM**: Architecture — Precise exceptions in OoO (how ROB enables them), DVFS concept, diminishing returns of wider issue
- **EVE**: Re-code AXI-Lite slave (spaced rep)

### Day 17: Full Mock Interview Day #2
- **AM RTL mocks**: SPI slave (new twist), parameterized pipeline chain with backpressure
- **AM cont Architecture mock**: "Design an L1 data cache from scratch" (talk 20 min fluently). "Design an SRAM from transistor to controller"
- **PM**: Behavioral interview prep — STAR stories for: hardest technical problem, team leadership, design tradeoff
- **EVE**: Gap analysis. Score each mock

### Day 18: Gap Fill + Hard Questions
- **AM**: Attack weakest areas from Day 17 mocks
- **PM**: Hard interview questions:
  1. Write-back buffer vs store buffer
  2. Hardware vs software TLB miss handling
  3. Spectre/speculative execution mitigations
  4. "IPC dropped from 3.2 to 2.1 — how do you debug?"
  5. SRAM: move L words with 1R1W port — how many cycles? (L+2)
- **EVE**: Re-code UART RX (spaced rep)

### Day 19: Company-Specific Prep + Speed Runs
- **AM**: Research target companies — Apple (deep arch, clean RTL), AMD (MOESI, multi-core), NVIDIA (CDC, throughput), Intel (TSO, pipeline), Qualcomm (power-perf)
- **PM RTL speed runs** (closed-book, timed):
  - Async FIFO: target 35 min
  - Arbiter: target 20 min
  - UART TX: target 25 min
  - Pipe stage: target 20 min
  - Clock divider: target 15 min
- **EVE**: Review all architecture Q&A pairs

### Day 20: Final Full Mock Simulation
- **Round 1 (60 min)**: RTL coding — new problem (programmable timer, or FIFO variant)
- **Round 2 (60 min)**: Architecture discussion — open-ended, practice talking fluently 10-15 min per topic
- **Round 3 (45 min)**: Design discussion — "design a memory controller" or "design a bus arbiter for multi-master system"
- **PM**: Update cheatsheet with everything learned. Review end-to-end
- **EVE**: REST

### Day 21: Light Review + Mental Prep
- 2-3 hrs max: skim cheatsheet, re-read Q&A pairs
- Do NOT learn anything new. Rest.

---

## Spaced Repetition Schedule

| Module | Built | Re-code 1 | Re-code 2 |
|--------|-------|-----------|-----------|
| Async FIFO | Day 2 | Day 4 eve | Day 11 eve |
| RR Arbiter | Day 4 | Day 6 eve | Day 13 mock |
| Pipeline | Day 5 | Day 7 eve | Day 15 eve |
| AXI-Lite | Day 6 | Day 10 eve | Day 16 eve |
| UART TX | Day 6 | Day 8 eve | Day 13 mock |
| Clock Divider | Day 7 | Day 9 eve | Day 19 speed |
| SPI Master | Day 9 | Day 12 eve | Day 17 mock |
| SRAM | Day 10 | Day 13 eve | Day 18 |
| Skid Buffer | Day 15 | Day 17 mock | Day 19 speed |

## Verification
- Each RTL module must compile and simulate (VCS or equivalent)
- Mastery = re-code from blank file in under 45 min, closed-book
- Architecture = answer aloud fluently for 5-10 min per topic without notes
- All files in `/home/fy2243/coding/design_and_perf/`
