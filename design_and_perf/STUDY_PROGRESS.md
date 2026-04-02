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
