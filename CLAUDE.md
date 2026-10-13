# CLAUDE.md — DC Solid-State Relay (SSR) Capstone

Project context for Claude working in this repo. Status as of 13 Oct 2026.

## Project

- RMIT OENG1168 Engineering Capstone Part B. Team: Hunter White (s4014560) and Liam Smart (s4004985).
- Industry partner: EnGenius (spell it "EnGenius", never "InGenius").
- Supervisors / panel: Stewart Parker, Brendan McGrath, Carlos Teixeira. Weekly supervisor meeting every Monday.
- Goal: self-contained "Smart" DC SSR, 60 V source / 45 A continuous, integrating precharge, isolation, control and protection on a single PCB, and staying safe under fault and post-fault conditions.
- Part A (OENG1167) progress report covered switch selection, current interruption and precharge in simulation. Part B takes it to schematic, PCB, firmware and board testing.

### Deadlines
- Final report: 18 Oct 2026 (Assessment 2, 50%)
- Presentation: 22 Oct 2026 (Assessment 3, 30%). The board will NOT be energised at the presentation event.
- Other outputs: Excel Gantt chart, group contribution table.

## Requirements (cite as R# in the report)

| ID  | Requirement | Acceptance criterion |
|-----|-------------|----------------------|
| R1  | Nominal source voltage | 60 V DC |
| R2  | Continuous conduction | 45 A at 45 °C ambient |
| R3  | Junction temperature | Below 175 °C abs max at rated current and ambient |
| R4  | Bidirectional blocking / reverse polarity | No conduction through the main switch when off, either polarity |
| R5  | Capacitive inrush limiting | Precharge current below 45 A, including mixed R-C loads |
| R6  | Inductive load interruption | Switch V_DS held below 100 V rating at turn-off |
| R7  | Overcurrent protection | Two-tier alarm + trip, in hardware, firmware-independent |
| R8  | Bus OV/UV protection | Relay opens when bus leaves its valid window |
| R9  | Galvanic isolation | Control side isolated; creepage/clearance meet isolator minimums |
| R10 | Fail-safe | Relay defaults open on loss of control power, MCU reset, or bias failure |
| R11 | Form factor | All functions on a single PCB |

## Current state

- Schematic complete (Altium, Full_Schematic.SchDoc). PCB ordered and arrived. Any replacement parts must fit existing footprints.
- Supervisor schematic review items (TPSI NC pins, IPC clearance, track widths, connectors, test points) resolved.
- Firmware: six-state machine on NUCLEO-F042K6 (see Firmware below).

## Architecture (locked)

Five subsystems: main disconnect (includes reverse polarity protection), precharge, OV/UV protection, bias supply, MCU supervisory control.

### Protection philosophy
- Overcurrent: hardware-speed and firmware-independent. TPSI3103 FLT1_CMP acts directly on VDRV regardless of EN, so a firmware hang cannot hold the main FETs on through an overcurrent.
- OV/UV: MCU-mediated. AMC23C14 comparators detect, MCU executes shutdown via EXTI interrupts (not polling). Latency = comparator propagation (~280 ns typ) + MCU interrupt latency.
- Fail-safe defaults:
  - AMC23C14 actively pulls BOTH outputs low (fault state) on loss/brownout of bus-side VDD1, so a dead supply reads as "outside window", never "healthy".
  - TPSI internal EN pull-down (500k typ, 390–640k) keeps the relay open if the MCU loses power, resets, or floats its GPIO. There are NO external EN pull-downs on the board (R109/R207 do not exist; R901/R903 on CCEN/PREEN are 0 Ω links).
- Isolation boundary: REC6K module + TPSI barriers + AMC23C14 barriers. AMC23C14 needs >= 8.5 mm creepage/clearance; TPSI310x needs >= 8 mm.

### 1. Main disconnect (incl. RPP)
- U101 TPSI3103 (non-latching, 1.23 V internal VREF). TPSI3103L latching was out of stock, so latching is done in firmware. ALM1 is non-latching in all variants.
- Q101/Q102 IRF100P218, back-to-back common-source (100 V, 1.28 mΩ R_DS(on), TO-247-3). Back-to-back blocking provides RPP on the 45 A path; no series diode (conduction loss would be unacceptable).
- Gate resistor 33 Ω. The 16.5 Ω + 10 nF network is an RC snubber, not gate drive.
- Current sense: R101 1 mΩ shunt + INA240A2 (50 V/V) = 50 mV/A, powered from VDDM, into FLT1_CMP / ALM1_CMP via dividers.
- Overcurrent thresholds adopted: ~50 A alarm / ~60 A trip. Divider tops 5.36k; alarm bottom 5.23k (49.8 A); trip bottom 3.74k (59.9 A). (Schematic still shows old 45 A/50 A values.)
- RESP tied to VSSS (minimum comparator response filtering); no R_RESP fitted.
- VDDM load: divider 0.33 mA + INA240A2 2.4 mA = 2.7 mA, within 5 mA IAUX limit.
- CDIV: C106 10 µF / C107 33 µF. Startup ~1.1 mJ at 55–73 mW transfer => allow 15–25 ms from EN to VDRV ready; gate on PGOOD.
- Heatsink footprint mismatch (TO-220 sinks vs TO-247 FETs) fixed. TVS conflict (SMDJ60A standoff below OV trip, clamp above 100 V) fixed. RC snubbers added.

### 2. Precharge
- U201 TPSI31P1-Q1 isolated switch driver, hysteretic current-limited buck, high-side switch.
- Q201/Q202 IRFP140N (TO-247AC drop-in), replacing IRF100P218 because of the TPSI31P1-Q1 gate-power budget.
- L201 220 µH Bourns 1140-221K-RC (fixed — PCB already ordered). D201 STPSC20065DI freewheel.
- Current sense: R201 5 mΩ shunt + INA240A1 (20 V/V) = 0.1 V/A into IS+. TPSI31P1-Q1 internal refs 1.23 V / 0.16 V => 12.3 A peak / 1.6 A valley. ~20 ms to charge 2 mF.
- CDIV: C206 4.7 µF / C207 15 µF.
- TPSI31P1-Q1 PGOOD only reflects its internal secondary rail, NOT load charge state. Handover is entirely MCU-managed, triggered by external voltage sensing (OUTCMP1).

### 3. OV/UV protection
- Two AMC23C14 reinforced isolated window comparators. VDD1 from a 12 V zener shunt off the bus (2x 2.4k 2512 + MM3Z12VST1G) with 10 Ω series resistor into each VDD1. VDD2 = 3.3 V. Open-drain outputs, 4.7k pull-ups to MCU.
- OUT2 trips on fixed 300 mV internal ref; OUT1 trips on V_REF + 4 mV, V_REF = 100 µA x R(REF–GND1).
- U301 input bus (602k / 3.74k): UV 48.6 V (OUT2), OV 65.8 V (OUT1, R306 = 4.02k).
- U302 output bus (602k / 4.02k): secondary threshold 45.2 V (OUT2). Primary threshold intended ~55 V, but R314 = 3.15k gives 48.1 V — change to 3.57k (55.0 V) or 3.65k (55.6 V).
- E96 1% resistors. Supersedes TLV1841 + SN74LVC1G08 AND-gate path, optocoupler UV path, and MCU-ADC bus sensing.

### 4. Bias supply
- REC6K-4805SRW isolated DC-DC: 60 V bus -> isolated 5 V, via fuse F1 (resized from 6 A) and series RPP diode, with L401 filter choke.
- 5 V powers both TPSI VDDP pins and the Nucleo +5V pin. 3.3 V comes from the Nucleo's on-board LDO (no separate regulator).
- Note: CR102 (STTH6002CW) is the OUTPUT freewheel diode across Vout+ (K) and Vout− (A1, A2), not the housekeeping RPP diode.

### 5. MCU / supervisory
- NUCLEO-F042K6 (STM32F042K6, Cortex-M0, 48 MHz, 32 KB flash, 6 KB RAM).
- Role: precharge/main handover, firmware fault latching, OV/UV shutdown, status reporting.
- Inputs: CCPGOOD, PREPGOOD, CCFLT, CCALM, INCMP1/2 (input bus window), OUTCMP1/2 (output bus window). Outputs: CCEN, PREEN.
- EN driven from 3.3 V GPIO vs TPSI EN rising threshold 2.7 V max at VDDP = 5 V (0.5 V margin — tight but acceptable). CE tied to VDDP on both TPSIs.
- Comms: USART1 over virtual COM only. No CAN (F042 bxCAN unused; FDCAN requirement dropped). No ADC telemetry.

### Dropped / out of scope
- Analog telemetry to MCU: dropped (INA240 outputs are on the isolated secondary side). Current and bus voltage measured with scope probes in validation.
- Thermal monitoring: out of scope this revision.
- Formal tolerance/RSS analysis: not done; thresholds set from nominal values. Cut from report methodology.

## Firmware

- Toolchain: VS Code + STM32CubeIDE extension, STM32CubeMX. Flash via Nucleo on-board ST-LINK; STLINK-V3MINIE for USART1 serial (virtual COM).
- State machine (six states): INIT -> OFF -> PRECHARGE -> HANDOVER -> ON -> FAULT.
- Relay commanded open/closed by USART1 UART commands. Latched faults cleared with UART 'r'.
- Precharge -> main handover triggered by OUTCMP1 (~55 V output bus).
- Output bus collapse (OUTCMP2) while ON trips the relay.
- All four AMC23C14 lines on EXTI interrupts, not polling. Measure and report trip latency.
- Mask AMC23C14 inputs for ~1–2 ms after VDD1 rises: the 100 nF REF cap charges at 100 µA and needs ~400 µs, longer than the 200 µs high-side blanking, so OUT1 can glitch at startup.
- IWDG mandatory (~100 ms timeout) — the MCU is inside the OV/UV trip path.
- Firmware latch replaces the hardware latch TPSI3103 lacks.

### Bring-up safety
- ST-LINK USB bonds the isolated secondary to PC ground. Flash with the bus de-energised, or use a USB isolator / isolated debugger.

## Open items

1. Correct U101 schematic symbol PTPSI3100DVXR -> TPSI3103 (TPSI3100 VREF 0.31 V would put trips at ~12 A).
2. Swap R314 3.15k -> 3.57k (or 3.65k).
3. Swap R110–R113 to the 50 A / 60 A divider values.
4. Board erratum: U102 (main-arm INA240A2) REF2 leg snapped off, floating; REF1 grounded. Zero-current output reads ~0.3 V — account for this offset in measurements.
5. RC snubber derivations.
6. Full conduction-loss and thermal verification across all power stages (IRF100P218 expected ~3.5–4 W per FET hot, ~8 W per arm).
7. Risk assessment for lab testing at RMIT (presentation risk assessment is done).
8. Showcase registration.

## Simulation (PSpice)

- OrCAD PSpice 25.1.0; PSpice for TI / TINA-TI to cross-validate TI encrypted macromodels.
- Separate per-subsystem models (the all-in-one model was too heavy for the solver). AutoConverge used for most models.
- Behavioural ABM models (GVALUE/EVALUE) for loop dynamics; TI macromodels for switching cross-validation.
- Gotchas:
  - "M"/"m" = milli. Megaohm is "MEG".
  - TI encrypted models: DC operating point convergence issues with closed behavioural loops; SKIPBP/UIC for transient; wrap G-device references in V(); sim window must exceed device startup latency.
  - Hot-plug artefact: 5 µH source inductance + input cap rings to ~120 V at t = 0. Pre-set the input cap to 60 V when studying handover (but it is real evidence for input TVS/OV consideration).
  - Subcircuit name collisions between TPSI31P1 and TPSI3052 libraries were resolved by renaming.

## Hard-won learnings (useful for report justification)

- TPSI310x (alarm + fault) is right for DC unidirectional tiered protection; TPSI311x (two fault comparators, no alarm) targets AC bidirectional sensing.
- Isolated switch drivers trade wiring simplicity for a hard secondary-side power budget (~55 mW on TPSI31P1-Q1), capping Qg x f_sw. This drove the precharge FET change. IRF100P218 Qg is 330 nC typ at 10 V (an earlier 840 nC figure was wrong).
- Bootstrap drivers (IR2117) fail in a static relay: gate droops during long closure with no switching refresh. Root cause of moving to the TPSI family.
- INA240A2 + 1 mΩ ≈ 2 W shunt loss at 45 A. Earlier INA240A4 + 6.2 mΩ saturated the amp and dissipated ~12.6 W.
- High-CM comparator precision: a 2 V window on a 60 V signal is ~0.1 V at the comparator; 1% divider mismatch can eat the whole margin.
- Rejected alternatives worth defending: LTC4368 for the main relay role; bootstrap gate drive; TLV1841 + AND-gate OV/UV; IRFP4110PbF (161 °C Tj at 65 °C ambient, no SPICE model) in favour of IRF100P218 (85 °C).

## Final report

- Written in LaTeX: scrartcl (KOMA-Script), 11 pt, biblatex IEEE with biber. Keep this; ignore the brief's Word/Times New Roman suggestion.
- Write in past tense, as if all work and testing is complete.
- Frame advice against the Assessment 2 rubric: writing style 10, format/presentation 10, literature review 20, R&D process 40, discussion/conclusion 20.
- Section plan:
  - Project Development: background sims, design calcs and component choices per subsystem, in order: Architecture, Switch Selection, Current Interruption, Gate Drive, Precharge, Monitoring Control and Protection, Auxiliary Power, Firmware Development, PCB Design. Handover sequencing goes in Firmware.
  - Findings (requirement-mapped PSpice evidence): 1 Simulation Setup and Model Validity; 2 Main Switch Conduction and Thermal (R2, R3); 3 Inductive Load Interruption (R6); 4 Precharge Performance (R5); 5 Overcurrent Protection (R7); 6 Bus OV/UV Detection (R8); 7 Fault and Fail-Safe Scenarios (R4, R10); 8 Summary Against Requirements (R# / target / predicted / simulated / margin / pass table). R9 and R11 go in Results & Discussion.
  - Results & Discussion: physical board performance only.
- Part A marker feedback to address: page numbers, typos, define all acronyms (e.g. IDMT), fix cross-references and empty appendix entry; hyperlinks and datasheet revision numbers/titles in references, manufacturer links not vendor; expanded methodology linked to research questions; thermal tables with explicit acceptability limits and heatsink impact; include spreadsheet excerpt; clarify whether thermal calcs were validated by simulation; clarify the IRF3710 stand-in vs IRF100P218 SPICE model; tighter time-axis scales on plots; full project timeline linked to methodology.

## Working style

- Hunter implements fixes himself and returns for review. Don't edit schematic/report files directly unless asked; propose changes.
- In chat, reply in plain text, not LaTeX. For report tables/equations, show the rendered version with the LaTeX alongside.
- Document trade-off rationale for every component choice — report-defensibility matters.
- Parts sourced from DigiKey.
