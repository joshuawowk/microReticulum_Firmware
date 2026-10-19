# Research: hardware options and prices (US 915 MHz)

Collected 2026-10-19. Prices from pages fetched directly are marked
**seen 2026-10-19**; prices from undated search snippets are marked **cached**.
AliExpress and Amazon pages could not be retrieved, so no AliExpress/Amazon
price here is verified. Re-check prices before ordering.

## Power budget decides viability

* A 1 W nameplate spotlight panel yields ≈ 0.5–0.7 W peak. At 2–3 winter
  peak-sun-hours and a 0.7 derate that is ≈ 0.7–1.5 Wh/day (estimate).
* Continuous load at 3.7 V:

  | Average draw | Energy/day |
  |---|---|
  | 8 mA | 0.71 Wh |
  | 10 mA | 0.89 Wh |
  | 12 mA | 1.07 Wh |
  | 15 mA | 1.33 Wh |
  | 20 mA | 1.78 Wh |
  | 50 mA | 4.44 Wh |

* Above ≈ 12–15 mA average, winter autonomy fails on this panel. That rules out
  every ESP32 board and every 1 W-PA board for the Harbor Breeze build.
* SX1262 continuous RX is ≈ 4.2–4.6 mA (Semtech brochure / Mouser snippets), so
  the radio floor is ≈ 5 mA; everything above that is the MCU and board.

## Node board comparison

| Board | Price | MCU / radio | TCXO | Max TX | RX idle current (community) | Charger | Solar input | Firmware target in this repo | Notes |
|---|---|---|---|---|---|---|---|---|---|
| **XIAO nRF52840 + Wio-SX1262 kit** | **$13.49 single unit, in stock**, [Seeed](https://www.seeedstudio.com/XIAO-nRF52840-Wio-SX1262-Kit-for-Meshtastic-p-6400.html) (re-checked 2026-10-19; no volume discount listed); $15.72 incl. tariffs, [Robotshop US](https://www.robotshop.com/products/xiao-nrf52840-wio-sx1262-kit-for-meshtastic) (cached). **No antenna included** (U.FL only, per [Meshtastic docs](https://meshtastic.org/docs/hardware/devices/seeed-studio/wio-sx1262/)); Seeed SMA-to-IPEX 120 mm cable $1.90. | nRF52840 / SX1262 | Yes ([wiki](https://wiki.seeedstudio.com/wio_sx1262/)) | +22 dBm | not documented | BQ25101, 50 mA (100 mA with P0.13 LOW), **4.2 V Li-ion only** | No dedicated input; VBUS pad range undocumented | **Added by this work** (`xiao_nrf52840`) | Keep P0.14 LOW (VBAT divider / P0.31 3.6 V limit). 21 × 17.8 mm. u.FL antenna on Wio module. Bootloader UF2, SoftDevice S140 7.3.0. |
| XIAO nRF52840 Plus | $11.19, [Robotshop](https://www.robotshop.com/products/seeed-studio-xiao-nrf52840-plus) (cached); Wio-SX1262 $4.29 add-on | nRF52840 | — | — | — | BQ25101 class | — | (same pinout for the kit radio) | Bare MCU board. |
| Faketec: ProMicro/SuperMini nRF52840 + HT-RA62 / E22-900M22S | ≈ EUR 10 total per [fakeTec_pcb README](https://github.com/gargomoma/fakeTec_pcb) (seen 2026-10-19); PCB ≈ EUR 0.40 | nRF52840 / SX1262 | HT-RA62, E22-xxxM-22S yes; RA-01SH no | 22 dBm | no measurement | nice!nano-clone charger | via board charger / v6 CN3791 (untested) | No | Clone quality varies; Faketec V4 gets hot charging (issue #57); LiFePO4 unsupported (#69). |
| Heltec Mesh Node T114 | $17.90–$33.70, [Heltec](https://heltec.org/project/mesh-node-t114/) (seen 2026-10-19) | nRF52840 / SX1262 | not stated | 21 dBm | deep sleep ≈ 11 µA | not named | **5–6.5 V** solar input | Boards.h only (`heltec_t114-local` dev env) | 50.8 × 22.86 mm |
| RAK4631 + RAK19007 | $29.99, [Ameridroid](https://ameridroid.com/products/wisblock-rak19007-rak4631-board) (cached) | nRF52840 / SX1262 | — | 22 dBm | **≈ 5–7 mA** Meshtastic on RAK19003 ([forum](https://meshtastic.discourse.group/t/rak-wisblock-nrf-power-usage/8448)); ≈ 8.4 mA MeshCore EasySkyMesh ([NodakMesh](https://nodakmesh.org/blog/easyskymesh-power-efficient-meshcore-firmware)) | 350 mA ([datasheet](https://docs.rakwireless.com/product-categories/wisblock/rak19007/datasheet/)), Li-ion only | JST-ZH, 5 V panel, **must not exceed 5.5 V** | Yes (`wiscore_rak4631`) | Panel Voc ≈ 6 V can exceed the 5.5 V limit. |
| Heltec MeshSolar | $38.99–$44.90, [Heltec](https://heltec.org/project/meshsolar/) (seen) | nRF52840 / SX1262 | — | 21 dBm | — | BQ4050 + CN3795 | 18–25 V MPPT | No | Unsuitable (input voltage, size, price). |
| SenseCAP Solar Node P1 | $73.72, [Robotshop](https://www.robotshop.com/products/sensecap-solar-node-p1-meshtastic-w-o-gps-battery) (cached) | nRF52840 / SX1262 | — | — | — | — | own panel | No | Rejected on price. |
| Wio Tracker L1 Pro | $52.99, [Seeed](https://www.seeedstudio.com/Wio-Tracker-L1-Pro-p-6454.html) (seen) | nRF52840 / SX1262 | — | — | — | — | — | No | GPS + OLED waste power. |
| GAT562 30s | $39.99, [NodakMesh](https://nodakmesh.org/blog/gat562-30s-mesh-solar-repeater-first-look) (seen) | nRF52840 / SX1262 + 1 W PA | — | 30 dBm | no data | — | 4.5–5.5 V | No | PA idle current undocumented. |
| *Heltec WiFi LoRa 32 V3 / V4* | V4 $17.90–$27.50 (Heltec) | ESP32-S3 / SX1262 (+PA on V4) | — | 21–28 dBm | V3 ≈ 9 mA and V4.3 5.5–13 mA only with EasySkyMesh light-sleep; **≈ 50 mA** stock MeshCore V4 ([rdumesh](https://rdumesh.org/heltec-v43-easyskymesh-power-saving/)) | on board | — | Yes | Fails the Harbor Breeze budget. |
| *XIAO ESP32S3 + Wio-SX1262* | GBP 10.50 ex-VAT (Pi Hut, cached) | ESP32-S3 / SX1262 | Yes | 22 dBm | no data (ESP32-S3 ≫ nRF52840) | BQ25101 class | — | Yes (`seeed_xiao_esp32s3`) | Comparison only. |

## Recommendation from the research

* **Lowest cost: XIAO nRF52840 + Wio-SX1262 kit, ≈ $15–16 US.** Cheapest
  complete nRF52840 + TCXO SX1262 board, antenna included, no PA/LNA to idle,
  charger current (50/100 mA) matched to a small panel. Caveats: needed a new
  firmware target (done in this work); BQ25101 is 4.2 V-only (unsafe for a
  LiFePO4 cell); no cell protection, so firmware cutoff matters.
* **Runner-up: RAK4631 + RAK19007, ≈ $30.** Existing target, best-documented low
  RX current, solderless JST connectors; solar pad limited to 5.5 V.
* **Faketec** is cheaper on paper (≈ EUR 10) but the charge path, clone quality
  and soldering effort make it a worse first build.

## Other BOM parts

| Part | Price | Source | Notes |
|---|---|---|---|
| u.FL → SMA-female bulkhead pigtail, 15 cm | $3.80 | [GetFPV](https://www.getfpv.com/sma-female-to-u-fl-extension-cable-15cm.html) (cached) | Optional; needed for an external antenna |
| 915 MHz u.FL whip | EUR 3.95 | [Olimex](https://www.olimex.com/Products/Components/RF/ANTENNA-20CM-915MHZ-U-FL/) | No US price captured |
| Protected 18650 3000 mAh (XTAR) | $7.99 | [IMR Batteries](https://imrbatteries.com/products/xtar-18650-3000mah-5a-battery-protected-button-top) (cached) | Only if the light's cell is unusable; protected cells are longer |
| LiFePO4 3.6 V charger module | $7.15 + $1.99 ship | [Walmart](https://www.walmart.com/ip/19088752401) (cached) | Poor fit (2.4 A); keeping the light's own LiFePO4 controller is better |
| PG7 cable gland 10-pack | AUD 13.99 | [Element14 AU](https://au.element14.com/multicomp-pro/pg7dg/gland-pg7-3-6-5mm-pk10/dp/1621060) (cached) | No US price found |
| Conformal coating, MG Chemicals 422B 340 g | $38.08 | [Hisco](https://www.hisco.com/Product/422B-340G-33018) (cached) | Expensive for one board; optional |
| E22-900M22S | $6.60 qty 1 | LCSC (cached) | Faketec radio option |

## Notes added on re-check (2026-10-19)

* The Wio-SX1262 in the nRF52840 kit is **not** pin-compatible with the one in
  the XIAO ESP32S3 kit ([Meshtastic](https://meshtastic.org/docs/hardware/devices/seeed-studio/wio-sx1262/)):
  a module from the other kit needs firmware with matching pin definitions.
  The `xiao_nrf52840` target uses the nRF52840 kit's wiring.
* The 180 lm Harbor Breeze solar flood light lists two 18500 LiFePO4 cells
  (Lowe's listing via search), more evidence that Harbor Breeze solar lights
  commonly use LiFePO4.
* SX1262 receive current: Semtech quotes 4.2–4.6 mA with the DC-DC regulator
  ([product page](https://www.semtech.com/products/wireless-rf/lora-connect/sx1262),
  [brochure](https://www.semtech.com/uploads/common/LoRa_products_sx126x.pdf));
  module vendors report ≈ 8–10 mA in LDO mode.

## Uncertainties

1. Battery chemistry of the 1 W spotlight (largest risk; all on-board chargers are 4.2 V).
2. Panel open-circuit voltage (matters for RAK 5.5 V cap, XIAO VBUS, T114 6.5 V).
3. Real RX current on this firmware — community figures are Meshtastic/MeshCore with MCU sleep.
4. Several prices are cached snippets or quantity-tier prices.
5. Internal dimensions of the housing were not found.
