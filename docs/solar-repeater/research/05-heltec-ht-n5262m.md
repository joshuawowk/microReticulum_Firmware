# Research: Heltec HT-N5262M (nRF52840 + SX1262 stamp module)

Collected 2026-10-19. Sources: [datasheet](https://resource.heltec.cn/download/HT-N5262M/Datasheet/HT-N5262M_V1.0.pdf),
[schematic](https://resource.heltec.cn/download/HT-N5262M/HT-N5262M_Schematic_Diagram.pdf),
[product page](https://heltec.org/project/ht-n5262m/),
[MeshTower datasheet](https://resource.heltec.cn/download/MeshTower/Datasheet/MeshTower_Datasheet.pdf).

## Facts

| Fact | Value |
|---|---|
| What | Standalone 54-pad, 1.27 mm stamp-hole module, 30 × 20 × 3.2 mm, nRF52840 + SX1262 ("a low-cost module based on Mesh Node T114") |
| Price | **$14.90** (Heltec, band-selectable); €21.95 / $25.59 at OpenELAB |
| Bands | 433 / 470–510 / 863–928 MHz (HF variant for US915); 21 ± 1 dBm max |
| Supply | **VDD only, 3.3 V typ. (1.8–3.6 V, abs. max 3.9 V)** — no VBAT/charger; needs a regulator (a full LiFePO4 at 3.65 V slightly exceeds the 3.6 V rating) |
| On module | 32.768 kHz crystal, 32 MHz crystal + 32 MHz oscillator (likely TCXO), DC/DC inductors (10 µH, 15 µH), UPG2179 RF switch, IPEX + castellated RF pads |
| Exposed | USB D+/D−/VUSB, SWDIO/SWCLK, most P0/P1 GPIOs, 2 × VDD |
| Bootloader | Not stated — plan for SWD |
| Sleep | 8–9 µA (Heltec) |
| **RX current** | **33 mA published, "with backplane @ 3.3 V"** — ≈ 6× what the SX1262 + a sleeping nRF52840 need; conditions unclear (likely dev board + MCU awake). **Unverified for a sleeping firmware.** |
| Internal LoRa pins | Not published as text; the pins the module does *not* expose are exactly the T114's (NSS P0.24, DIO1 P0.20, BUSY P0.17, RESET P0.25, SCK P0.19, MOSI P0.22, MISO P0.23) — very likely identical (inference). |

## Products

MeshTower and MeshSolar are built on the module (MeshSolar's input is
18–24 V MPPT — incompatible with a ≈ 6 V spotlight panel). The T114 is the
module's basis but not confirmed to embed it. MeshPocket, T1, T096 and RC52
wire the SX1262 differently (discrete designs) despite the shared
"HT-n5262" USB product string, which is just Heltec's core board name.

## Verdict for the Harbor Breeze power budget

**Uncertain.** The silicon should allow ≈ 5.5–6.5 mA with the repeater
profile, but Heltec's own 33 mA RX figure would need ≈ 2.9 Wh/day — far
beyond the panel. Measure before relying on it. It is also the most effort
(carrier PCB with regulator, charger, USB; possible SWD bootloader flash) and
costs more than a XIAO nRF52840 ($9.99) for a one-off.
