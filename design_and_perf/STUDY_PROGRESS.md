# Study Progress & Review Q&A

## Day 1: CDC Fundamentals (2026-03-31)

### Modules Completed
- `sync_2ff.sv` + testbench — compiled, simulated
- `pulse_sync.sv` + testbench — compiled, simulated
- `async_fifo.sv` + testbench — compiled, simulated (ahead of schedule, Day 2-3 material)

### Quiz Q&A (Review Before Interview)

**Q1. Why does crossing clock domains cause metastability?**
The input signal transitions during the receiver FF's setup/hold window. The FF can't resolve to 0 or 1, so the output floats at an intermediate voltage for an unpredictable time. It's a timing violation at the receiver FF — the two clocks have no phase relationship, so eventually the input will change right at the receiver's clock edge.

**Q2. 2-FF synchronizer: latency? Why not multi-bit?**
- Latency: **2 receiver clock cycles** (one FF to absorb metastability, second FF to produce a clean output).
- Multi-bit fails because each bit resolves through its own synchronizer independently. Going from `01 -> 10`, the receiver might see `00` or `11` for one cycle — bits arrive **incoherently**. Use async FIFO for multi-bit CDC.

**Q3. clk_a = 500MHz, clk_b = 50MHz, 1-cycle pulse on clk_a. Can 2-FF on clk_b catch it?**
No. The pulse is 2ns (one clk_a cycle), but clk_b's period is 20ns. The pulse is gone before clk_b even gets one rising edge. Need a **pulse synchronizer** — convert pulse to toggle (persistent level change), synchronize the level, then regenerate pulse via XOR edge detect.

**Q4. Why does the pulse synchronizer use a toggle FF?**
A short pulse might vanish before the slow receiver clock can sample it. The toggle FF converts each pulse into a **persistent level change** — the level stays flipped indefinitely until the next pulse. This gives the slow receiver's 2-FF chain all the time it needs to capture it. On the receiver side, XOR of current vs previous synchronized value detects the level transition and regenerates a 1-cycle pulse.

**Q5. Async FIFO: why is gray code full detection "top 2 bits inverted, rest same" instead of just MSB like binary?**
In binary full detection: MSB different + rest same (simple — write pointer wrapped once past read pointer).
In gray code, the wrap-around pattern affects 2 bits. Consider 4-bit gray code (3-bit address, depth=8):

| Binary | Gray |
|--------|------|
| 0000   | 0000 |
| 0001   | 0001 |
| ...    | ...  |
| 0111   | 0100 |
| 1000   | 1100 |  <-- binary flips 1 bit (MSB), gray flips **2 bits** (bits 3 and 2)
| 1001   | 1101 |
| ...    | ...  |
| 1111   | 1000 |

At the binary midpoint (0111 -> 1000), binary changes only the MSB. But gray code changes from `0100` to `1100` — **both bit 3 and bit 2 flip**. So to detect that the write pointer has wrapped exactly once past the read pointer, you must check: top 2 bits inverted + remaining bits equal.

**Q6. When to use 2-FF vs pulse synchronizer vs async FIFO?**
- **2-FF synchronizer**: single-bit signal that stays stable for >= 2 receiver clock cycles (e.g., slow-to-fast, or level signals like status flags).
- **Pulse synchronizer**: single-bit pulse that might be too short for the receiver to catch (e.g., fast-to-slow, or any pulse shorter than 2 receiver cycles).
- **Async FIFO**: multi-bit data transfer across clock domains. Gray-coded pointers ensure only 1 bit changes at a time for safe synchronization.

**Q7. What is the minimum hold time for a 2-FF synchronizer input?**
The input must be stable for at least **2 receiver clock cycles**. This ensures both FFs in the chain get a chance to sample the stable value. If the signal changes faster than this, transitions can be missed entirely.

---

### PM Topics: Setup/Hold, Reset, Latch vs FF

#### Setup & Hold Violations

**Q8. Setup violation: you increase the clock period. Fixed? What about hold violation — does increasing clock period fix that?**
- Setup: **Yes.** Setup slack = clock_period - Tclk-to-q - Tcomb - Tsu. More period → more slack.
- Hold: **No.** Hold slack = Tclk-to-q + Tcomb - Th. No clock period term — frequency-independent.

**Q9. A path FF_A → 3 levels of AND gates → FF_B has a setup violation. Two fixes without changing the clock?**
1. **Pipeline it**: insert a FF in the middle, splitting 3 levels into 2 stages (trades 1 cycle latency).
2. **Restructure logic**: tree of 2-input ANDs reduces depth from 3 to 2 levels.
3. (Bonus) **Use faster cells**: swap HVT → LVT (more leakage, but faster).

**Q10. Why can't you fix a hold violation by changing clock frequency?**
Hold slack = Tclk-to-q + Tcomb - Th. There is no clock period term in this equation. Hold is a race condition between two edges of the **same clock** — it depends only on the data path delay vs hold requirement, not the clock frequency.

#### Sync vs Async Reset

**Q11. Clock might not be stable during power-on. Which reset type and why?**
**Async reset.** It takes effect immediately when rst_n goes low — no clock edge needed. Sync reset only triggers at a clock edge, so if the clock is dead or unstable at power-on, the chip never enters a known state.

**Q12. What is the danger of async reset *release*, and how do you fix it?**
When rst_n goes 0→1 (release), it's asynchronous to the clock. If release happens inside a FF's **recovery/removal window** (analogous to setup/hold for the reset pin) → metastability.
**Fix:** 2-FF reset synchronizer — assertion stays instant (async), release is synchronized to the clock:
```
rst_n (async) --> [FF1] --> [FF2] --> rst_n_sync (to all FFs)
                   ^clk      ^clk
```

**Q13. Synthesis report shows unexpected latches. Most likely coding mistake?**
Incomplete `if/else` or `case` without `default` in a combinational `always_comb` block. If any code path doesn't assign a value, synthesis must "remember" the old value → latch inferred.
```systemverilog
always_comb begin
    if (sel) out = a;
    // missing else → latch on 'out'
end
```
Fix: always assign a default at the top of the block, or ensure every branch assigns every signal.

#### Latch vs FF

**Q14. What is "time borrowing" and why does it require latches?**
With FFs, each pipeline stage gets exactly 1 clock period — hard boundary at the edge. With latches, the boundary is **transparent** during one clock phase. If stage A is slow (60% of period) and stage B is fast (40%), stage A's late signal can flow through the latch into stage B's time budget. A "borrows" slack from B.
FFs can't do this — they block data at the edge regardless of downstream slack.

**Q15. In a clock gating cell (ICG), why is there a latch on the enable signal?**
`gated_clk = clk & enable`. If `enable` toggles while `clk` is high → **glitch** on gated_clk (false clock edge → all downstream FFs fire incorrectly).
The latch is negative-level sensitive (transparent when clk=0). Enable can only update while clk is low. By the time clk goes high, the latched enable is locked and stable → no glitch.
```
enable --> [NEG LATCH] --> latched_en --> [AND] --> gated_clk
              ^clk                         ^clk
```

---

## Day 4: Arbiters (2026-03-31)

### Modules Completed
- `arbiter_fixed_priority.sv` + testbench — compiled, simulated
- `arbiter_round_robin.sv` + testbench — compiled, simulated

### Quiz Q&A (Review Before Interview)

#### Arbiter Fundamentals

**Q16. What is an arbiter? When do you need one?**
An arbiter decides who gets access when multiple requestors want the same shared resource simultaneously (e.g., multiple CPU cores accessing a memory bus). Input `req[N-1:0]` has multiple bits high; output `grant[N-1:0]` is one-hot — exactly one winner per cycle.

**Q17. Fixed-priority arbiter — how does it work in one line of code?**
`grant = req & (~req + 1)` — isolates the lowest set bit using the two's complement trick. `req[0]` has highest priority. LSB = highest priority because the two's complement trick naturally isolates the LSB — the convention follows the cheapest hardware implementation.

**Q18. Round-robin arbiter — what state does it need and why does it require a clock?**
It needs a **mask register** to remember who was last served. After granting bit `i`, the mask disables bits `[i:0]` so priority rotates. New mask: `~(grant | (grant - 1))`. The mask is a flip-flop → requires a clock. Fixed-priority has no state → no clock (pure combinational).

**Q19. Round-robin: what happens when all masked requests are zero?**
The mask has filtered out all active requestors (everyone in this round has been served). Fall back to unmasked fixed-priority on the raw `req` — this starts a new round. `grant = (masked_req != 0) ? masked_grant : unmasked_grant`.

**Q20. Two key bit tricks for arbiter design?**
- **Isolate lowest set bit:** `x & (~x + 1)` — used for fixed-priority grant.
- **Mask everything at and below a one-hot bit:** `~(x | (x - 1))` — used to generate the round-robin mask after a grant.

**Q21. Fixed-priority arbiter: what is the biggest problem?**
**Starvation.** Low-priority requestors may never get access if higher-priority requestors keep requesting. Round-robin solves this by rotating priority after each grant.

---

## Day 4 PM: Cache Hierarchy (2026-03-31)

### Quiz Q&A (Review Before Interview)

#### VIPT Aliasing

**Q22. VIPT aliasing check formula and example.**
Formula: `cache_size / associativity ≤ page_size`. You do NOT need cache line size for this check.
Example: 64KB L1, 4-way, 4KB pages → 64KB / 4 = 16KB > 4KB → **aliases!**
Index+offset = log2(16KB) = 14 bits, but page offset = 12 bits. The 2 extra index bits come from virtual page number → different virtual addresses can map to different sets for the same physical line.
Fixes: increase associativity to 16-way (64KB/16=4KB), use 16KB pages, or OS page coloring.

**Q23. Why VIPT instead of VIVT or PIPT?**
- VIVT: fast but aliasing — two virtual addresses can map to same physical address → incoherent copies in cache.
- PIPT: no aliasing but TLB is on critical path — must translate before indexing → slow.
- VIPT: index with virtual bits (parallel with TLB), tag with physical bits (after TLB). Fast + no aliasing (if formula is met).

**Q24. Why can snoops skip L1 in an inclusive hierarchy?**
L2 is a superset of L1. If a snooped line is not in L2, it's guaranteed not in L1. Saves power and avoids extra port on L1 tag array.

**Q25. L2 evicts a line in an inclusive hierarchy. What happens to L1?**
L1 must be **back-invalidated**. If L1 holds that line in Modified state, it must **write back dirty data first**, then invalidate. Otherwise the most recent data is lost.

**Q26. Why does Intel Skylake+ use NINE (non-inclusive non-exclusive) for L3?**
With many cores (6-16+), inclusive L3 wastes too much capacity — every line in every core's L1+L2 must also live in L3, filling it with duplicates. NINE lets L3 hold unique data not in any private cache, giving much better effective capacity. Requires a snoop filter/directory to track which cores hold which lines.

**Q27. Inclusive vs exclusive: effective capacity?**
- 32KB L1 + 256KB L2 **exclusive**: 32 + 256 = 288KB (no overlap, a line lives in one or the other).
- 32KB L1 + 256KB L2 **inclusive**: 256KB (L1's 32KB is duplicated in L2).

#### Inclusion Policy Summary

| | Inclusive | Exclusive | NINE |
|--|----------|-----------|------|
| Duplicate data? | Yes | No | Maybe |
| L2 evict → L1? | Back-invalidate | No | No |
| Snoop check | L2 only | Both L1+L2 | Snoop filter |
| Capacity | Wasted | Optimal | Good |
| Used by | Intel (older), ARM | AMD | Intel Skylake+ |

#### Cache Internals

**Q28. VIPT aliasing formula — why is it `cache_size / associativity`?**
`cache_size / associativity` = one way's worth of cache = the total address space covered by index + offset bits. Associativity doesn't consume address bits — all N ways share the same index. So address bits needed = `log2(cache_size / associativity)`. If this exceeds `log2(page_size)`, extra index bits come from virtual page number → aliasing.

**Q29. What is OS page coloring?**
When VIPT aliases (extra index bits above page offset), the OS constrains physical page allocation so that those extra bits always match between virtual and physical addresses. Pages are grouped by "color" (the value of those bits). Aliases must use the same color. Downside: constrains memory allocation, can cause fragmentation. Hardware fix (more associativity) preferred.

**Q30. Cache lookup structure: how does index + tag comparison work?**
Index selects a set in the tag SRAM and data SRAM (both read in parallel). N tag comparators fire in parallel comparing address tag against all N ways. The matching way's data is muxed to output. This is NOT a full CAM — index narrows to one set first, then only N comparisons (cheap). Full CAM (like TLB) compares against every entry (expensive, doesn't scale).

**Q31. Inclusive means L2 has a copy — does that mean write-through?**
No. Inclusive only means L2 has an **entry** (tag) for every line in L1. The data can be stale — L1 may hold a Modified (dirty) copy with newer data. On L2 eviction, L1 must write back dirty data before invalidating. Write-through is a separate policy (every write goes to L2 immediately) — most modern caches use write-back instead.

**Q32. Exact definitions: Inclusive vs Exclusive vs NINE.**
- **Inclusive:** L2 ⊇ L1 (always). Enforced by back-invalidation on L2 eviction.
- **Exclusive:** L2 ∩ L1 = ∅ (always). Lines swap between levels — L1 eviction sends line to L2, L2 hit moves line up to L1 and removes from L2.
- **NINE:** No guarantee. A line can be in both, either, or neither. No enforcement. Needs snoop filter to track what's where.

#### MSHRs, Replacement, Prefetch

**Q33. What are MSHRs and what is a non-blocking cache?**
MSHRs (Miss Status Holding Registers) track outstanding cache misses (4-16 entries). On a miss: check if an MSHR already tracks that address. If yes (secondary miss) → merge the requestor into the existing entry, no new memory request. If no (primary miss) → allocate new MSHR, send request to next level. When data returns, wake all waiting requestors. A **non-blocking cache** = cache with MSHRs that can keep servicing hits while misses are pending.

**Q34. What happens when all MSHRs are full?**
New misses must **stall** until an MSHR frees up. MSHR count limits memory-level parallelism — too few MSHRs bottleneck performance on memory-intensive workloads.

**Q35. Replacement policies: LRU vs Tree-PLRU vs RRIP.**
- **True LRU:** Track full access ordering. Needs `log2(N!)` bits per set. 16-way → 45 bits. Perfect eviction choice but expensive in hardware.
- **Tree-PLRU:** Binary tree of bits (N-1 per set). 16-way → 15 bits. Each access flips bits on the path. To evict, follow the tree to find approximate LRU victim. Not perfect but close, far cheaper.
- **RRIP:** 2-3 bit counter per line predicting re-reference interval. New lines inserted as "far reuse" (not MRU — prevents scan pollution). Hits set to "near reuse." Evict max-counter lines. Better than LRU for LLC with large/streaming worksets.

**Q36. Why is pure LRU bad for LLC with large working sets?**
When the working set exceeds cache capacity, LRU thrashes — every access evicts a line that will be needed again soon, cycling through the entire set with zero hits. RRIP fixes this by inserting new lines at low priority ("far reuse"), so a streaming scan doesn't evict hot data. Only lines that get re-accessed are promoted to high priority.

**Q37. Prefetcher types.**
- **Next-line:** Miss on line N → prefetch N+1. Catches sequential access. Simple.
- **Stride:** Track miss addresses per PC, detect constant stride. Catches `arr[0], arr[256], arr[512]...`
- **Stream:** Detect sequential streams, run ahead aggressively. Multiple stream trackers (8-32).

---

## Day 5: Pipeline Design (2026-04-01)

### Modules Completed
- `pipeline_5stage.sv` + testbench — compiled, simulated (7/7 tests pass)
- `pipeline_5stage_2wide.sv` + testbench — compiled, simulated (6/6 tests pass)

### Quiz Q&A (Review Before Interview)

#### Pipeline Fundamentals

**Q38. What are the 5 classic pipeline stages and what does each do?**
- **IF (Instruction Fetch):** Latch instruction from input into IF/ID pipeline register.
- **ID (Instruction Decode):** Read register file (with forwarding), decode control signals, detect hazards.
- **EX (Execute):** ALU computes result (arithmetic) or address (load/store). Result goes to EX/MEM register.
- **MEM (Memory):** Load reads data memory; store writes data memory. Result goes to MEM/WB register.
- **WB (Write Back):** Write result back to register file.

**Q39. What is a RAW (Read-After-Write) hazard and how does forwarding solve it?**
RAW: instruction B reads a register that instruction A writes, but A hasn't reached WB yet. Without forwarding, B reads stale data.
**Forwarding** (bypass): tap the result from a later pipeline stage (EX/MEM or MEM/WB) and feed it directly to the ID stage's operand mux, bypassing the register file. Priority: EX/MEM > MEM/WB > regfile (newest value wins).

**Q40. Why does a LOAD instruction cause a 1-cycle stall even with forwarding?**
A LOAD's data isn't available until the **end of the MEM stage** (it must read data memory). If the next instruction needs that value in its EX stage, there's no forwarding path — the data doesn't exist yet. Solution: insert a 1-cycle **bubble** (stall IF/ID, flush ID/EX to NOP). After the stall, the loaded value is in MEM/WB and can be forwarded.

**Q41. Load-use stall detection logic?**
```systemverilog
stall = id_ex.mem_read && (id_ex.rd != 0) &&
        (id_ex.rd == if_id.rs1 || id_ex.rd == if_id.rs2);
```
Check: is the instruction in EX a LOAD (`mem_read=1`)? Does it write to a non-zero register? Does the instruction in ID read that register?

**Q42. Why use `typedef struct packed` for pipeline registers?**
Industry standard. Each pipeline boundary has different fields (IF/ID needs rs1/rs2 addresses, ID/EX needs operand data, EX/MEM needs ALU result, etc.). Structs make it clear what data each stage passes to the next. `packed` ensures the struct maps to a flat bit vector — synthesizable and compatible with `<= '0` for reset.

**Q43. What are control signals and why do they travel through the pipeline?**
Control signals (e.g., `reg_write`, `mem_read`) are decoded **once** in the ID stage from the opcode, then propagated through pipeline registers alongside the data. Each stage uses the control signals to decide what to do (EX: which ALU op; MEM: read or write memory; WB: write register file or not). This avoids re-decoding the opcode at every stage.

#### Forwarding Bug: LOAD vs ALU

**Q44. Why must the forwarding path use `mem_result` instead of `ex_mem.alu_result`?**
For ALU instructions, `ex_mem.alu_result` is the correct result. But for LOADs, `ex_mem.alu_result` is the **memory address**, not the loaded data. The actual loaded data comes from the MEM stage's combinational read: `mem_result = dmem[ex_mem.alu_result]`. Using `mem_result` works for both: it equals `alu_result` for non-loads and `dmem[addr]` for loads.

#### 2-Wide Superscalar

**Q45. What is intra-group dependency and how does a 2-wide pipeline handle it?**
Intra-group dependency: pipe 1's source register matches pipe 0's destination register **in the same fetch group**. Since pipe 0 hasn't even started executing, there's no forwarding path. Solution: squash pipe 1 to NOP, issue only pipe 0. Pipe 1's instruction will be re-fetched next cycle.
```systemverilog
intra_dep = inst_reg_write[0] && (inst_rd[0] != 0) &&
            (inst_rd[0] == inst_rs1[1] || inst_rd[0] == inst_rs2[1]);
dual_issue = !intra_dep && !stall;
```

**Q46. How does the forwarding network scale in a 2-wide pipeline?**
- **1-wide:** 2 operands × 2 forward sources (EX/MEM, MEM/WB) = **4 comparators**
- **2-wide:** 4 operands × 4 forward sources (EX/MEM[0], EX/MEM[1], MEM/WB[0], MEM/WB[1]) = **16 comparators**
Forwarding priority: EX/MEM[0] > EX/MEM[1] > MEM/WB[0] > MEM/WB[1] > regfile. Pipe 0 has higher priority within the same stage because it's the older instruction in program order.

**Q47. What happens when both pipes write to the same register in WB?**
Pipe 0 writes first, pipe 1 writes second. Since pipe 1 is the **newer** instruction in program order, its value wins — which is correct (last-writer-wins semantics). In SystemVerilog, the two `if` blocks in the same `always_ff` execute sequentially, so pipe 1's write naturally overwrites pipe 0's.

**Q48. 2-wide load-use stall — how many comparisons?**
Must check both EX pipes against both ID instructions: 2 EX loads × 4 ID operands = **8 comparisons**. If either EX pipe has a LOAD whose rd matches any ID operand, stall both pipes.

**Q49. What extra hardware does 2-wide superscalar need vs 1-wide?**
| Resource | 1-wide | 2-wide |
|----------|--------|--------|
| Pipeline registers | 1 set | 2 sets |
| Register file read ports | 2 | 4 |
| Register file write ports | 1 | 2 |
| ALUs | 1 | 2 |
| Memory ports | 1R+1W | 2R+2W |
| Forwarding comparators | 4 | 16 |
| Dependency check | None | Intra-group (2 comparisons) |

The forwarding network and multi-port register file are the main area/timing costs. This is why most designs cap at 4-6 wide — the forwarding network grows as O(width²).

### Pipeline Deep-Dive Quiz (2026-04-02)

**Q50. What is `rs1_data` in the pipeline — register or wire? What drives it?**
Wire (combinational signal). Driven by the forwarding mux output. The mux selects between `mem_result` (from EX/MEM stage), `mem_wb.result` (from MEM/WB stage), or `regfile[rs1]` (default). The wire connects the forwarding mux output to the `id_ex.rs1_data` pipeline register input.

**Q51. Why does the forwarding mux use `mem_result` instead of `ex_mem.alu_result`?**
For ALU instructions, `ex_mem.alu_result` is the correct result. But for LOADs, `ex_mem.alu_result` holds the **memory address**, not the loaded data. `mem_result` is the output of the MEM-stage mux: it reads `dmem[addr]` for LOADs and passes through `alu_result` for ALU ops. So `mem_result` always has the correct value regardless of instruction type.

**Q52. What is the critical path in a 5-stage pipeline with forwarding? Trace the full chain.**
The MEM-to-ID forwarding path for LOADs:
`ex_mem (Q output)` → load/ALU mux (`mem_read` select) → SRAM read (`dmem[addr]`) → forwarding mux (comparator + select) → `id_ex (D input)`
This is two muxes plus an SRAM access in one cycle. The SRAM access dominates. Fix: split MEM into 2 stages, or accept a 2-cycle load-use penalty.

**Q53. During a load-use stall, what happens to `if_id` and `id_ex` on the same clock edge?**
- `if_id` **holds** its current value (keeps the instruction for retry next cycle)
- `id_ex` is **flushed to zero** (bubble inserted to prevent the held instruction from executing twice)
Rule: upstream holds, downstream flushes. This happens on a single clock edge.

**Q54. Why don't EX and MEM stages need stall logic?**
The bubble (`'0`) inserted into `id_ex` during the stall naturally flows downstream through EX → MEM → WB. Those stages always advance unconditionally — they just see a NOP pass through and do nothing. No special logic needed.

**Q55. Why does `if_id <= '0` work as a bubble for a packed struct?**
`'0` sets all bits to zero. The critical fields: `reg_write=0` (no register write), `mem_read=0` (no memory access), `op=000=OP_NOP`. These ensure the bubble does nothing as it flows through the pipeline. This works because we designed `OP_NOP=0` and all control signals default to inactive at zero.

**Q56. How does the synthesis tool implement the stall (clock enable) on `if_id`?**
As an ICG (Integrated Clock Gating) cell: a negative-level-sensitive latch + AND gate. When `stall=1`, the ICG blocks the clock — the flip-flop never sees a clock edge, so it holds its value naturally. The latch prevents glitches on the enable signal from creating false clock edges. This saves power vs. the feedback-mux alternative.

**Q57. The forwarding priority is EX/MEM > MEM/WB > regfile. Why does EX/MEM win?**
EX/MEM holds the **newer** (younger) instruction's result. If two in-flight instructions both write to the same `rd`, the most recent one must win — that's program-order correctness. EX/MEM is closer to ID than MEM/WB, meaning it contains a younger instruction. Forwarding the older value would give the wrong result.

**Q58. With 2 FUs in a 1-wide pipeline, what new forwarding source is needed?**
`id_ex` — forwarding the **combinational ALU result** from the EX stage. With alternating dispatch to 2 FUs, back-to-back dependent instructions land in different FUs. When the consumer is in ID, the producer is still in `id_ex` (not yet in `ex_mem`). So we must forward from `id_ex` using the combinational ALU output. This doesn't work for LOADs (only have the address), which is why the load-use stall still applies.

**Q59. In OoO designs, why can't we use a single global stall wire?**
Different structures (issue queue, ROB, load queue, store queue) stall independently. A cache miss stalls only that memory instruction while ALU instructions keep issuing. Queues between stages absorb timing mismatches. Each structure uses its own valid/ready handshake or credit-based flow control instead of a global stall.

### Skid Buffer + Valid/Ready Handshake (2026-04-02)

#### Module Completed
- `skid_buffer.sv` + testbench — compiled, simulated (5/5 tests pass)

#### Concept Q&A

**Q60. What problem does a skid buffer solve?**
It breaks the combinational `ready` path between modules. Without it, each module's `ready` depends on the next module's `ready`, creating a long combinational chain across every hop. The skid buffer replaces this with `up_ready = !buf_valid` — a local register output with no dependency on downstream.

**Q61. How does a skid buffer achieve zero latency?**
When the buffer is empty, `dn_data = up_data` is a combinational wire through a mux (selected by `buf_valid=0`). No register in the data path — data passes straight through in the same cycle. The register only activates during backpressure.

**Q62. What are the 4 operating cases of a skid buffer?**
| `buf_valid` | `up_valid` | `dn_ready` | Action |
|---|---|---|---|
| 0 | 1 | 1 | **Pass-through** — wire, no buffering |
| 0 | 1 | 0 | **Catch** — save `up_data` into buffer |
| 1 | X | 1 | **Drain** — downstream takes buffered data |
| 1 | X | 0 | **Hold** — nothing moves, wait |
When `buf_valid=1`, `up_valid` doesn't matter because `up_ready=0` — upstream is blocked.

**Q63. How does upstream get stalled when the buffer is full?**
`up_ready = !buf_valid`. When buffer is full, `up_ready=0`. The valid/ready protocol requires the sender to hold `valid` and `data` stable when `ready=0`. No special stall logic needed — the protocol enforces it.

**Q64. Why can't we do "drain + catch" (swap) on the same cycle?**
To swap, `up_ready` must be `!buf_valid | (buf_valid & dn_ready)` — now `dn_ready` appears combinationally in `up_ready`, which is exactly the timing path the skid buffer was designed to break. The swap gives full throughput but re-introduces the problem.

**Q65. What is the throughput under sustained backpressure? How to fix?**
1 transfer per 2 cycles — one cycle to drain, one cycle to accept. Fix: use a 2-entry FIFO, which can accept and drain simultaneously (writing to different entries). `up_ready = (count != 2)` is purely registered, no combinational ready path, full throughput.

**Q66. How does the `ready` signal propagate without a skid buffer?**
Each module has its own combinational logic to generate `ready` (AND, OR, etc.), and each module's `ready` depends on the next module's `ready`. Across 5-10 hops, this creates one long combinational chain. It's not specifically muxes — it's whatever logic each module uses to decide "can I accept?" A skid buffer cuts this chain entirely.

**Q67. When do you need more than 2 entries (i.e., a real FIFO)?**
Deeper FIFOs solve a different problem — **rate mismatch** or **burst absorption**, not timing decoupling. Examples: async FIFO for clock domain crossing, depth >= burst length for AXI burst absorption, buffering when producer is faster than consumer. For pure timing decoupling (breaking the ready path), 1-2 entries is sufficient.

**Q68. Pipeline stall vs valid/ready handshake — when to use which?**
| | CPU Pipeline | AXI / Bus |
|---|---|---|
| Mechanism | Stall + bubble | Valid/ready handshake |
| Control | Global stall wire | Per-link, independent |
| Data loss prevention | Hold upstream register | Sender holds data until ready |
| Gap insertion | Bubble (flush to zero) | Sender deasserts valid |
| Coupling | Tight — stages move in lockstep | Loose — each link independent |
| Why | Stages need to see each other's data (forwarding) | Modules are independent, just moving data |

### AXI-Lite Slave + Interconnect (2026-04-02)

#### Modules Completed
- `axi_lite_slave.sv` + testbench — 5/5 tests pass (write/read REG0, all 4 regs, byte strobe byte 0, byte strobe byte 2, reset clears)
- `axi_lite_interconnect.sv` + testbench — 5/5 tests pass (slave 0 w/r, slave 1 w/r, slaves 2+3, slave isolation, byte strobe through IC)

#### Quiz Q&A (Review Before Interview) — Score: 5.5/8

**Q69. Name the 5 AXI-Lite channels and their direction (master→slave or slave→master).**
- **AW** (Write Address): master→slave — carries write address
- **W** (Write Data): master→slave — carries write data + byte strobes (wstrb)
- **B** (Write Response): slave→master — carries write response (bresp)
- **AR** (Read Address): master→slave — carries read address
- **R** (Read Data): slave→master — carries read data + response (rresp)

**Q70. What is `wstrb` and why is it needed?**
Byte-enable mask (4 bits for 32-bit data). Each bit enables writing one byte lane. Allows partial writes — e.g., `wstrb = 4'h1` writes only byte 0. Uses indexed part-select `regs[idx][b*8 +: 8]` because SystemVerilog forbids variable indices on both sides of `:`.

**Q71. What does `s_awready = !s_bvalid | s_bready` mean?**
The slave can accept a new write address when either: (1) no pending B response (`!s_bvalid`), or (2) the pending B response is being consumed this cycle (`s_bvalid & s_bready`). Same pattern as skid buffer — "I'm free, or I'm about to be free."

**Q72. In AXI-Lite, why is the slave's perspective "output ready, input valid" — opposite of what you'd expect?**
The slave is the *responder*. On AW/W/AR channels, the master drives valid (it has a request) and the slave drives ready (it can accept). On B/R channels, the slave drives valid (it has a response) and the master drives ready (it can accept). Valid = "I have data for you", ready = "I can take data."

**Q73. What is the address decoding in an AXI-Lite interconnect?**
Top bits of the address select which slave, lower bits are the offset within that slave's register space. For 4 slaves with 8-bit address: `wr_sel = m_awaddr[7:6]` (top 2 bits select slave 0-3), `s_awaddr = m_awaddr[3:0]` (lower 4 bits = register offset).

**Q74. Is the interconnect combinational or sequential?**
Purely combinational — just muxes. It routes valid/data from master to selected slave, and routes ready/response from selected slave back to master. No registers, no state. Skid buffers are placed at interconnect boundaries (not inside) when timing requires it.

**Q75. What is the difference between AXI-Lite and AXI4 full?**
AXI4 adds: burst transfers (AWLEN/ARLEN — up to 256 beats), transaction IDs (AWID/BID/ARID/RID for out-of-order responses and multiple outstanding), cache/lock/QoS signals, and wider address/data. AXI-Lite is one transaction at a time, no bursts, no IDs — for simple register-mapped peripherals.

**Q76. Why does AXI separate write address and write data into different channels?**
So they can be accepted independently — the slave might accept the address before the data or vice versa. In AXI4 full, this enables pipelining: the address can arrive before data, and data can stream in bursts. In AXI-Lite, the practical benefit is simpler handshaking (each channel has its own valid/ready).

### UART TX (2026-04-04)

#### Modules Completed
- `uart_tx.sv` + testbench — 6/6 tests pass (0x55, 0xA3, 0x00, 0xFF, tx_busy, idle line)

#### Quiz Q&A (Review Before Interview) — Score: 6/10

**Q77. What is the UART frame format? How many bits total per byte?**
1 START bit (0) + 8 DATA bits (LSB first) + 1 STOP bit (1) = **10 bits total**. No clock wire — both sides agree on baud rate ahead of time. Standard config: 8N1 (8 data, No parity, 1 stop).

**Q78. Why is the start bit 0 and stop bit 1?**
IDLE line sits at 1. Start bit = 0 creates a **falling edge** the receiver can detect. Stop bit = 1 returns to idle, guaranteeing a falling edge for the next frame's start bit. If the last data bit were 0 and no stop bit, the next start bit would be invisible (no edge).

**Q79. What happens if TX and RX baud rates don't match?**
Receiver samples at wrong times, gets corrupted data. The sampling point drifts further with each bit — by bit 7-8, it may land in the wrong bit entirely. Must match within ~2-3%.

**Q80. With 50MHz clock and 115200 baud, how many clock cycles per bit? Per byte?**
- Per bit: `50,000,000 / 115,200 = 434 cycles`
- Per byte: `434 × 10 = 4,340 cycles` (10 bits: start + 8 data + stop)

**Q81. What does `tx_busy` tell the CPU, and why is it needed?**
Tells the CPU the transmitter is still sending the last frame — don't send another byte yet. CPU must wait for `tx_busy = 0` before pulsing `tx_start` again, otherwise the new byte overwrites the shift register mid-transmission.

**Q82. In UART RX, why sample at the middle of each bit period?**
Two reasons: (1) At bit edges, the signal is still transitioning (rise/fall time), voltage is unstable. Middle is most stable. (2) Any baud rate mismatch between TX and RX causes the sample point to **drift** over 10 bits — sampling at the middle gives maximum margin before drifting into the wrong bit.

### Clock Divider (2026-04-05)

#### Modules Completed
- `clk_divider.sv` + testbench — 6/6 tests pass (÷3 freq, ÷4 freq, ÷5 freq, ÷3 duty 50%, ÷4 duty 50%, ÷5 duty 50%)

#### Quiz Q&A (Review Before Interview) — Score: 6/10

**Q83. For an even divide-by-6 with 50% duty, how many cycles high? How many low?**
Period = 6 input cycles. 50% duty = **3 high, 3 low**. Divide-by-N means the output period is N input cycles, not N high + N low.

**Q84. Why can't a simple posedge counter achieve 50% duty for odd division (e.g., ÷3)?**
Period = 3 cycles, 50% duty needs 1.5 high + 1.5 low. But posedge-only logic can only toggle at integer cycle boundaries (0, 1, 2, 3...). Best you can do is 1 high + 2 low (33%) or 2 high + 1 low (67%).

**Q85. How does the posedge + negedge OR trick achieve 50% duty for odd division?**
Negedge gives half-cycle resolution — edges at 0.5, 1.5, 2.5... Combined with posedge (0, 1, 2...), you can now toggle at any half-cycle boundary. For ÷3: `clk_pos` is high for count 0 (cycle 0–1), `clk_neg` is the same but shifted by 0.5 cycle. OR them → high from 0 to 1.5, low from 1.5 to 3.0. Exactly 50%.

**Q86. What is `generate` used for in SystemVerilog? Two common use cases.**
Conditionally creates hardware at **compile time** (not runtime — no hardware is being "decided" dynamically). Two uses: (1) `generate if` — conditional implementation based on parameters (e.g., even vs odd divider). (2) `generate for` — instantiate N copies of a module (e.g., N SRAM banks, N pipeline stages, N slaves in an interconnect).

### Dual-Port RAM + Register File (2026-04-05)

#### Modules Completed
- `dual_port_ram.sv` (1R1W, sync read) + testbench — 5/5 tests pass
- `regfile_2r1w.sv` (2R1W, async read, x0 hardwired) + testbench — 6/6 tests pass

#### Quiz Q&A (Review Before Interview) — Score: 3.5/6

**Q87. Synchronous read vs asynchronous read — what's the difference? Which does SRAM use? Register file?**
- Sync read: output registered, data available **next cycle**. SRAM uses this because the physical circuit needs time — address decode → wordline drive → bitline sense → latch output. That chain takes a full cycle.
- Async read: output combinational, data available **same cycle**. Register file uses this because FFs have a direct mux to output — no sense amp delay. The pipeline needs operands immediately in the decode cycle.

**Q88. Why does the dual-port RAM have no reset on the memory array?**
Adding reset with a for-loop prevents the synthesis tool from **inferring an SRAM macro**. The tool sees the reset loop and thinks "each element needs individual reset" → builds from flip-flops instead. Real SRAM macros have no reset pin — contents are undefined at power-on. If you want FF inference (e.g., small register file), then adding reset is fine.

**Q89. Why does a pipeline register file need 2R1W?**
The decode stage reads **rs1 and rs2 simultaneously** in the same cycle. 1R1W can only read one operand at a time — would need 2 cycles, halving throughput. So: 2 read ports for rs1/rs2, 1 write port for rd writeback.

**Q90. How is RISC-V x0=0 implemented in RTL?**
Two places: (1) **Write side**: `if (wr_en && wr_addr != '0)` — block all writes to address 0. (2) **Read side**: `assign rd_data = (rd_addr == '0) ? '0 : regs[rd_addr]` — always return 0 for address 0. x0=0 provides a free constant for compare-to-zero (`beq x1, x0`), discard results (`add x0, x1, x2`), and register copy (`add x1, x0, x2`).

**Q91. Read and write same address on same posedge — what happens in our dual-port RAM?**
**Read-first.** Both `always_ff` blocks use nonblocking `<=`. On posedge, all right-hand sides evaluate first (read sees old `mem` value), then all left-hand sides update (write commits new value). So `rd_data` gets the old value. This is a SystemVerilog simulation guarantee of `<=` — all RHS evaluate before any LHS updates. In real SRAM, behavior depends on the macro (read-first, write-first, or undefined).

**Q92. Why does a register file synthesize to flip-flops, not SRAM?**
Small depth (32 entries) — no matching SRAM macro from the foundry (macros come in sizes like 256×32, 512×64). Also, SRAM macros don't support async read or multi-port (2R1W), which register files require.

---

## Day 6: RSD DCache 1R1W Redesign (2026-04-02)

### Milestone Completed
- Reworked RSD `DCache` from the original generic 2-port read/write array model to a true `1R1W` cache-array contract
- Verified the redesign with the repo's Verilator `test-1` regression: full level-1 suite passes
- `test-2` is still blocked by missing external `RSD_ENV` software/compliance assets, not by RTL failure

### Main RTL Changes
- Split the DCache array interface into one read-side slot and one write-side slot
- Added write-slot reservation for read-hit updates so the cache still supports store-hit and flush behavior under `1R1W`
- Replaced `BlockTrueDualPortRAM` usage in `DCache` with `BlockDualPortRAM`
- Fixed follow-up correctness issues:
  - replayed loads may complete from either the live MSHR or the newly filled cache line
  - store commit must stop waiting once the related MSHR has already been invalidated
  - MSHR fill / store reservation arbitration needed fairness to avoid deadlock

### Quiz Q&A (Review Before Interview)

**Q50. Why is converting a cache from 2RW to 1R1W not just a memory-wrapper change?**
Because the original RTL may assume two fully capable array ports in the same cycle. If the new memory only supports one read port and one write port, the arbitration and update timing must be redesigned. That changes the cache control logic, not just the storage primitive.

**Q51. What is the key control idea in a 1R1W DCache?**
Treat the array as one read-side slot plus one write-side slot. A read request can reserve the next-cycle write slot if it may need to update the line after tag compare. This preserves store-hit and flush behavior without violating the `1R1W` contract.

**Q52. What bug can happen if refill writes always beat store-hit reservations?**
The store queue can stop draining. A committed store needs a read now and a write on the following cycle. If MSHR refill traffic keeps taking the write side forever, the store never completes and the core can deadlock.

**Q53. After a cache fill, why can a replayed load complete from either MSHR or cache?**
Under the old model, the replay path could rely on the live MSHR entry. Under the new `1R1W` model, once the fill is accepted and written into the cache, the line may be visible from the cache before the old replay assumptions clear. The replay logic must accept both sources.

**Q54. If the redesign is functionally correct but slower, what should regression do?**
Do not call it a logic bug. First check whether the program eventually reaches the correct final PC/register state with a larger cycle budget. If it does, update the regression's `MaxTestCycles` to match the new microarchitectural latency.

### Follow-On: DCache SRAM Wrapper Bring-Up
- Added a new `DCacheSRAM.sv` wrapper file for the `1R1W` DCache arrays
- Data array plan:
  - `256 x 64` line storage per way
  - implemented as `2 x 128 x 64` depth banks
  - uses macro bit-write masking to preserve byte writes
- Tag array plan:
  - `256 x 22` tag+valid storage per way
  - implemented as `2 x 128 x 32` depth banks with width padding
- Dirty and replacement arrays are still left on generic inferred RAM for now because they are tiny
- Verilator now uses dedicated functional models of the same TSMC16 SRAM macro interfaces, so cache simulation stays on the SRAM-macro path without reverting to the original `BlockDualPortRAM`

### Current Validation Status
- Added a local Verilator-only SRAM model file that implements the exact cache macro interfaces used by RSD:
  - `TS6N16ADFPCLLLVTA128X64M4FWSHOD`
  - `TS6N16ADFPCLLLVTA128X32M4FWSHOD`
- Updated the Verilator build flow to use the SRAM-macro path directly and fixed the host-side PCH build quirk in the generated make flow
- Full `test-1` now passes with the SRAM-backed cache path enabled in simulation
- Focused macro-path checks also pass:
  - `DCache`
  - `MemoryDependencyPrediction`
  - `ReplayQueueTest`
- The next ASIC-facing step is to resume the physical-design flow knowing that the cache macro path is now verified on both:
  - DC macro mapping
  - functional regression

### Real TSMC16 Cache Macro Integration
- Enabled the real cache macro path in the dedicated DC flow with:
  - TSMC16 stdcell `.db`
  - TSMC16 SRAM `.db`
  - synthesis defines `RSD_SYNTHESIS_DESIGN_COMPILER` and `RSD_USE_TSMC_CACHE_SRAM`
- Proved real macro mapping directly in Design Compiler by synthesizing the wrappers themselves:
  - `ICacheWaySRAM_TSMC16`
  - `DCacheLineSRAM_TSMC16`
  - `DCacheTagSRAM_TSMC16`
- The mapped netlists now contain real TSMC SRAM macro cells:
  - `TS6N16ADFPCLLLVTA128X64M4FWSHOD`
  - `TS6N16ADFPCLLLVTA128X32M4FWSHOD`
- Wrapper-level DC proof summary:
  - `ICacheWaySRAM_TSMC16`: 2 x `128x64` + 2 x `128x32`
  - `DCacheLineSRAM_TSMC16`: 2 x `128x64`
  - `DCacheTagSRAM_TSMC16`: 2 x `128x32`
- Ran top-level `Core` elaboration under the same real DC flow and confirmed the cache wrapper blocks are linked into the full design hierarchy:
  - `ICacheWaySRAM_TSMC16`
  - `DCacheLineSRAM_TSMC16`
  - `DCacheTagSRAM_TSMC16`

### Honest Status After Integration
- `ICache` main storage is now truly integrated to real TSMC16 SRAM macros in the DC flow
- `DCache` main line and tag storage are now truly integrated to real TSMC16 SRAM macros in the DC flow
- Small cache-side arrays such as dirty/replacement state are still generic inferred RAM/logic, which is reasonable because they are tiny
- The remaining runtime bottleneck in full-core synthesis is no longer cache storage. It is the large inferred OoO memories such as issue queues, rename tables, and register-file-related structures

### Follow-On: BTB / Gshare / MDP SRAM Integration
- Continued the ASIC-minded storage cleanup after finishing `L1I` and `L1D`
- Chose the next blocks using the rule:
  - map each structure to the implementation style that makes sense in a real OoO ASIC
- Result:
  - `BTB`: moved to real SRAM-backed storage
  - `Gshare` PHT: moved to real SRAM-backed storage
  - `MemoryDependencyPredictor`: moved to real SRAM-backed storage
  - `ReplayQueue`: intentionally left as logic for now

### Why ReplayQueue Was Deferred
- I tried a `ReplayQueue` SRAM conversion first, but it was not a clean fit
- The synchronous `1R1W` version passed elaboration but caused real functional regressions in `ReplayQueueTest`
- That is a sign the queue behavior is more timing-sensitive than the regular predictor/storage tables
- For this project, it is more realistic to keep `ReplayQueue` as logic for now instead of forcing a bad SRAM mapping

### What Changed In RTL
- Added [BTBSRAM.sv](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Src/FetchUnit/BTBSRAM.sv)
  - replaces the old inferred `BTB` table with banked `128x32` TSMC16 macros
  - current mapping is `1024 x 19`, implemented as `2` read banks and `4` depth slices per bank
- Added [GshareSRAM.sv](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Src/FetchUnit/GshareSRAM.sv)
  - packs the `2048 x 2` PHT counters into `32-bit` macro words
  - current mapping is `2` read banks backed by `2` total `128x32` macros
- Added [MDTSRAM.sv](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Src/Scheduler/MDTSRAM.sv)
  - packs the `1024 x 1` memory-dependency bits into `32-bit` macro words
  - current mapping is `2` read banks backed by `2` total `128x32` macros
- Rewired:
  - [BTB.sv](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Src/FetchUnit/BTB.sv)
  - [Gshare.sv](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Src/FetchUnit/Gshare.sv)
  - [MemoryDependencyPredictor.sv](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Src/Scheduler/MemoryDependencyPredictor.sv)
- Updated [CoreSources.inc.mk](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Src/Makefiles/CoreSources.inc.mk) to compile the new wrappers

### Verification
- Design Compiler elaboration accepted the new wrappers in the full `Core` hierarchy
- Rebuilt the Verilator simulator on the SRAM-macro path
- Focused checks passed:
  - `ControlTransfer`
  - `ControlTransferZynq`
  - `MemoryDependencyPrediction`
  - standalone `Gshare`
- Full `test-1` passed again after the BTB/Gshare/MDP integration

### Small Tooling Improvement
- Updated [Makefile.verilator.mk](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Src/Makefile.verilator.mk) to build the generated Verilator C++ model with:
  - `-j16`
  - `-fuse-ld=gold`
- This keeps the regression rebuild practical on the current server

### Honest Status Now
- `ICache`: SRAM-backed and verified
- `DCache`: SRAM-backed and verified
- `BTB`: SRAM-backed and verified
- `Gshare`: SRAM-backed and verified
- `MemoryDependencyPredictor`: SRAM-backed and verified
- `ReplayQueue`: still logic, by design for now
- Remaining major non-cache/non-predictor blocks are still things like:
  - issue queue
  - register file
  - rename / active-list structures

### Full DC Rerun After Cache + Predictor SRAM Work
- Reran full `Core` synthesis in TSMC16nm after integrating SRAM-backed `ICache`, `DCache`, `BTB`, `Gshare`, and `MemoryDependencyPredictor`
- Used the updated DC flow with `set_host_options -max_cores 16`
- Run directory:
  - `/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/DesignCompiler/runtime_full_sram_16c`

### Final Synthesis Results
- Wall-clock runtime:
  - `real 3064.62s` which is about `51m 05s`
- QoR summary from [qor.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/DesignCompiler/runtime_full_sram_16c/reports/qor.rpt):
  - `Macro Count`: `28`
  - `Leaf Cell Count`: `500140`
  - `Combinational Cell Count`: `341451`
  - `Sequential Cell Count`: `158689`
  - `Design Area`: `386213.269227`
  - `Critical Path Length`: `2.20`
  - `Critical Path Slack`: `7.68`
  - `Total Negative Slack`: `0.00`
  - `No. of Violating Paths`: `0`
  - `Worst Hold Violation`: `-0.26`
  - `Total Hold Violation`: `-8109.75`
  - `No. of Hold Violations`: `160755`
- Generated outputs:
  - [Core.ddc](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/DesignCompiler/runtime_full_sram_16c/mapped/Core.ddc)
  - [Core.v](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/DesignCompiler/runtime_full_sram_16c/mapped/Core.v)
  - [area.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/DesignCompiler/runtime_full_sram_16c/reports/area.rpt)
  - [timing.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/DesignCompiler/runtime_full_sram_16c/reports/timing.rpt)

### What The Numbers Mean
- Setup is clean at the current `10ns` synthesis target
- Hold is still very noisy, which is expected before CTS and physical implementation
- The macro count increased from the earlier cache-only baseline because `BTB`, `Gshare`, and `MDP` are now also using real SRAM-backed storage
- The mapped netlist contains:
  - `8` instances of `TS6N16ADFPCLLLVTA128X64M4FWSHOD`
  - `20` instances of `TS6N16ADFPCLLLVTA128X32M4FWSHOD`

### Main Remaining Bottlenecks
- From [area.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/DesignCompiler/runtime_full_sram_16c/reports/area.rpt), the next dominant structures are now:
  - `registerFile`: `151451.2884` area, about `39.2%`
  - `dCache`: `38894.0077` area, about `10.1%`
  - `iCache`: `37285.4124` area, about `9.7%`
  - `issueQueue`: `31024.5817` area, about `8.0%`
  - `btb`: `27340.2134` area, about `7.1%`
  - `activeList`: `25732.8062` area, about `6.7%`
  - `brPred`: `7349.4098` area, about `1.9%`
- This confirms that the remaining heavy problem is not the cache/predictor tables anymore. It is the OoO core state structures, especially:
  - register file
  - issue queue
  - active list / rename-related multiported state

## Day 8: First Innovus Bring-Up On RSD Core

### Goal
- Start the real physical-design flow on the SRAM-backed `Core` netlist from:
  - [Core.v](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/DesignCompiler/runtime_full_sram_16c/mapped/Core.v)
  - [Core.sdc](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/DesignCompiler/runtime_full_sram_16c/mapped/Core.sdc)
- Use a loose first target:
  - `10ns` clock period
  - equivalent to `100 MHz`

### New Innovus Flow
- Added a dedicated Innovus project under:
  - `/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus`
- Added:
  - [Makefile](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/Makefile)
  - [innovus_mmmc_qrc.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_mmmc_qrc.tcl)
  - [innovus_flow.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_flow.tcl)
  - [rsd_core_apr.sdc](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/rsd_core_apr.sdc)
- Used:
  - TSMC16nm tech LEF
  - stdcell LEF/lib
  - SRAM LEF/lib
  - QRC tech file

### What Worked
- `init` completed cleanly and saved:
  - `/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/db/init.enc`
- `place` completed cleanly and saved:
  - `/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/db/place.enc`
- `cts` completed cleanly and saved:
  - `/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/db/cts.enc`

### Placement Snapshot
- Early global-route congestion after placement was effectively clean:
  - `0.00%` horizontal overflow
  - `0.00%` vertical overflow
- The main pre-CTS setup problem was concentrated in `ReplayQueue`
- Placement reports:
  - [place_area.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/place_area.rpt)
  - [place_timing.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/place_timing.rpt)

### CTS Snapshot
- CTS runtime:
  - about `19m 15s` real time
- Clock-tree summary:
  - `2325` clock buffers
  - `158717` sinks
  - insertion delay about `0.212ns` average
  - skew about `0.019ns` vs `0.013ns` target
- CTS completed with no tool errors and saved:
  - [cts_timing.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/cts_timing.rpt)
- Main remaining CTS setup issue:
  - `ReplayQueue` paths around `-0.50ns`

### First Route Attempt: Important Findings
- Clock-only / early route portion looked healthy:
  - only `4` route DRCs at one stage
  - antenna `0` at that point
- Full signal routing showed two important issues:
  1. The flow had not globally connected standard-cell body-bias pins
     - `VPP`
     - `VBB`
     - This caused repeated `NRIG-34` warnings for CTS-created cells in the route log
  2. The long timing-driven route cleanup became expensive and did not converge cleanly in the first pass before I stopped it
     - one stage reached `6675` DRC violations
     - first optimization iteration reduced that to `2504`
     - antenna also rose into the `~1900-2100` range during that route optimization

### Flow Fix Already Applied
- Patched [innovus_flow.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_flow.tcl) to globally connect:
  - `VPP -> VDD`
  - `VBB -> VSS`
- Also made the PG connection helper run after checkpoint restore, not just during initial import
- Verified with a short rerun:
  - the old `NRIG-34` body-bias warnings no longer appeared at the same route stage

### Current Physical-Design Status
- Real APR flow is alive and usable
- Clean milestones achieved:
  - import
  - floorplan
  - placement
  - CTS
- Route is not closed yet
- The next work is route-strategy cleanup, not more front-end/RTL change

### Immediate Next Steps
- Rerun route with the fixed `VPP/VBB` global-net connection
- Let the full signal-route optimization finish once with the corrected PG setup
- Examine:
  - final route DRC count
  - antenna count
  - connectivity/LVS-style report
  - post-route timing
- If route is still unstable, adjust route strategy before pushing toward the final target of:
  - `0 DRC`
  - `0 LVS/connectivity`

## Day 11: RSD Innovus Route Debugging and Floorplan Iteration

### What We Learned From The Full Route Flow
- The PG/body-bias fixes were real and necessary:
  - `VPP/VBB/VDDM` connection warnings are gone
- Restricting signal routing to `M1..M10` and turning off aggressive SI/timing routing cleaned up the first major route phase:
  - no AP-layer routing
  - antenna reached `0`
  - one routed phase dropped to only `4` remaining M2 DRCs
- But the full `routeDesign` still has a second major signal-routing phase, and that phase is still the real blocker
  - it climbed to thousands of DRCs again
  - it was worse than the earlier `~1150` plateau in the interrupted experiments

### Conclusion
- The remaining problem is no longer a simple route-option bug
- The remaining problem is physical structure:
  - floorplan
  - macro spacing
  - local routing access around macros and dense logic

### Structural Fix Applied
- Reworked the Innovus init stage in:
  - [innovus_flow.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_flow.tcl)
- Changes:
  - floorplan target changed from `0.40` to `0.30`
  - core margins increased from `60` to `80`
  - added `5um` halo around all hard macros with `addHaloToBlock 5 5 5 5 -allMacro`

### Rebuilt Checkpoints
- `init` reran cleanly with the larger floorplan
- `place` reran cleanly from scratch and saved a new:
  - [place.enc](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/db/place.enc)

### New Placement Snapshot
- placement runtime:
  - about `19m 14s` real time
- early global-route congestion is still essentially clean:
  - `Overflow: 1 = 0 (0.00% H) + 1 (0.00% V)`
  - see [place_congestion.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/place_congestion.rpt)
- the main timing hotspot is still `ReplayQueue`
  - pre-CTS setup is about `-0.533ns`
  - see [place_timing.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/place_timing.rpt)

### Current Status
- APR bring-up remains healthy through:
  - init
  - place
  - CTS on the earlier checkpoint
- The route flow is alive, but full route is still not converged
- We now know the next backend work is structural APR tuning, not more SRAM/RTL work

### Next Step
- Rerun `CTS -> route` on the rebuilt larger-floorplan checkpoint
- Compare whether the new geometry reduces the second full-route DRC explosion
- If not, move to deliberate macro placement rather than more router-option tuning

## Day 12: RSD PG Connectivity Root-Cause Debug

### What Changed In The Innovus Flow
- Updated [innovus_flow.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_flow.tcl) to strengthen the power-grid strategy:
  - added `M8/M9` PG stripes during init
  - reran `sroute` after CTS and again before route
  - disabled post-route wire spreading so the router does not reintroduce cleanable DRC after a zero-DRC detailed-route phase
  - added a bounded `ecoRoute -fix_drc` loop in the route stage

### Important Backend Diagnosis
- The remaining special-connectivity failures are not just generic PG-routing misses.
- I checked the TSMC16 stdcell LEF and confirmed:
  - ordinary standard cells expose `VDD/VSS` on `M1`
  - but `VPP/VBB` are on `NW/PW` well layers, not normal signal-routing metal
- I also checked the synthesized `Core.v` and found:
  - no `TAPCELL` instances were present in the mapped netlist

### Why This Matters
- That means the repeated `VPP/VBB` opens are structurally consistent with missing or insufficient well-tap / secondary-power handling.
- A plain `corePin` or `blockPin` `sroute` pass is not enough by itself.
- The backend needs body-bias-aware tap cells plus explicit secondary-power handling.

### New Structural Fix Applied
- Updated the `place` stage in [innovus_flow.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_flow.tcl) to insert well taps:
  - `addWellTap -cell TAPCELLBWP16P90_VPP_VBB -cellInterval 30 -prefix TAP`
  - followed by `refinePlace`
- Also updated the PG routing helper to include a dedicated `secondaryPowerPin` pass instead of only treating everything like ordinary `corePin` routing.

### Current Status
- This is a real flow fix, not a guess:
  - it directly addresses the physical meaning of `VPP/VBB`
  - it follows the same class of lesson from `soc_design`: solve the actual PG/connectivity structure first, then judge final LVS-style clean reports
- I started a fresh `place` rerun with tap-cell insertion and the revised PG flow, then stopped it rather than leaving a blind long job running.

### Next Step
- Re-run `place` fully with:
  - well taps inserted
  - secondary-power-aware `sroute`
- Then continue `CTS -> route` and recheck:
  - `route_special_connectivity.rpt`
  - `route_drc.rpt`
  - post-route timing

## Day 13: RSD Post-Route DRC Root Cause

### What Changed In The Flow
- Updated [innovus_flow.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_flow.tcl) again so backend iteration is cheaper:
  - added [route_raw.enc](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/db/route_raw.enc) save point immediately after `routeDesign`
  - added [route_clean.enc](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/db/route_clean.enc) save point after the post-route DRC cleanup loop
  - added a new `route_finish` stage so later debug can restart from `route_raw.enc` instead of rerunning the full route
  - disabled post-route filler insertion by default, so filler behavior does not hide the real routed DRC state

### What We Learned
- The backend problem is no longer generic signal-route congestion.
- During route:
  - signal-route DRC falls from about `10.7k` down to low double digits during the router’s own optimization loop
  - but full-chip [route_drc.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/route_drc.rpt) still explodes to the report limit of `10000`
- I parsed that report and found:
  - about `9998/10000` checked violations are `Special Wire`
  - the dominant rule is `VIA1 CUTSPACING` on net `VSS`
  - this is a PG/special-route geometry problem, not a normal signal-wire problem

### Important False Lead Closed
- I tested whether post-route fillers were the main cause.
- Result:
  - fillers were not the primary root cause
  - even with fillers disabled, the post-route `verify_drc` still reports roughly the same `VIA1` special-wire explosion
- So the correct focus stays on `sroute`/PG topology, not filler cleanup.

### New PG Strategy Change
- I changed the PG core-pin routing in [innovus_flow.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_flow.tcl) away from:
  - `corePinTarget {firstAfterRowEnd}`
- and back toward the staged `soc_design` style:
  - `corePinTarget {stripe}` on lower layers first
  - then `corePinTarget {stripe ring}`
  - then `corePinTarget {stripe ring blockring}`

### Why This Matters
- The DRC coordinates cluster at repeated PG-via sites, not random signal-routing hotspots.
- That strongly suggests the special-route connection pattern itself is wrong or too dense.
- Matching the `soc_design` staged `sroute` style is the right next experiment.

### Current Status
- A fresh `pg` rerun with the new staged `corePin` strategy is in progress.
- We still do **not** have `0 DRC / 0 LVS/connectivity`.
- But the backend debug is now much more structured:
  - we know the dominant violation class
  - we know it is special-route PG, not signal route
  - and we now have finer checkpoints to avoid re-running the full route every time

## Day 14: RSD Route Topology Split

### What Improved
- Re-ran full route from the latest `cts.enc` after the staged `sroute` fix.
- Important change in behavior:
  - the old post-route `VIA1 CUTSPACING` `Special Wire` explosion is no longer the dominant failure mode
  - regular connectivity is now clean in [route_connectivity.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/route_connectivity.rpt)
  - `routeDesign` itself still gets ordinary routed-signal DRC to `0` before the later full-chip verification step

### New Post-Route DRC Signature
- The remaining routed database is still not clean.
- Current [route_drc.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/route_drc.rpt) is dominated by:
  - `MINSTEP` on `Special Wire`
  - mainly on `M6` and `M7`
  - net `VSS` on `M6`
  - net `VDD` on `M6/M7`
- The current report summary is roughly:
  - `8903` total DRC
  - about `8246` `MINSTEP`
  - about `480` `CUTSPACING`
  - about `94` `SPACING`
- Coordinate clustering shows these are concentrated around the cache macro region, not randomly across the die.

### Connectivity Diagnosis
- [route_special_connectivity.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/route_special_connectivity.rpt) is no longer dominated by generic signal pins.
- The remaining special-connectivity failures are overwhelmingly:
  - `VPP` on standard-cell instances
  - one `VBB` residual case
- Top modules with the most `VPP` opens:
  - `brPred`
  - `btb`
  - `iCache`
- This means:
  - regular nets are already fine
  - body-bias connectivity is still a separate closure problem

### Structural Change Kept
- Updated [innovus_flow.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_flow.tcl) to remove per-macro block rings from the main PG strategy.
- The new default PG direction is:
  - macro `blockPin -> ring/stripe`
  - no `blockring` target in the normal flow
- Reason:
  - the remaining special-wire `M6/M7` min-step clusters line up with the SRAM-heavy cache region
  - per-macro block rings are the strongest suspected contributor to that geometry

### Structural Change Rejected
- I also tested an explicit `secondaryPowerPin` `sroute` pass from `route_raw.enc`.
- Result:
  - it does touch the missing body-bias network
  - but it does **not** reduce the main post-route DRC signature
  - and it introduces extra via-generation pressure and instability during the post-route cleanup experiment
- So `secondaryPowerPin` routing is not the main DRC fix and should not be the default path for every route iteration.

### Current Best Next Step
- Keep the simplified macro PG topology.
- Rebuild from `prepg.enc` using:
  - no per-macro block rings
  - direct macro `blockPin` PG hookup
- Then rerun:
  - `pg`
  - `cts`
  - `route`
- Goal of the next iteration:
  - see whether removing block rings collapses the remaining `M6/M7` special-wire min-step cluster
  - then revisit `VPP/VBB` connectivity only after the main routed geometry is cleaner

## Day 15: RSD Route-Finish PG Closure Experiments

### Stable Best Geometry Baseline
- Rebuilt the backend from the fresh full-design `place.enc` / `cts.enc` chain and confirmed the best clean raw-route base is still:
  - [route_raw.enc](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/db/route_raw.enc)
  - raw route finishes with only `4` geometry + `4` antenna markers before post-fill cleanup
- The best geometry-focused `route_finish` result still ends at:
  - `24` DRC in [route_drc.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/route_drc.rpt)
  - `0` regular-net connectivity violations

### What Helped
- Adding fillers after `route_raw.enc` is still necessary.
- After fillers, a macro-focused reconnect path is better than the earlier broad `corePin` pass:
  - `blockPinTarget {stripe ring blockring}`
  - `floatingStripeTarget {stripe ring blockring}`
- This path:
  - keeps geometry near the `24`-DRC baseline
  - avoids the runaway `M6/M7` special-wire explosion
  - confirms the ordinary routed netlist is already clean

### What Did Not Help
- Post-fill `corePin -> stripe` full-range `sroute`
  - too slow
  - gets stuck in macro-edge via failures
- Post-fill low-layer `corePin -> ring` (`M1..M4`)
  - routed `0` new wires
  - did not reduce the special-net opens
- Reintroducing per-macro `block_rings`
  - made things much worse
  - raw post-fill route jumped to hundreds of real geometry DRC
  - final special connectivity also degraded into mixed special + regular opens

### Current Best Understanding
- The remaining blocker is still **special PG connectivity**, not signal routing.
- The problem is now clearly localized around macro-side PG stitching.
- The best current no-blockring result still has:
  - `24` geometry DRC
  - `1000` capped special-net opens in [route_special_connectivity.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/route_special_connectivity.rpt)
- The open spans cluster around the SRAM macro regions:
  - `dCache`
  - `iCache`
  - `BTB`
  - `Gshare`
  - `MDP`

### Practical Conclusion
- The overnight experiments narrowed the search space:
  - **do not** use per-macro block rings in the current RSD flow
  - **do not** rely on post-fill `corePin` repair
  - **keep** the no-blockring route baseline with fillers + macro-focused reconnect
- The next structural fix should target the PG mesh itself, most likely:
  - sparse additional horizontal strap / bridge support around the macro bands
  - then rerun from the saved early checkpoint chain instead of patching only the final routed database

## Day 16: Fresh PG Rebuild and Row-Rail Connectivity Analysis

### Fresh Base Rebuilt
- Rebuilt the Innovus backend base from scratch after the PG-flow edits:
  - fresh `init.enc`
  - fresh `prepg.enc`
  - fresh `place.enc`
- Key flow changes in [innovus_flow.tcl](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/innovus_flow.tcl):
  - widened macro halo from `5` to `10`
  - added a local low-layer helper mesh in the cache / predictor / MDP / central-if bands
  - changed the low `corePin` stitch range from `M1..M7` to `M1..M6`

### Important New Result
- The new PG debug flow is materially better than the older stale-checkpoint path.
- With the fresh `prepg.enc` and the new low helper mesh:
  - `pg_after_blockpin_special_connectivity.rpt` is still capped at `1000` open terminals
  - but after the low `corePin` pass, [pg_after_corepin_low_special_connectivity.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/pg_after_corepin_low_special_connectivity.rpt) improves to:
    - `190` terminal opens
    - `810` special-wire opens
- This is the first clear evidence that the new PG topology is helping, but not enough for closure.

### Root Cause Refined Again
- The low-pass report shows the remaining `IMPVFC-200` problems are **long horizontal VDD/VSS row rails** that stay open across wide spans of the core.
- Example pattern:
  - `Net VDD: has special routes with opens at (79.957, 79.971) (1214.873, 80.061)`
  - repeated across many adjacent row heights
- That means the dominant remaining failure is:
  - not ordinary signal routing
  - not random local pin opens
  - but **standard-cell followpin rails that are still not being stitched vertically into the PG mesh**

### What Was Rejected
- A full `corePin` climb on top of the new base was tried again.
- Result:
  - it falls back into repeated via-obstruction hotspots near the SRAM/control clusters
  - it does not produce a clearly better debug checkpoint than the low-pass result
- A sparse full-core low mesh experiment was also started:
  - this is more invasive
  - it changes the geometry heavily
  - early signs show it may be too blunt compared with the more surgical local-helper approach
- So the best current checkpoint remains the **fresh low-pass PG result**, not the broader full-climb/global-mesh path.

### Current Best Understanding
- The remaining physical-design blocker is still **PG special-net closure**.
- More specifically:
  - standard-cell row rails need a better vertical stitch strategy
  - the present upper-mesh + local-helper solution improves things, but still leaves hundreds of full-width open PG fragments
- So the next useful backend experiments should stay focused on:
  - row-rail stitch strategy
  - local low-layer PG support
  - not more general signal-route tuning

### Next-Stage Plan: Soc-Design-Style PG Isolation
- The next backend step should follow the same diagnostic logic that worked in `soc_design`:
  - first isolate and clean the **stdcell / tap / followpin PG skeleton**
  - then reintroduce **macro PG hookup**
- Why this may work:
  - when macros are present during PG closure, `sroute` is trying to solve two problems at once:
    - standard-cell row-rail stitching
    - macro `blockPin` connection into the upper PG mesh
  - if macro geometry fragments the rows or blocks via ladders, the base row-rail network never becomes clean
  - once the base stdcell PG is clean, macro reintegration becomes a smaller delta problem instead of a coupled failure
- How to apply this to RSD:
  - keep the same placed design and macro locations
  - first run PG closure with **macro hookup disabled** or minimized, focused only on:
    - taps
    - endcaps
    - stdcell followpin rails
    - row-rail stitch into the mesh
  - verify whether the stdcell-only PG skeleton becomes clean
  - then re-enable macro `blockPin` hookup on top of that clean checkpoint
  - if needed, reintroduce macro connection by cluster:
    - cache cluster
    - predictor cluster
    - MDP cluster
- Decision rule:
  - if the stdcell-only PG skeleton becomes clean, then macro reintegration is the real remaining problem
  - if it does not, then the problem is deeper than macro interaction and the row-stitch topology itself still needs work

### Implemented: Split PG Stages and First Results
- Added two explicit Innovus stages in the RSD backend flow:
  - `pg_stdcell`: build the stdcell / tap / followpin PG skeleton without macro `blockPin` hookup
  - `pg_macro`: reintroduce macro PG hookup on top of the saved stdcell-only checkpoint
- The split flow now saves:
  - [pg_stdcell.enc](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/db/pg_stdcell.enc)
  - [pg_macro.enc](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/db/pg_macro.enc)
- It also writes dedicated reports:
  - [pg_stdcell_special_connectivity.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/pg_stdcell_special_connectivity.rpt)
  - [pg_stdcell_special_connectivity_summary.txt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/pg_stdcell_special_connectivity_summary.txt)
  - [pg_macro_special_connectivity.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/pg_macro_special_connectivity.rpt)
  - [pg_macro_special_connectivity_summary.txt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/pg_macro_special_connectivity_summary.txt)

### What The Split Experiment Proved
- `pg_stdcell` result:
  - `macro_terminal_opens: 84`
  - `tap_or_physical_terminal_opens: 190`
  - `stdcell_terminal_opens: 0`
  - `special_wire_opens: 727`
- `pg_macro` result:
  - `macro_terminal_opens: 0`
  - `tap_or_physical_terminal_opens: 190`
  - `stdcell_terminal_opens: 0`
  - `special_wire_opens: 811`
- Interpretation:
  - the split staging works as intended
  - macro reintegration does remove the expected macro terminal opens
  - but the dominant problem survives almost unchanged after macro reintegration
  - therefore the current root cause is **not only macro hookup**

### Refined Root Cause After The Split
- The base stdcell skeleton is **not** clean yet.
- The remaining terminal opens are almost entirely tap/physical-cell PG pins.
- The remaining special-wire opens are still large row-rail fragments, especially in the lower macro/cache region.
- Example signature from [pg_stdcell_special_connectivity.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/pg_stdcell_special_connectivity.rpt):
  - `Net VSS: has special routes with opens at (79.957, 80.547) (1214.873, 80.637)`
  - repeated across many adjacent row heights
- So the current blocker is now more specific:
  - **row-rail stitch / tap connectivity in the stdcell PG skeleton**
  - not plain macro `blockPin` hookup by itself

### Practical Conclusion From This Stage
- The `soc_design`-style isolation plan was still the right move, because it removed ambiguity.
- It told us exactly where the next fix must go:
  - improve the stdcell/tap row-rail stitch first
  - then keep macro reintegration as a second-stage delta
- The next PG experiments should therefore target:
  - low-layer boundary stitch support for taps / row rails
  - localized row-rail bridge support in the lower cache-side region
  - not another broad macro PG reroute

### Update: Stdcell-Only PG Baseline Improved, But Top-Edge VDD Still Blocks Closure
- I strengthened the `pg_stdcell` stage in the RSD Innovus flow with:
  - a boundary low-layer helper mesh
  - a denser top-edge stitch band
  - `corePin` routing allowed up to `M10`
- Best stdcell-only result with that stronger baseline:
  - `macro_terminal_opens: 37`
  - `tap_or_physical_terminal_opens: 58`
  - `stdcell_terminal_opens: 0`
  - `special_wire_opens: 3`
- This is much better than the earlier `84 / 190 / 0 / 727` split-baseline result.
- However, the remaining failures are still capped at `1000` total because of repeated **regular-routing** `VDD` opens concentrated near the top boundary.
- The dominant remaining signature in [pg_stdcell_special_connectivity.rpt](/home/fy2243/coding/design_and_perf/rsd_fengze/Processor/Project/Innovus/out/reports/pg_stdcell_special_connectivity.rpt) is:
  - `Net VDD: has regular routing with opens at (x, 1213.296) (x+0.96, 1213.872)`
  - repeated across the full core width in ~30um columns and many adjacent top-edge rows
- Interpretation:
  - the broad stdcell/tap PG problem is much smaller now
  - the remaining blocker is a narrow **top-edge VDD row-fragment stitch problem**, not generic signal routing and not ordinary macro hookup

### Update: `corePin -> ring` Experiment Did Not Beat The Stronger Baseline
- I also tested a `soc_design`-style simplification inside `pg_stdcell`:
  - `sroute -connect {corePin}`
  - `-corePinTarget {ring}`
  - instead of `-corePinTarget {stripe ring}`
- Result:
  - `Number of Core ports routed: 3942 open: 4788`
  - connectivity summary:
    - `105` terminal opens
    - `230` special-wire opens
    - `665` regular-routing opens
- This was **worse overall** than the stronger `stripe/ring` baseline, because it reduced some regular fragments but exploded the special-wire side and left many more open core ports.
- Conclusion:
  - keep the stronger `corePin -> stripe/ring` stdcell-only baseline
  - do **not** use pure `corePin -> ring` as the default RSD fix path

### Current Backend Status After These PG Experiments
- Best current understanding:
  - macros are **not** the first-order blocker anymore
  - the remaining stdcell-only PG issue is a persistent top-edge `VDD` row-stitch pattern
  - the baseline with the strongest evidence remains the improved `pg_stdcell` flow with `corePin -> stripe/ring`, not the pure ring-target variant

### Update: Top-Edge Failure Pattern Is On The Tap Pitch, Not Random PG Congestion
- I parsed the remaining `pg_stdcell` regular-routing opens and the pattern is highly structured:
  - all `903` regular opens are on `VDD`
  - the first row is:
    - `(1213.620, 1213.296) (1214.580, 1213.872)`
    - `(1188.780, 1213.296) (1189.740, 1213.872)`
    - `(1158.810, 1213.296) (1159.770, 1213.872)`
    - and so on at about `29.97um` pitch
- That pitch matches the tap-cell placement interval, and the open rectangles line up with top-row power-rail/tap locations rather than arbitrary mesh gaps.
- So the late-stage diagnosis is tighter now:
  - the current stdcell-only PG blocker is **top-row rail capture**, especially `VDD`
  - this is more specific than “macro PG problem” or “general MINSTEP problem”

### Update: Two Follow-Up Fixes Were Tested And Rejected
- `pg_stdcell` with an added staged low-layer `corePin -> stripe` pass:
  - result became worse overall:
    - `181` terminal opens
    - `2` special-wire opens
    - `817` regular-routing opens
  - interpretation:
    - it reduced some regular fragments
    - but created too many new terminal opens, mostly `VSS` tap/physical terminals
  - conclusion:
    - do **not** use the staged low core-pin pass as the default stdcell-only fix
- `pg_stdcell` with a VDD-only top-edge low-metal capture strap:
  - no measurable change from the better baseline
  - still:
    - `94` terminal opens
    - `3` special-wire opens
    - `903` regular-routing opens
  - conclusion:
    - the issue is not solved by a small local top-edge helper alone

### Update: Stronger Interpretation Of The `soc_design` Lesson
- The earlier split experiment disabled macro PG hookup, but the macros were still physically present and still fragmented rows.
- Since the tap-pitch/top-row pattern survives that experiment, the next faithful version of the `soc_design` method should be:
  - build a **true macro-less control netlist**
  - run backend on that macro-less design
  - prove whether the row/tap PG skeleton can become clean without any SRAM macros physically present
  - then reintroduce the SRAM macros afterward
- I updated the DC flow to make cache SRAM macro use selectable by environment:
  - `RSD_DC_USE_CACHE_SRAM=0` removes `RSD_USE_TSMC_CACHE_SRAM` from synthesis defines
- Next backend control experiment:
  - synthesize a real no-cache-macro netlist
  - use that to test whether a truly macro-less placement/PG baseline closes cleanly

### `soc_design` Sequence Confirmed From Git History
- I checked two key `soc_design` commits to confirm the exact order:
  - `f45de395041ada24391582bc3448965b768f73d4`
  - `f11b0ecadc93f1b0db6067acbaca42e79e7a3373`
- `f45de39` is the true **no-SRAM baseline** milestone:
  - `rtl/sram.sv` was changed so synthesis no longer instantiated the TSMC hard macro
  - the memory functionality stayed in RTL as synthesizable logic
  - `complete_flow_with_qrc.tcl` was used as the clean backend flow
  - `AGENTS.md` and `DESIGN.md` explicitly recorded this as the no-SRAM `0 DRC / 0 LVS-connectivity` baseline
- `f11b0ec` is the later **with-SRAM reintroduction** milestone:
  - `rtl/sram.sv` restored the hard macro path under `SYNTHESIS`
  - `syn_complete_with_tech.tcl` added checks to require `u_sram/u_sram_macro` of `TS1N16ADFPCLLLVTA512X45M4SWSHOD`
  - `complete_flow_with_qrc_with_sram.tcl` added the dedicated macro-aware Innovus flow

### What This Means For RSD
- The useful lesson from `soc_design` is not “finish macro-less and stop there.”
- The real lesson is:
  - first prove the **base stdcell backend flow**
  - then reintroduce SRAM macros in a separate controlled flow
  - then debug only the delta created by macro reintegration
- The no-macro baseline does **not** directly become the final macro layout.
- Its value is:
  - proving the base PG/row/tap/filler recipe is sound
  - giving a known-good reference
  - shrinking the later debug space when macros are added back

### Current Direction For RSD Backend Closure
- For the real RSD implementation, the mainline flow should keep the SRAM macros present from the start.
- The no-macro flow is still useful as a control experiment, but it is **not** the production direction.
- Current diagnosis:
  - the remaining blocker is not timing
  - the remaining blocker is not ordinary signal routing
  - the blocker is the macro-present PG/special-net topology, especially row-rail/tap stitch disruption near macro-heavy regions
- Important working assumption now:
  - this is more a **macro placement + PG topology** problem than a route-effort problem

### Planned Next Stage
- Keep macros present in the real backend flow.
- Stop treating the no-macro path as the main closure target.
- Move to a more deliberate macro-driven closure strategy:
  - manually place the cache/predictor macros instead of relying on the current coarse arrangement
  - align macro placement to the PG strap geometry
  - give wider clean channels / halo around macro groups
  - avoid placing macro edges in the regions where top-row rail capture is already weak
- Use a simpler macro PG strategy:
  - prefer `blockPin -> stripe/ring`
  - avoid per-macro block rings unless a specific macro proves it needs one
- Workflow for the next iterations:
  1. update macro placement / halo / spacing
  2. rerun from early backend checkpoints (`prepg` / `place`)
  3. check PG special connectivity first
  4. only after special-net connectivity is near clean, proceed again to `cts`, `route`, DRC, and final connectivity checks

### Key Lesson Right Now
- The next high-value change is **manual macro floorplanning**, not more generic route-option tuning.

### Update: Manual Macro Floorplan Is Now The Active Fix Path
- I refactored the RSD Innovus flow so macro placement is no longer a coarse static pattern.
- The current `innovus_flow.tcl` now:
  - snaps macro columns to the PG strap grid
  - uses a more deliberate banded floorplan for cache and predictor macros
  - drives the low helper mesh from the **actual placed macro groups** instead of stale hardcoded windows
- This matters because the earlier helper mesh was tuned for an old macro arrangement; once the macros moved, the PG helper geometry no longer overlapped the real problem regions.

### Update: First Manual-Floorplan Result Confirmed The Direction
- After switching to the new grid-aligned manual macro placement plus dynamic helper mesh, the low-layer `corePin` PG pass improved materially:
  - `Number of Core ports routed: 3238 open: 17346`
- This was a real improvement over the first broken manual-floorplan attempt, which had:
  - `Number of Core ports routed: 0 open: 20594`
- Interpretation:
  - macro geometry and PG helper alignment are tightly coupled
  - once the helper mesh followed the real macro positions, PG access improved immediately

### Update: Simply Making The Global Macro Spacing Larger Did Not Solve It
- I then widened the cache/predictor row spacing further and increased macro halo.
- The next measured low-layer `corePin` result was:
  - `Number of Core ports routed: 3441 open: 17927`
- This is **not** a clear improvement over the earlier `3238 / 17346` result.
- Interpretation:
  - blindly adding more whitespace is not enough
  - the remaining problem is more localized to the specific channels between the stacked cache macro rows

### Update: The Current Fix Focus Is Now The Inter-Row Cache Channels
- I added a new `rsd_add_pg_row_channel_stitch` step in the Innovus flow.
- This injects low-layer PG ladders specifically in the narrow channels between:
  - the `dCache` tall data row and the `dCache` short tag row
  - the `iCache` tall low-bank row and the `iCache` short high-bank row
- The intent is to give the row rails and low-layer PG a deliberate bridge **exactly** where the macro bands are fragmenting them, rather than applying broader global helpers.
- The new stitch step built cleanly during `make init`, and Innovus reported that it actually created local geometry in those channels:
  - extra M5/M6 wires
  - extra M5-M6 / M6-M7 vias
- The next measurement point is to rerun `place` with this new channel-stitch geometry and compare it against the current best low-layer `corePin` checkpoint (`3238 routed / 17346 open`).

### Current Tactical Conclusion
- The manual macro-floorplan path is still the right direction.
- The evidence now says:
  - **macro alignment to the strap grid matters**
  - **helper mesh must follow the real macro coordinates**
  - **general extra whitespace is not sufficient**
  - the next useful geometry change is **surgical PG stitching inside the cache-row channels**

### Update: Wider Halo And Wider Cache-Band Gaps Were Not The Right Macro Fix
- I reran the macro-present flow with:
  - widened cache/predictor row spacing
  - wider macro halo
  - an additional low-layer cache-row channel stitch
- The low-layer `corePin` checkpoint still landed at:
  - `Number of Core ports routed: 3441 open: 17927`
- That matched the earlier widened-gap result and did **not** improve the main PG access problem.
- Interpretation:
  - the extra low-layer stitch was creating real M4/M5/M6/VIA work in the cache channels
  - but it was not changing the main core-port accessibility metric
  - so that patch should not be the default macro-present path

### Update: Smaller Halo Restored A Better Macro-Present PG Baseline
- I then reverted toward the better manual-floorplan baseline:
  - smaller macro halo
  - smaller cache/predictor row gaps
  - row-channel stitch disabled by default
- That materially changed the PG behavior.
- The new macro-present low `corePin` result became:
  - `Number of Core ports routed: 1602 open: 16854`
- The total port accounting changed because the fixed tap/physical-cell picture changed, so the raw routed count is not directly comparable to the earlier widened-gap run.
- The more important number here is the open count:
  - `16854` is lower than the widened-gap `17927`
- Interpretation:
  - the earlier “bigger halo + bigger channels” direction was over-fragmenting rows
  - the smaller-halo macro baseline is the better foundation for the next macro-present fixes

### Update: Short-Row Y Phase Shift Helps Only Marginally
- I next tested a macro-present variant with:
  - the same smaller-halo baseline
  - plus a `40um` Y-phase shift for the short macro rows
- Result:
  - `Number of Core ports routed: 1598 open: 16836`
- This is only a marginal improvement over `1602 / 16854`.
- Interpretation:
  - short-row Y alignment is part of the repeated `VIA5/VIA7` conflict pattern
  - but it is not the main lever

### Current Macro-Present Direction Tightened Further
- The more promising structural next move is to **stagger the short cache rows in X** so the upper tag/high-bank rows are no longer directly stacked over the lower data/low-bank rows.
- Reason:
  - the remaining macro-present warnings are still highly regular
  - they cluster around the stacked cache-row bands
  - changing only Y phase does not move the core-port result enough
- I added an explicit control knob for this in the Innovus flow:
  - `RSD_CACHE_SHORT_X_SHIFT`
- The current live experiment is:
  - keep the better smaller-halo macro-present baseline
  - stagger the short cache rows by `40um` in X
  - rerun `init -> place`
  - compare the low `corePin` result against the current best open-count baseline (`16836`)

### Update: X-Staggering The Short Cache Rows Is The First Real Macro-Present PG Improvement
- With `RSD_CACHE_SHORT_X_SHIFT=40`, the macro-present `place` flow improved materially.
- The key `sroute` checkpoints were:
  - first `corePin -> stripe`: `2487 routed / 15969 open`
  - second `corePin -> stripe ring`: `15191 routed / 822 open`
- The post-place special-connectivity report still hit the `1000`-violation cap, but the remaining failures were no longer broad macro collapse.
- The surviving problem narrowed to:
  - repeated `TAP` terminal opens in a handful of row bands
  - a smaller set of special-wire PG breaks clustered around the cache/predictor region

### Update: Remaining Macro-Present Opens Are Now Band-Localized
- I parsed the best `place_special_connectivity.rpt` and the terminal opens now cluster in repeated Y bands rather than across the whole macro region.
- The most obvious surviving bands are:
  - full-width `VSS` at the bottom and top rows
  - repeated `VDD` bands around `211`, `428`, `497`, `537`, `662`, `937`
  - repeated `VSS` bands around `622`, `778`, `902`, `1062`
- Interpretation:
  - the X-staggered floorplan is good enough that broad helper meshes are no longer the right tool
  - the next PG work should target these exact row/tap bands directly

### Update: Added A Surgical Tap-Band PG Capture Ladder
- I patched the Innovus flow with a new `rsd_add_pg_tap_band_capture` helper.
- This adds local low-layer `M4/M5/M6` capture ladders only in the exact row bands that survived the current best X-stagger run.
- The intent is to give the affected tap/followpin rows a nearby capture path into the existing PG mesh without going back to a broad global low-layer mesh.
- I rebuilt `init` successfully with this patch on the same `RSD_CACHE_SHORT_X_SHIFT=40` floorplan.
- The matching `place` rerun was started and then intentionally stopped so the current backend-debug state could be committed cleanly before the next measurement.

### Update: Current Best Macro-Present PG Result And Ruled-Out Branches
- I continued the macro-present PG debug from the `RSD_CACHE_SHORT_X_SHIFT=40` checkpoint and used per-pass connectivity reports to isolate what each `sroute` stage is actually fixing.
- Current best useful checkpoints are:
  - after low `corePin -> stripe`: `270` terminal opens + `730` special-wire opens
  - after an added final `corePin -> ring` edge catch-up pass: `118` terminal opens + `882` special-wire opens in the immediate edge report
- Interpretation:
  - the broad edge-ring pass is genuinely fixing a large chunk of the terminal-open problem
  - but it is also creating too much new special-wire fragmentation to keep as-is

### Update: What I Ruled Out In This Round
- I tested a mid-layer `corePin -> stripe` lift (`M5..M8`) between the low pass and the edge-ring pass.
- It did nothing useful:
  - `Number of Core ports routed: 0 open: 15739`
- I also tested broader boundary and phase-2 helper strategies:
  - boundary/top-edge low mesh in the macro-present `pg` path
  - VSS-only low-layer phase-2 capture
  - VDD-only low-layer phase-2 capture
  - VDD-only upper-metal hotspot bridge assist
- All of those branches were rejected because they reintroduced highly intrusive `IMPPP-133` boundary-expansion behavior or regressed the PG checkpoint back toward `1000` terminal-open failures before the low pass finished.

### Current Understanding Of The Remaining Problem
- The live macro-present problem is now much narrower than before:
  - the low `corePin` pass is the stable baseline
  - a broad edge-ring pass helps terminals but over-fragments special wires
  - extra low-layer capture ladders are too invasive
- So the next useful direction is not “more low-layer mesh.”
- The next useful direction is:
  - keep the current X-staggered macro floorplan
  - keep the good low `corePin` baseline
  - replace the broad edge-ring pass with a narrower upper-level reconnection around only the surviving boundary/cache bands

### Update: VSS-Only Edge Ring Is Real, But VDD Still Needs A Narrower Fix
- I reran the macro-present `pg` stage on the current best X-staggered floorplan and confirmed the low-pass baseline still reproduces exactly:
  - `270` terminal opens
  - `730` special-wire opens
- I then ran a `VSS`-only edge-ring pass on top of that baseline.
- Result:
  - `113` terminal opens
  - `887` special-wire opens
- Breakdown from the immediate edge report:
  - surviving terminals are almost entirely `VDD`
  - `VSS` terminal opens collapse from `157` down to `1`
  - the special-wire problem becomes almost entirely `VDD`
- This proves the edge-ring machinery itself is not useless. It is strongly effective for `VSS`, but too broad for `VDD`.

### Update: VDD-Only Broad Edge Stripe Is Not The Right Follow-Up
- I tested a `VDD`-only `corePin -> stripe` edge pass as a possible complement to the `VSS`-only ring pass.
- That branch was not useful:
  - it was much slower than the `VSS` edge pass
  - it hit repeated `IMPPP-570` and `IMPPP-531` cut-layer obstruction / via-spacing warnings in the cache windows
  - it did not produce a clean immediate improvement signal quickly enough to justify keeping it as the next mainline path
- Conclusion:
  - `VDD` should not be fixed with another broad edge `sroute`
  - `VDD` needs a narrower helper than a full edge-side reconnect

### Update: Added Exact-Band VDD Upper Reconnect Helper
- I added `rsd_add_pg_vdd_upper_band_reconnect` to the Innovus flow.
- The helper adds only narrow `M10` `VDD` bridges in the exact surviving `VDD` bands from the current best report:
  - bottom cache around `y ≈ 211/428/497`
  - right cache around `y ≈ 537/662`
  - predictor band around `y ≈ 937`
- I also changed the flow so this helper can be inserted **after** the stable low `corePin` pass and **before** the optional edge pass, instead of perturbing the whole low-pass baseline from the start.
- The reordered branch has been wired into the flow, but I stopped the long run once it was clear it had not yet produced a new report before the next decision point.

### Current Best Practical Conclusion
- The strongest proven branch is still:
  - X-staggered macro floorplan
  - stable low `corePin -> stripe` baseline
  - `VSS`-only edge-ring catch-up if we specifically want to eliminate the `VSS` terminal-open side
- The remaining open technical problem is now very specific:
  - find a narrow `VDD` reconnect that preserves the `VSS` win without causing the `VDD` special-wire explosion from the broad edge-ring path

### Update: Removed The Main Brute-Force Bottleneck In The PG Debug Flow
- The real inefficiency in the current backend work was not just the choice of PG experiments. It was that every targeted `VDD`/`VSS` reconnect experiment still had to replay the full macro-present low-pass PG baseline from `prepg.enc`.
- I patched the flow to add explicit checkpoint stages for that split:
  - `pg_low`
  - `pg_edge`
- New checkpoints in the Innovus flow:
  - `pg_low.enc`
  - `pg_edge.enc`
- The intent is:
  - build the stable low-pass macro-present baseline once
  - branch all narrow edge/reconnect experiments from that saved baseline
  - stop spending multi-minute runtime on the same low `corePin` stage for every hypothesis
- I also added a dedicated `rsd_pg_sroute_edge_only` path so the edge-side reconnect can be tested independently from the low-pass stages.

### Update: Macro-Present PG Is Now Reduced To A Pure `VDD` Special-Wire Problem
- I added finer-grain controls for the macro-present `pg_vdd` stage:
  - selectable `VDD` upper-band reconnect windows
  - selectable `VDD` column reconnect windows
  - selectable `VSS` phase-2 capture windows
  - env controls to skip the helper-local `editPowerVia` passes so stripe geometry can be tested independently from global via stitching
- The important finding is that the helper stripes themselves, not just the via pass, are what move the result:
  - `pred_col_left` with helper-local vias disabled still changes the report
  - so the added geometry is real, not just a side effect of global via insertion
- The clearest current branch is:
  - keep the macro-present `pg_edge` baseline
  - add only `pred_vdd_top`
  - add only `upper_vss_p2_b` and `upper_vss_p2_c`
  - keep helper-local via insertion disabled for both
- Result from that branch:
  - `113` terminal opens
  - `887` special-wire opens
  - surviving terminals are effectively all `VDD` (`VSS` drops to `1`)
  - surviving special-wire opens are entirely `VDD`
- Breakdown from the report:
  - top predictor columns remain the dominant `VDD` issue
  - the dcache band is still present, and a new lower-left cache-side `VDD` band appears
  - but the mixed `VDD/VSS` special-wire failure mode is gone in this branch
- This is not final closure, but it is still a meaningful reduction:
  - the macro-present PG problem is now a **single-net** reconnect problem
  - the next targeted work should stay on narrow `VDD` reconnect in the remaining cache/predictor bands, not broad mixed-net PG repair

### Update: The Remaining `VDD` Terminals Are Full-Width Row-Band Opens, Not Small Hotspots
- I ran a focused sweep of the narrow `VDD` reconnect helpers from the current macro-present `pg_edge` base:
  - `dcache_vdd_low`
  - `dcache_vdd_mid`
  - `dcache_vdd_high`
  - cache-only `VDD` phase-2 capture
- These branches do change the report, but one pattern stayed constant:
  - the stubborn `VDD` terminal count stays at `113`
- Parsing the terminal coordinates shows why:
  - the remaining `VDD` terminals are not a few isolated macro-edge points
  - they are spread across repeated row bands at approximately:
    - `y ≈ 211.5 / 211.7`
    - `y ≈ 428.1 / 428.2`
    - `y ≈ 497.2 / 497.4`
    - plus a smaller predictor-top band around `y ≈ 937`
  - and the `x` locations span most of the cache row width instead of one narrow hotspot
- Practical consequence:
  - more upper-metal hotspot bridges alone are not enough
  - the next `VDD` fix should be a **row-aligned capture** for those exact `y` bands
  - most likely with a shifted/additional `x` phase, because the current `VDD` phase-2 capture windows cover the right `y` ranges but still miss the actual terminal lattice
- Best current sub-results from this sweep:
  - `dcache_vdd_high` alone:
    - `113` `VDD` + `62` `VSS` terminal opens
    - `826` special-wire opens, all `VDD`
  - cache-only `VDD` phase-2 capture:
    - `113` `VDD` + `130` `VSS` terminal opens
    - `758` special-wire opens, all `VDD`
  - enabling local vias on the cache-only `VDD` phase-2 capture did **not** fix the stubborn `113` `VDD` terminals
- So the current best interpretation is:
  - the remaining problem is a row-stitch alignment problem on `VDD`
  - not a missing global PG feature and not a mixed `VDD/VSS` closure problem anymore

### Update: Low-Layer `VDD` Row-Attach Ladder Reaches The Right Layer But Not Final Closure
- I added a new exact-band `VDD` row-attach helper in the macro-present `pg_vdd` flow:
  - `M3` horizontal row capture
  - `M4` vertical ladders on the exact repeated `VDD` x phases
  - then a `VDD`-only `corePin -> stripe` `sroute` pass from `M1..M4`
- This is materially different from the older `M4/M5/M6` helper:
  - the new branch actually routed real low-layer attach wires in `sroute`
  - the earlier `M1..M4` attach attempt on the old helper routed `0` wires
- Result from the new low-layer row-attach branch:
  - `114` terminal opens total
    - `113` on `VDD`
    - `1` on `VSS`
  - `887` special-wire opens, all on `VDD`
- So the branch preserves the clean single-net picture:
  - `VSS` is effectively solved
  - the remaining problem is still entirely `VDD`
- The important new diagnosis is:
  - low-layer row attachment is now proven necessary, but it is not sufficient by itself
  - the remaining blocker is the same small set of `VDD` vertical trunk families, not the row-band identification anymore
- Current surviving `VDD` special-wire trunk families after the low-layer attach:
  - `x = 663.247..704.933` (`213`)
  - `x = 583.237..624.923` (`213`)
  - `x = 503.227..544.913` (`213`)
  - `x = 343.297..384.893` (`187`)
  - `x = 263.287..304.883` (`60`)
  - plus one full-width bottom-band artifact
- Practical conclusion:
  - the next fix should not be another broad row-capture change
  - it should target those surviving `VDD` trunk families directly, now that the low-layer row attachment has been proven to work as far as it can

### Update: Explicit `VDD` Floating-Stripe Stitch Finally Moves Off The 887 Plateau
- I added an explicit `VDD` floating-stripe stitch pass to the macro-present `pg_vdd` stage in `Processor/Project/Innovus/innovus_flow.tcl`.
- This follows the same basic idea that worked in `soc_design`: once helper stripes are inserted, they still need a dedicated `sroute -connect {floatingStripe}` pass instead of relying only on `corePin` attachment.
- This was the first branch that actually moved the long-stalled macro-present `VDD` result:
  - old plateau:
    - `113` terminal opens
    - `887` special-wire opens
  - with explicit `VDD` floating-stripe stitch:
    - `138` terminal opens
    - `862` special-wire opens
- The detailed report parse shows the useful part clearly:
  - `VDD` terminals stayed flat at `113`
  - `VDD` special-wire opens dropped from `887` to `863`
  - the extra terminal count came from now-visible `VSS` terminals, not a new `VDD` terminal regression
- So this is real progress:
  - the flow was leaving useful `VDD` helper stripes floating
  - the remaining `VDD` special-wire problem can be reduced further by explicit stripe stitching

### Update: What Does Not Stack On Top Of The New Floating-Stripe Branch
- I tested a final `VSS` edge catch-up on top of the new `VDD` floating-stripe branch.
- That was a bad branch:
  - it collapsed back to the old mixed-net state
  - final result became `270` terminal opens and `730` special-wire opens
- I also tested the predictor-only mid-layer (`M5`) `VDD` trunk bridge on top of the floating-stripe branch.
- That did **not** provide any further benefit:
  - the result stayed `138` terminal opens and `862` special-wire opens
- So the current conclusion is:
  - explicit `VDD` floating-stripe stitch is useful
  - reintroducing the old broad `VSS` edge cleanup on top of it is not
  - the current predictor `M5` trunk-bridge geometry is not the missing piece

### Current Best Branch After This Round
- macro-present flow
- X-staggered macro floorplan
- `pg_edge` baseline
- exact-band low-layer `VDD` row attach
- explicit `VDD` floating-stripe stitch
- `VDD`-only `corePin -> stripe` attach

### Current Best Practical Interpretation
- the first real reduction in the residual `VDD` problem came from explicitly stitching floating `VDD` helper stripes
- the remaining work should stay on that branch
- the next useful experiments should focus on the still-open `VDD` families directly, instead of bringing back broad mixed-net cleanup passes

### Update: Floating-Stripe Target Mode Matters More Than Raising The Layer Range
- I tested whether the explicit `VDD` floating-stripe stitch could be made less intrusive by raising the bottom layer from `M3` to `M4`.
- That was a dead branch:
  - `M4..M10` floating-stripe stitch fell all the way back to the old low-layer plateau
  - result: `113` terminal opens + `887` special-wire opens
- So the helpful effect is tied to reaching `M3`; simply lifting the stitch higher removes the gain.

- I also tested the three meaningful `floatingStripeTarget` modes for the `M3..M10` `VDD` stitch:
  - `ring stripe`:
    - `138` terminal opens
    - `862` special-wire opens
    - still the best balanced branch so far
  - `stripe`:
    - `153` terminal opens
      - `113` on `VDD`
      - `40` on `VSS`
    - `848` special-wire opens, all on `VDD`
    - this is the strongest raw `VDD` special-wire improvement so far
  - `ring`:
    - fell back to `113` terminal opens + `887` special-wire opens
    - no meaningful gain

- Important interpretation from these target-mode tests:
  - the useful `VDD` improvement comes from stripe-side stitching, not ring-only stitching
  - but a pure `stripe` target exposes a localized `VSS` terminal problem
  - the `ring stripe` branch remains the safer macro-present baseline because it keeps the `VDD` gain while containing the `VSS` fallout

### Update: Bottom-Row `VSS` Repair Is Sensitive To Stage Ordering
- I added a post-`VDD` hook in `Processor/Project/Innovus/innovus_flow.tcl` so the bottom-row `VSS` helper can be inserted *after* the `VDD` branch is built, instead of only before it.
- This was needed because the pre-`VDD` `VSS` bottom-row helper can interfere with the `VDD` floating-stripe gain.
- First confirmed result:
  - on the aggressive `stripe`-only `VDD` branch, adding the bottom-row `VSS` helper after the `VDD` passes still collapsed the result back to `270` terminal opens + `730` special-wire opens
- So the current conclusion is:
  - bottom-row `VSS` repair is still a real issue
  - but the current `addStripe`-style `VSS` helper is too intrusive when stacked on top of the `VDD` improvement path
  - the better mainline remains a macro-present `VDD`-driven branch, not a mixed-net cleanup branch

### Update: Pivoted To The `soc_design`-Style Simple Macro Flow
- I added a separate simple macro-present flow in:
  - `Processor/Project/Innovus/innovus_flow.tcl`
  - `Processor/Project/Innovus/Makefile`
- The simple path keeps the SRAM macros present, but removes the custom PG scaffolding:
  - core ring only
  - manual macro placement
  - small `M5/M6` signal halo with `-exceptpgnet`
  - one simple `sroute -connect {corePin blockPin} -corePinTarget ring -blockPinTarget ring`
- First clean result from `out_simple/`:
  - `158` terminal opens
    - `157` on `VSS`
    - `1` on `VDD`
  - `843` special-wire opens
    - all on `VSS`
- This is a meaningful simplification versus the earlier complex branch:
  - the residual problem is no longer mixed or `VDD`-dominated
  - the simple flow reduces the problem to an almost pure `VSS` closure issue

### Update: Cheap Simple Reroutes Help Specials, But Reintroduce Mixed-Net Problems
- I added a `simple_pg` stage so simple-flow experiments can restore a saved `prepg` checkpoint and rerun only the simple `sroute`, instead of paying for a full `place` every time.
- I also fixed the simple Makefile targets so they can write to isolated output trees instead of hardwiring `out_simple/`.
- Two cheap reroute experiments are now characterized:
  - `out_simple_vss/`:
    - extra simple `VSS` reroute after the simple baseline
    - result:
      - `270` terminal opens
      - `731` special-wire opens
      - net split:
        - terminals: `157 VSS`, `113 VDD`
        - specials: `362 VSS`, `369 VDD`
  - `out_simple_vss_vdd/`:
    - extra simple `VSS` reroute followed by extra simple `VDD` reroute
    - result:
      - `270` terminal opens
      - `730` special-wire opens
- Interpretation:
  - extra simple reroutes can reduce the raw special-wire count a lot (`843 -> ~730`)
  - but they destroy the clean almost-pure-`VSS` baseline and bring `VDD` back into the problem
  - so the reroute-only path is informative, but not yet a new mainline

### Current Simple-Flow Mainline And Next Lever
- The plain `out_simple/` baseline is still the safer simple mainline:
  - `158` terminals
  - `843` specials
  - almost entirely `VSS`
- Since the cheap reroute-only experiments did not preserve that clean net split, the next simple-compatible lever should be geometry, not more PG passes.
- I parameterized the bottom cache-row placement in `Processor/Project/Innovus/innovus_flow.tcl` with:
  - `RSD_MACRO_BOTTOM_GUARD` (default `140.0`)
- I then started a geometry-only simple-flow experiment in `out_simple_bg180/` with:
  - `RSD_MACRO_BOTTOM_GUARD=180`
- Goal of this branch:
  - move the lower cache macro band upward
  - see whether the stubborn bottom/cache-side `VSS` row bands are being caused by the current macro-row placement

### Update: Three More Simple-Flow Branches, None Better Than `out_simple`
- I tested a cache short-row `X` stagger inside the simple macro-present flow:
  - branch: `out_simple_x40/`
  - knob: `RSD_CACHE_SHORT_X_SHIFT=40`
  - result:
    - `270` terminal opens
      - `113` on `VDD`
      - `157` on `VSS`
    - `730` special-wire opens
      - `577` on `VDD`
      - `154` on `VSS`
- Conclusion:
  - the short-row `X` stagger that helped earlier PG-debug work is **not** a good fit for the current simple baseline
  - in the simple flow it destroys the clean almost-pure-`VSS` split and turns the problem back into a mixed `VDD/VSS` failure

- I also tested exact-column `VSS` stripes from the saved simple `prepg` checkpoint:
  - branch: `out_simple_vss_cols/`
  - added only `VSS` `M9` vertical stripes at the seven repeated open columns from the `out_simple` report
  - then stitched those stripes to the ring with a `floatingStripe -> ring` pass
  - result:
    - `270` terminal opens
      - `113` on `VDD`
      - `157` on `VSS`
    - `730` special-wire opens
      - `362` on `VDD`
      - `369` on `VSS`
- Conclusion:
  - this recreated the same mixed-net failure shape as the earlier broad `VSS` reroute
  - it also triggered a flood of `IMPPP-133` boundary-expansion warnings on taps/macros along those columns
  - so this exact-column stripe style is a dead branch for the simple mainline

- Finally, I matched the simple RSD flow more closely to the final `soc_design` geometry:
  - branch: `out_simple_socgeom/`
  - knobs:
    - `RSD_FP_MARGIN_ALL=50`
    - `RSD_RING_WIDTH=2.0`
    - `RSD_RING_SPACING=2.0`
    - `RSD_RING_OFFSET_ALL=5.0`
  - result:
    - `270` terminal opens
      - `113` on `VDD`
      - `157` on `VSS`
    - `730` special-wire opens
      - `381` on `VDD`
      - `350` on `VSS`
- Conclusion:
  - simply copying the `soc_design` floorplan margins and lighter ring is **not** enough for RSD
  - it again collapses the clean simple baseline into a mixed-net problem

### Current Best Simple Mainline After These Tests
- `out_simple/` remains the best simple macro-present baseline:
  - `158` terminal opens
    - `157` on `VSS`
    - `1` on `VDD`
  - `843` special-wire opens
    - all on `VSS`
- Practical interpretation:
  - the current best state is still the original simple branch, because every tested follow-up that lowered the raw special-wire total also reintroduced a broad `VDD` problem
  - the next useful fix should therefore stay anchored to `out_simple/` and avoid:
    - cache short-row `X` staggering
    - exact-column `VSS` stripes
    - blindly copying the `soc_design` ring/margin geometry into RSD

### Boundary-Only And Phase-2 `VSS` Fixes From `out_simple/place.enc`
- I then switched to delta-only `simple_pg` branches restored from the already
  cleanest simple checkpoint, `out_simple/db/place.enc`, to see whether the
  residual `VSS` problem could be fixed without rerunning the full simple
  `sroute` and perturbing `VDD`.

- Corrected phase-2 `VSS` helper with `M6` attach:
  - branch: `out_simple_vss_phase2_m6/`
  - knobs:
    - `RSD_SIMPLE_SKIP_BASE_SROUTE=1`
    - `RSD_SIMPLE_ADD_VSS_PHASE2_CAPTURE=1`
    - `RSD_SIMPLE_ENABLE_VSS_COREPIN_ATTACH=1`
    - `RSD_VSS_COREPIN_TOP_LAYER=M6`
  - result:
    - `250` terminal opens
    - `750` special-wire opens
  - conclusion:
    - this was the first version where the `VSS` phase-2 helper was actually
      visible to `sroute`
    - but it still collapsed the design into a mixed-net shape, so it is not a
      better simple mainline

- Boundary-only `VSS` row attach with local `M3/M4` lattice:
  - branch: `out_simple_vss_boundary/`
  - result:
    - `270` terminal opens
      - `157` on `VSS`
      - `113` on `VDD`
    - `730` special-wire opens
      - `364` on `VSS`
      - `367` on `VDD`
  - conclusion:
    - even a narrow boundary-local `VSS` patch on top of `out_simple/place.enc`
      collapses into the same `270 / 730` mixed-net state

- Boundary-rails-only `VSS` helper:
  - branch: `out_simple_vss_rails/`
  - knobs:
    - only the top/bottom horizontal `VSS` rails
    - no vertical boundary post lattice
  - result:
    - `270` terminal opens
      - `157` on `VSS`
      - `113` on `VDD`
    - `730` special-wire opens
      - `529` on `VSS`
      - `202` on `VDD`
  - conclusion:
    - this is the cleanest of the post-`out_simple` repair branches because it
      preserves most of the `VSS` special-wire reduction while introducing less
      `VDD` damage than the other mixed branches
    - but it still does not beat the plain `out_simple` baseline as a new
      place-stage checkpoint

- Exact-band `VDD` cleanup on top of `out_simple_vss_rails/`:
  - branch: `out_simple_vss_rails_vdd/`
  - knobs:
    - `RSD_SIMPLE_ADD_VDD_ROW_ATTACH=1`
    - `RSD_VDD_ROW_ATTACH_NAMES='cache_row_low cache_row_mid cache_row_high pred_row_top'`
    - `RSD_SIMPLE_ENABLE_VDD_COREPIN_ATTACH=1`
  - result:
    - `270` terminal opens
      - `157` on `VSS`
      - `113` on `VDD`
    - `730` special-wire opens
      - `394` on `VSS`
      - `337` on `VDD`
  - conclusion:
    - the exact-band `VDD` attach did not improve the total shape
    - it only redistributed the `730` special-wire errors between `VSS` and `VDD`

### Updated Interpretation
- Place-stage `VSS` repair on the simple macro-present baseline now looks
  plateaued:
  - `out_simple/` stays the least bad mainline at place stage
  - every delta-only `VSS` helper restored from `out_simple/place.enc` still
    falls back into the same `270 / 730` mixed-net family once it tries to fix
    the residual `VSS` rows
- The next step should therefore follow the actual `soc_design` closure path:
  - stop over-iterating place-stage PG fixes
  - push the plain simple baseline through `CTS -> route -> ECO`
  - then evaluate post-route DRC and regular/special connectivity on the routed
    design instead of demanding a fully clean place-stage PG report first

### Routed Checkpoint: Simple Macro-Present Flow Is the Real Mainline
- I pushed the plain `out_simple/` branch through `CTS -> route -> ECO` all the
  way to a routed checkpoint.
- `CTS` completed cleanly enough to proceed:
  - clock tree built for `158717` sinks
  - the residual CTS route DRC was already small (`4` geometry violations)
- The full `simple_route` run then completed and saved:
  - `route_clean.enc`
  - `route.enc`
  - routed reports under `Processor/Project/Innovus/out_simple/reports/`

### Final Routed Status From `out_simple/`
- Regular routing/connectivity is now clean:
  - `route_connectivity.rpt`: `0` violations
- The remaining routed DRC is narrow and stable:
  - `route_drc.rpt`: `286` total
  - all `286` are `SHORT` violations
  - all are on `M5/M6`
- The remaining special-net problem is also narrow:
  - `route_special_connectivity.rpt`: `1000` capped violations
  - almost all visible entries are `Net VDD ... /VPP` unconnected terminals
  - only `1` visible `VSS ... /VBB` terminal appears before the report hits the
    `1000` cap

### What This Means
- The simple macro-present flow is now the clear production mainline.
- The broad early PG problem is no longer the blocker in the old sense:
  - we have a full routed checkpoint
  - regular routing is already clean
- The remaining closure task is now two-stage and very specific:
  1. fix routed `M5/M6` shorts, which are concentrated around routing blockages
     in the predictor / BTB region
  2. then fix the residual `VDD/VPP` special-net closure from the routed
     checkpoint

### Updated Next Step
- Do **not** go back to broad place-stage PG experimentation.
- Stay on the routed `out_simple/` checkpoint and fix:
  - `DRC` first
  - then special-net `VPP/VBB` connectivity
- The practical next move is to locally relax or refine the offending
  `M5/M6` simple signal-halo blockage pattern around predictor / BTB, reroute,
  and then perform a routed-db `VPP/VBB` cleanup pass.

### DRC Closure On `out_simple`
- The routed `286` DRC issue was fully root-caused.
- On the routed `route_clean.enc` checkpoint, deleting the synthetic
  `sig_halo_*` M5/M6 signal-halo route blockages drops routed DRC from `286`
  to `0`.
- This proved the remaining routed DRC was not true wire-wire damage. It was
  regular wire intersecting the temporary simple-flow macro signal halos.
- The Innovus flow was updated so the simple-flow route-report stage removes
  `sig_halo_*` before final signoff-style DRC reporting.
- Re-running the report stage from `route_clean.enc` now gives:
  - `route_drc.rpt`: `0`
  - `route_connectivity.rpt`: `0`

### Routed Special-Net Truth State
- The old `1000` capped `VPP`-open report was mostly a body-bias accounting
  artifact from binding `VPP/VBB` directly into the final `VDD/VSS`
  connectivity gate.
- Re-running the routed report from `route_clean.enc` with
  `RSD_CONNECT_BODY_BIAS=0` gives the real routed special-net state:
  - `270` terminal opens
  - `730` special-wire opens
- The opens are no longer vague. They cluster into repeated tap/row bands:
  - `VDD`: `y≈211`, `428`, `497`, with smaller bands at `537/662/937`
  - `VSS`: `y≈100`, `622`, `778`, `902`, `1062`, `1234`
- The terminal entries are dominated by explicit `TAP_* /VDD` or `TAP_* /VSS`
  pins, which means the remaining routed problem is a row-rail / tap attach
  problem, not a general signal-routing problem.

### Failed Routed Special-Net Experiment
- A first routed special-net repair experiment was tried from `route_clean.enc`:
  - add exact-band M10 horizontal straps on the routed open bands
  - then run `sroute` to connect floating stripes and core pins
- This did **not** help.
- The log showed:
  - floating-stripe pass created `0` wires
  - core-pin attach routed `0` core ports and left `14514` open
- Conclusion:
  - upper-metal-only band straps are not enough
  - the next meaningful fix must be low-layer exact-band row attachment, not
    more M10-only stitching
