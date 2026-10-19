# Research: Pro Micro nRF52840 ("SuperMini" / nice!nano clone) + SX1262 ("Faketec")

Collected 2026-10-19 for the solar repeater. Sources inline; anything without a
source is marked unverified.

## Verdict for the Harbor Breeze power budget

**It can fit** (same nRF52840 + SX1262 silicon as the XIAO, so ≈ 5.5–6.5 mA is
plausible with the headless repeater profile), but with more caveats than the
XIAO and **no measured full-node RX figure exists** (one unoptimised Meshtastic
prototype reported ≈ 9 mA with BLE connected —
[forum](https://meshtastic.discourse.group/t/power-requirements/868)).

| Factor | Impact on a ≈ 5.5 mA budget |
|---|---|
| Board leakage, VCC off | Good boards ≈ 4 µA, leaky batches ≈ 60 µA (Si diode instead of Schottky); early batches with a 5.6 kΩ pull-up ≈ 700 µA ([joric wiki](https://github.com/joric/nrfmicro/wiki/ALternatives), [sasodoma](https://github.com/sasodoma/nrf52840-promicro)). Only the early-batch leak (≈ 13 % of budget) matters. |
| 3.3 V LDO quiescent (ME6217 / RT9013 / TP2028 / unmarked) | Not found — unverified. Typically tens of µA for these parts. |
| nRF52840 DC/DC | Inductor unverified on clones; a clone drew 1.40 mA vs 0.08 mA standby with DCDC-HV **on** ([ZMK #2990](https://github.com/zmkfirmware/zmk/issues/2990)). Keep DC/DC off — with idle sleep the LDO penalty is ≈ 0.05 mA. |
| LEDs | Listing says a software switch cuts LED power; user LED on P0.15 (active high) stays dark in repeater builds. Check your batch for any hard-wired power LED. |
| 32 kHz crystal | Unreliable on clones; Meshtastic uses the RC oscillator (`USE_LFRC`) — negligible power cost. |
| Battery sensing | **None usable on the board** (divider unpopulated or wired to P0.24). The Faketec PCB adds a divider on P0.31, but the ratio is unconfirmed (Meshtastic define says 1.5 M + 1 M, README says 2 × 1 M). Without it the firmware cannot hibernate on a low cell. |
| Charger | TP4054 / LTH7R, ≈ 100 mA (300 mA with jumper), **Li-ion 4.2 V only** — same LiFePO4 rule as the XIAO: keep the light's controller and feed the cell to BAT. |
| Radio module | Use a 22 dBm TCXO module: HT-RA62 ($3.90, [Heltec](https://heltec.org/project/ht-ra62/)) or E22-900M22S (≈ $6.60, LCSC). **Avoid E22-900M30S**: ≈ 650 mA TX bursts add ≈ 6.5 mA average at 1 % airtime and can brown out a small cell. |
| Quality | Reported failure rates from 1 in 50 to 1 in 2 per batch ([Faketec #30](https://github.com/gargomoma/fakeTec_pcb/issues/30)); some ship without a bootloader. |

## The Teyleten listing (Amazon B0CYLNZ6V4)

"Teyleten Robot Pro Micro NRF52840 Development Board … 3pcs": **$16.88 per
3-pack (≈ $5.63/board)** seen once in a cached widget — verify live. Generic
Pro Micro nRF52840; charger IC, regulator, crystal and divider not listed.
4.3★ (42 reviews). [Amazon](https://www.amazon.com/Teyleten-Robot-Development-Bluetooth-Management/dp/B0CYLNZ6V4)

## Firmware facts (for a future target)

* Bootloader: nice!nano-style Adafruit UF2, **SoftDevice S140 6.1.1** (use the
  core's v6 linker, not v7). Faketec recommends bootloader ≥ 0.10.0.
* Radio (Meshtastic `nrf52_promicro_diy_tcxo`): NSS P1.13, DIO1 P0.10,
  BUSY P0.29, RESET P0.09, RXEN P0.17, SCK P1.11, MOSI P1.15, MISO P0.02,
  TXEN NC (P1.06 optional), **P0.13 must be driven HIGH** (3V3 rail), DIO2 RF
  switch, DIO3 TCXO 1.8 V on TCXO modules.
* LF clock: RC. DC/DC: off. Battery: P0.31 with a build-time divider ratio.

## Cost per node (before shipping)

Board ≈ $5.63 + HT-RA62 $3.90 + Faketec PCB ≈ $2 + antenna ≈ **$12–16**
(≈ $11 in a [third-party build](https://adrelien.com/diy-meshtastic-how-to-build-your-own-meshtastic-device-with-faketec-pcb-nrf52840/)),
plus SMD soldering of the module. Compare: XIAO nRF52840 $9.99 + a Wio-SX1262
you already own (or $13.49 for the kit), plug-in/through-hole.
