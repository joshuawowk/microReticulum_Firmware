# Headless solar Reticulum repeater

A self-contained Reticulum transport node that lives on a pole or fence post,
runs from a small solar panel and one cell, and never needs a person or a
host computer. It uses this firmware's embedded microReticulum stack with
the **headless repeater profile** (`-DHEADLESS_REPEATER`).

The reference build reuses the panel, cell and housing of a Lowe's
**Harbor Breeze 60 lm / 1 W solar spotlight**, the same light many Meshtastic
and MeshCore solar nodes are built from. It adds a **Seeed XIAO nRF52840 +
Wio-SX1262** kit and an antenna, for about **$25–30** all in.

* [Quick start](#quick-start)
* [Why an nRF52840 board](#why-an-nrf52840-board-power-budget)
* [Bill of materials](#bill-of-materials)
* [Converting the Harbor Breeze light](#converting-the-harbor-breeze-light)
* [Firmware: build, flash, configure](#firmware-build-flash-configure)
* [What the repeater profile does](#what-the-repeater-profile-does)
* [Settings](#settings)
* [Measuring power](#measuring-power)
* [Troubleshooting](#troubleshooting)
* [Limitations](#limitations)

Background research (sources, prices, pin maps) is in [`research/`](research/).

## Quick start

1. **Identify the light's cell** before buying anything else. Open the battery
   compartment and read the label: **LiFePO4 (3.2 V)** or **Li-ion (3.7 V)**.
   Builds and wiring differ, and charging a LiFePO4 cell from a Li-ion charger
   is a fire risk. See [the cell](#1-identify-the-cell).
2. Build and flash the matching image:
   * LiFePO4: `pio run -e xiao_nrf52840-repeater-lifepo4`
   * Li-ion: `pio run -e xiao_nrf52840-repeater`

   Then double-tap RESET on the XIAO and copy
   `.pio/build/<env>/rnode_firmware_<variant>.uf2` onto the USB drive it
   mounts.
3. With the cell **not** connected, set the radio once over USB, using your
   network's parameters (example values for the US 915 MHz band):
   ```
   rnodeconf --tnc --freq 914875000 --bw 125000 --sf 8 --cr 5 --txp 22 /dev/ttyACM0
   ```
   Then press RESET. The node boots into transport mode from now on.
4. Wire the XIAO to the cell, cut the LED leads, fit the antenna, and seal the
   housing (see [conversion](#converting-the-harbor-breeze-light)).
5. Mount it in full sun, with the panel facing south in the northern hemisphere.

## Why an nRF52840 board (power budget)

The spotlight's panel is rated 1 W, but a teardown of a sibling model measured
about **0.5 W in full sun**. That works out to roughly **0.7–1.5 Wh per day**
in winter and **270–360 mAh per day** at best
([research](research/01-harbor-breeze-light.md)). A repeater listens all the
time, so its **average receive current** decides whether it survives winter.

| Average draw at 3.7 V | Energy per day | Fits a ~0.5 W panel? |
|---|---|---|
| 6 mA | 0.53 Wh | Yes, with margin for cloudy days |
| 10 mA | 0.89 Wh | Marginal in winter |
| 15 mA | 1.33 Wh | No, except in summer |
| 40–60 mA (ESP32 boards) | 3.6–5.3 Wh | No: needs a 5–10 W panel and 2+ 18650 cells |

The SX1262 radio needs about 4.6 mA to listen with its DC-DC regulator on, and
roughly double that from its LDO. An nRF52840 that sleeps between packets adds
only tens of µA. An ESP32 cannot drop that low while listening in this
firmware.

**Estimated draw of the XIAO nRF52840 repeater.** These figures are worked
out from datasheets, not measured. See [Measuring power](#measuring-power).

| | Stock build (busy loop, radio LDO) | Repeater profile |
|---|---|---|
| nRF52840 | ≈ 6.3 mA, 100 % active, LDO | ≈ 0.05–0.15 mA, asleep about 98 % of the time, DC/DC |
| SX1262 receive, boosted gain | ≈ 10 mA (LDO) | ≈ 5.3 mA (DC-DC) |
| Board leakage (divider, flash, LDO) | ≈ 0.05 mA | ≈ 0.05 mA |
| **Idle total** | **≈ 16 mA (1.4 Wh/day)** | **≈ 5.5 mA (0.5 Wh/day)** |
| Transmit at 22 dBm, ≈ 118 mA, at 1 % airtime | +1.2 mA | +1.2 mA |

With the repeater profile, a 1500 mAh Li-ion cell carries the node through
about **7 days** without sun. A 1000 mAh LiFePO4 cell lasts about **5 days**.

### Board options

| Board | MCU / radio | ≈ Price | Fits the Harbor Breeze budget | Firmware env |
|---|---|---|---|---|
| **Seeed XIAO nRF52840 + Wio-SX1262 kit** | nRF52840 / SX1262, TCXO | **$13.49** | **Yes** | `xiao_nrf52840-repeater[-lifepo4]` |
| RAK4631 + RAK19003/19007 | nRF52840 / SX1262 | ≈ $30 | Yes, solderless JST connectors | `wiscore_rak4631-repeater[-lifepo4]` |
| Heltec V4 | ESP32-S3 / SX1262 + 28 dBm PA/LNA | ≈ $18–28 | No: needs a bigger panel | `heltec_wifi_lora_32_V4-repeater` |
| LilyGO LoRa32 v2.1 (T3 v1.6 / v1.6.1) | ESP32 / SX1276 | ≈ $15–20 | No: needs a bigger panel | `ttgo-lora32-v21-repeater` |
| XIAO ESP32S3 + Wio-SX1262 | ESP32-S3 / SX1262 | ≈ $14 | No; also has no battery sensing | `seeed_xiao_esp32s3-repeater` |
| Heltec T114, ProMicro "Faketec" | nRF52840 / SX1262 | $10–25 | Yes | Not yet (see [Limitations](#limitations)) |

The Wio-SX1262 in the nRF52840 kit is **wired differently** from the one in
the XIAO ESP32S3 kit. The ESP32S3 kit's radio can still be moved onto a XIAO
nRF52840 with a little soldering and its own firmware build; see
[Reusing a Wio-SX1262 from a XIAO ESP32S3 kit](#reusing-a-wio-sx1262-from-a-xiao-esp32s3-kit).

## Bill of materials

### Lowest cost: Harbor Breeze + XIAO nRF52840 (≈ $25–30)

Prices were checked on 2026-10-19. Lowe's blocks automated price checks, so
confirm the light's price yourself.

| # | Part | Qty | Unit price | Source | Notes |
|---|---|---|---|---|---|
| 1 | Harbor Breeze 60 lm 1 W solar LED spot light | 1 | $15 regular, often $9.98 on sale, ≈ $9 each for 6+ | [Lowe's 5018244381](https://www.lowes.com/pd/Harbor-Breeze-60-Lumens-1-Watt-Black-Warm-White-Solar-LED-Spot-light/5018244381) | Provides the panel, cell and housing |
| 2 | Seeed XIAO nRF52840 & Wio-SX1262 Kit | 1 | **$13.49** | [Seeed](https://www.seeedstudio.com/XIAO-nRF52840-Wio-SX1262-Kit-for-Meshtastic-p-6400.html) | In stock; no antenna included |
| 3a | 915 MHz antenna with U.FL/IPEX lead, inside the housing (cheapest) | 1 | ≈ $2–4 | Any 915 MHz U.FL whip or FPC antenna, e.g. [Olimex 20 cm whip](https://www.olimex.com/Products/Components/RF/ANTENNA-20CM-915MHZ-U-FL/) (EUR 3.95) | Keep it away from the panel's metal and the cell |
| 3b | *or* SMA-to-IPEX 120 mm bulkhead cable + 915 MHz SMA antenna (better range) | 1 + 1 | $1.90 + ≈ $5–9 | [Seeed cable](https://www.seeedstudio.com/XIAO-nRF52840-Wio-SX1262-Kit-for-Meshtastic-p-6400.html) ("Also Add") | Drill the housing and seal with silicone |
| 4 | Hook-up wire, 24–26 AWG silicone, ≈ 20 cm red + black | 1 | ≈ $0.20 | — | Cell to XIAO BAT pads |
| 5 | Heat-shrink, Kapton tape, neutral-cure silicone | — | ≈ $1 (shop stock) | — | Insulate the XIAO; seal holes and the weep hole |
| | **Total** | | **≈ $25 (internal antenna) to ≈ $30 (external antenna)** | | |

Tools: soldering iron, side cutters, multimeter, and a USB-C cable for the
one-time setup.

### Solderless alternative: RAK4631 (≈ $35–40)

| Part | Price | Notes |
|---|---|---|
| Harbor Breeze light | $10–15 | as above |
| RAK19007 + RAK4631 bundle | ≈ $30 ([Ameridroid](https://ameridroid.com/products/wisblock-rak19007-rak4631-board), cached) | JST-PH battery and JST-ZH solar connectors. The solar input is Li-ion only and limited to **5.5 V maximum**. |
| IPEX-to-SMA pigtail + 915 MHz antenna | ≈ $6–12 | |

Build `wiscore_rak4631-repeater` (or `-lifepo4`). If the cell is LiFePO4, do
**not** use the RAK's solar or battery charger. Keep the light's controller
and feed the RAK from the cell, exactly as in the XIAO LiFePO4 wiring below.

### Reusing a Wio-SX1262 from a XIAO ESP32S3 kit

If you already own XIAO ESP32S3 & Wio-SX1262 kits, you only need a bare
**XIAO nRF52840** ([$9.99 at Seeed](https://www.seeedstudio.com/Seeed-XIAO-BLE-nRF52840-p-5201.html),
$8.99 each for 10+, checked 2026-10-19). Reuse the kit's Wio-SX1262 and its
2 dBi antenna. The radio then costs nothing extra, so the node is about
**$20–25** including the light.

The kit's radio is the **board-to-board (B2B) version** of the Wio-SX1262
("Wio-SX1262 for XIAO V1.0"). The XIAO nRF52840 has no B2B connector, but the
Wio board also routes every LoRa signal to its two rows of XIAO header holes
([Seeed schematic](https://files.seeedstudio.com/products/SenseCAP/Wio_SX1262/Schematic_Diagram_Wio-SX1262_for_XIAO.pdf)).
The pinout differs from the nRF52840 kit's radio:

| XIAO pin | B2B Wio (ESP32S3 kit) | Wio from the nRF52840 kit |
|---|---|---|
| D0 | DIO1 | — |
| D1 | BUSY | DIO1 |
| D2 | RESET | RESET |
| D3 | NSS | BUSY |
| D4 | RXEN (RF switch) | NSS |
| D5 | — | RXEN (RF switch) |
| D8 / D9 / D10 | SCK / MISO / MOSI | SCK / MISO / MOSI |
| 5V / GND / 3V3 | VIN / GND / 3V3 | VIN / GND / 3V3 |

Assembly:

1. Unplug the Wio-SX1262 from the XIAO ESP32S3's B2B connector.
2. Solder the battery leads to the XIAO nRF52840's **BAT+ / BAT−** pads on
   its underside first. They are hard to reach once the boards are stacked.
3. Stack the XIAO nRF52840 on the Wio board in the **same orientation the
   ESP32S3 had**, with the same outline and the USB-C over the same edge. That
   puts D0 on the Wio's DIO1 hole. Before soldering, confirm with a
   continuity tester that the Wio's GND and 3V3 holes line up with the XIAO's
   GND and 3V3 pins. A flipped stack would put 5 V on a signal pin. Then
   solder both 1×7 rows with pin headers (stacking above or below both work).
4. Flash a `xiao_nrf52840-wio-b2b*` build (table below). The plain
   `xiao_nrf52840*` builds use the other pinout and **will not find the radio**.

The Wio board's user button and green LED are wired only to the B2B
connector, so they don't work on this stack. A repeater doesn't need them.
This pin map matches Meshtastic's `SEEED_XIAO_NRF_WIO_BTB` variant, which
community builders have used on this exact combination.

### Using the ESP32 boards you already have

The Heltec V4, LoRa32 T3 v1.6.1 and XIAO ESP32S3 all run the repeater
profile. They save power where they can: 80 MHz CPU, idle sleep, display and
Wi-Fi off, and on the V4 the Vext rail off. They still draw tens of mA, so they
need roughly a 6 V 5–10 W panel, a solar Li-ion charger such as a CN3791 or
the board's own charger, and two or more 18650 cells. The Harbor Breeze panel
cannot keep them running. The Heltec V4's PA/LNA front end adds a few mA in
receive, the price of its longer range.

## Converting the Harbor Breeze light

### 1. Identify the cell

Different production runs of these lights ship different cells:

* **LiFePO4** 18500, 3.2 V, about 1000 mAh. Lowe's listings for the 60 lm
  spotlight family (and the 180 lm flood light) name LiFePO4. Treat this as
  the likely case.
* **Li-ion** 18650, 3.7 V, about 1500 mAh. This is what NodakMesh found in the
  SL1832 flood light.

Read the label. Then measure the cell after a sunny day: a LiFePO4 cell sits
near 3.3–3.4 V, while a Li-ion cell sits at 3.9–4.2 V. If you cannot tell,
treat the cell as **LiFePO4**:

* **Wiring:** the LiFePO4 wiring below (keep the stock controller, never
  USB-charge the cell through the XIAO) is safe for either cell.
* **Firmware:** the thresholds should still match the cell. If you really
  cannot tell, the `-lifepo4` build is the lesser evil. On a Li-ion cell it
  runs the cell down to 2.95 V, a deeper discharge than ideal but not
  dangerous. A Li-ion build on a LiFePO4 cell would instead hibernate at
  about 50 % charge and only wake in strong sun.

### 2. Wiring

The safest wiring for **both** chemistries keeps the light's own solar
charge controller. It was designed for the cell it shipped with. Power the
XIAO **directly from the cell terminals**. Do not take power from the
controller's LED output: its switch and sense resistor cause brownouts, which
Philly Mesh reported with a XIAO. Cut the LED wires so the lamp cannot drain
the cell at night.

```
   solar panel ──► stock controller board ──► cell (+)──────┬──── XIAO BAT+ pad
   (unchanged)     (LED leads cut,                          │
                    slide switch ON)        cell (−)──────┬─┼──── XIAO BAT− pad
                                                          │ │
                                         (short, direct wires from the cell
                                          holder, not from the LED output)
```

* **Leave the XIAO's USB-C / 5 V pin unconnected in the field.**
* **LiFePO4 cell: never plug USB into the XIAO while the cell is connected.**
  The XIAO's BQ25101 charger charges to 4.2 V, which overcharges a 3.6 V
  LiFePO4 cell. Configure the board before wiring the cell in, or unplug
  the cell first. After installation, manage it remotely over Reticulum
  instead (see [Remote management](#remote-management)).
* **Li-ion cell:** the same wiring works, and a USB connection at the bench is
  safe because the XIAO's charger is a Li-ion charger.
* If you replace the stock controller, use a charger that matches the cell:
  3.6 V for LiFePO4, 4.2 V for Li-ion. Never wire the panel directly to the
  cell.

The XIAO measures the cell through its own 1 MΩ / 510 kΩ divider on P0.31,
which the firmware keeps enabled. That is all the repeater needs for its
low-battery protection.

### 3. Antenna

* **Internal:** route a U.FL whip upward inside the plastic head, away from
  the panel and the cell. This is the cheapest option and fine for local
  coverage.
* **External:** fit an SMA bulkhead on the side or bottom of the housing. Seal
  it with silicone and do not overtighten, because the plastic cracks.
* **Never transmit without an antenna connected.** It can damage the SX1262.
  Configure the radio with the antenna attached.

### 4. Assembly and weatherproofing

1. Remove the LED board, or cut its leads and insulate them.
2. Insulate the back of the XIAO and Wio-SX1262 with Kapton tape, then fix
   them inside the head with foam tape or hot glue.
3. Seal every hole you drilled. The panel compartment is tighter than the LED
   head, so keep the electronics up there if they fit. Add a small desiccant
   pack, and keep the factory weep hole clear or seal it deliberately.
4. Hot glue the U.FL connector so it cannot vibrate loose.
5. Leave the light's slide switch **ON** so the controller keeps charging.

Several 3D-printed mounts and adapters for this light are listed in
[research/01-harbor-breeze-light.md](research/01-harbor-breeze-light.md#existing-build-guides).

## Firmware: build, flash, configure

Builds use [PlatformIO](https://platformio.org/). The repeater environments
are:

| Env | Board | Cell |
|---|---|---|
| `xiao_nrf52840-repeater` | XIAO nRF52840 + Wio-SX1262 | Li-ion |
| `xiao_nrf52840-repeater-lifepo4` | XIAO nRF52840 + Wio-SX1262 | LiFePO4 |
| `xiao_nrf52840-wio-b2b-repeater` | XIAO nRF52840 + B2B Wio-SX1262 from an ESP32S3 kit | Li-ion |
| `xiao_nrf52840-wio-b2b-repeater-lifepo4` | XIAO nRF52840 + B2B Wio-SX1262 from an ESP32S3 kit | LiFePO4 |
| `wiscore_rak4631-repeater` | RAK4631 | Li-ion |
| `wiscore_rak4631-repeater-lifepo4` | RAK4631 | LiFePO4 |
| `heltec_wifi_lora_32_V4-repeater` | Heltec V4 | Li-ion |
| `ttgo-lora32-v21-repeater` | LilyGO LoRa32 v2.1 / T3 v1.6.1 | Li-ion |
| `seeed_xiao_esp32s3-repeater` | XIAO ESP32S3 + Wio-SX1262 | — (no battery sensing) |

```
pio run -e xiao_nrf52840-repeater-lifepo4             # build
pio run -e xiao_nrf52840-repeater-lifepo4 -t upload   # or flash over USB DFU
```

**Flashing nRF52 boards by UF2.** Every nRF52 build also writes
`.pio/build/<env>/rnode_firmware_<variant>.uf2`. Double-tap RESET to enter the
bootloader, then copy the file onto the drive that appears.

**First-time radio setup.** The repeater provisions its own identity on first
boot, so the `rnodeconf --autoinstall` / `--rom` steps are not needed. Radio
parameters are deliberately **not** preset, because they must match your
region's rules and your network. Set them once while the board is on USB (for
a LiFePO4 build, before the cell is connected):

```
rnodeconf --tnc --freq 914875000 --bw 125000 --sf 8 --cr 5 --txp 22 /dev/ttyACM0
```

You can also use the RNode Console (`Release/console.html`) over Web Serial.
Press RESET afterwards: the saved configuration puts the node into transport
mode at boot, and it stays there across power cycles and hibernation.

**Building in your own defaults.** To preset the radio in a custom build, add
all five `REPEATER_LORA_*` flags. They are applied only while the EEPROM has
no radio configuration:

```ini
[env:my-repeater]
extends = env:xiao_nrf52840-repeater-lifepo4
build_flags =
	${env:xiao_nrf52840-repeater-lifepo4.build_flags}
	-DREPEATER_LORA_FREQ=914875000
	-DREPEATER_LORA_BW=125000
	-DREPEATER_LORA_SF=8
	-DREPEATER_LORA_CR=5
	-DREPEATER_LORA_TXP=22
```

### Remote management

Once the node is deployed, its Provisioning interface (transport settings,
metrics and reboot) is reachable over Reticulum through the
`rnstransport.remote.management` destination. Use the RNode Console's
"RNS (via rnsapid)" transport; see [Provisioning.md](../Provisioning.md). The
optional NomadNet stats pages report battery voltage and channel use.

## What the repeater profile does

All of this is compiled in only with `-DHEADLESS_REPEATER`. Other builds are
unchanged.

* **Sleeps between packets.** At the end of every main-loop pass the loop task
  blocks on a FreeRTOS task notification for up to 50 ms. The radio's
  receive-done interrupt gives that notification, so an arriving packet wakes
  the CPU at once, and FreeRTOS tickless idle sleeps it in between:
  `sd_app_evt_wait()` on nRF52, `waiti` on ESP32. The node stays fully awake
  in five situations:
  * packets are queued to transmit (CSMA needs exact timing)
  * serial input is pending
  * a USB host has the port open (nRF52)
  * a BLE client is connected or pairing
  * the console or Wi-Fi is active (not compiled into ESP32 repeater builds)
* **Protects the cell.** If the averaged battery voltage stays below the
  cutoff for 60 s, the node does three things:
  1. saves the path table
  2. puts the radio to sleep, and stops BLE advertising and any connection
  3. hibernates (on ESP32 it also holds the radio's chip-select line high so
     the radio stays asleep)

  An nRF52 stays in System ON sleep and re-checks the battery every 30 s. An
  ESP32 uses timed deep sleep and re-checks every 5 min. After three readings
  at or above the resume voltage, the node restarts by itself. No button
  press or ladder climb is needed. The radio is also never started at boot
  on a cell already below the cutoff, which prevents brown-out boot loops at
  dawn.

  | Cell | Cutoff | Resume | Reported 0–100 % range |
  |---|---|---|---|
  | Li-ion (default) | 3.30 V | 3.70 V | 3.15–4.2 V (board default) |
  | LiFePO4 (`-lifepo4` envs) | 2.95 V | 3.25 V | 2.90–3.40 V |

* **Self-provisions.** On an unprovisioned EEPROM it writes the board's
  product, model, serial (from the chip ID), checksum and lock. It also skips
  rnodeconf's firmware-hash check, so UF2 and DFU updates keep working. A board
  already provisioned by rnodeconf keeps its identity.
* **Trims everything else:**
  * LEDs stay dark
  * RNS logs only at NOTICE level and above
  * nRF52 DC/DC regulator on (RAK4631, RAK3401, XIAO)
  * SX126x on its DC-DC converter
  * ESP32 at 80 MHz, with no display, Wi-Fi or web console
  * Heltec V4 Vext (OLED/GNSS) off
  * BLE stays off unless you enable it with `rnodeconf -b`

## Settings

Override any of these in `build_flags` (see `RepeaterConfig.h`):

| Flag | Default | Meaning |
|---|---|---|
| `REPEATER_BATTERY` | `REPEATER_BATTERY_LIION` | Or `REPEATER_BATTERY_LIFEPO4` |
| `REPEATER_SHUTDOWN_V` | 3.30 / 2.95 | Hibernate below this (V) |
| `REPEATER_RESUME_V` | 3.70 / 3.25 | Restart at or above this (V) |
| `REPEATER_SHUTDOWN_DELAY_S` | 60 | Seconds below the cutoff before hibernating |
| `REPEATER_HIBERNATE_CHECK_S` | 30 (nRF52) / 300 (ESP32) | Battery check interval while hibernating |
| `REPEATER_RESUME_CHECKS` | 3 | Consecutive good readings needed to restart |
| `REPEATER_NO_BATTERY_V` | 2.0 | Readings below this mean "no cell, USB powered" |
| `REPEATER_IDLE_MS` | 50 | Longest main-loop sleep |
| `REPEATER_LOG_LEVEL` | `RNS::LOG_NOTICE` | RNS log level |
| `REPEATER_CPU_MHZ` | 80 | ESP32 CPU clock |
| `SX126X_USE_DCDC` | `true` | SX126x DC-DC regulator; set to 0 for a module without the inductor |
| `REPEATER_PRODUCT` / `REPEATER_MODEL` | per board, 868/915 MHz variant | Identity written by self-provisioning |
| `REPEATER_LORA_FREQ` / `_BW` / `_SF` / `_CR` / `_TXP` | unset | Optional build-time radio defaults (all five or none) |

## Measuring power

The figures above are estimates. To measure your own build:

1. Put a USB power meter or a µCurrent/PPK2-style ammeter **in series with the
   cell** (not the USB port), with the light's controller in circuit.
2. Let the node run in transport mode for 10 minutes, then read the average.
   Quiet-channel draw for the XIAO should be single-digit mA. Spikes of about
   120 mA during transmit are normal.
3. Compare with the plain `xiao_nrf52840` build to see the profile's effect.

Please share your numbers so the estimates can be replaced with data.

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| Boots but never transmits; console shows "Device unprovisioned" | You flashed a non-repeater build. Use a `*-repeater` env, or provision with `rnodeconf --rom`. |
| "RNS transport mode is DISABLED" in the logs | Radio parameters are not set. Run the `rnodeconf --tnc …` command above. |
| Node dies overnight, recovers in the afternoon | Hibernation is working: the cell is too small, or the panel is shaded. Clean and re-aim the panel, use a bigger cell, or lower `--txp`. |
| Node never hibernates when the cell is low | The board has no battery sensing (XIAO ESP32S3), or the reading is below `REPEATER_NO_BATTERY_V`. Check the voltage reported in the console. |
| Cell voltage creeps above 3.65 V on a LiFePO4 build | Something is charging it as Li-ion (USB on the XIAO, or a Li-ion charger). Disconnect it immediately. |
| Brownouts or boot loops when the radio transmits | The node is powered through the light's LED output or long, thin leads. Wire it straight to the cell terminals. |

## Limitations

* No LoRa board was available while this was written, so the power numbers
  are datasheet estimates. Builds are verified for every repeater env; field
  measurements are welcome.
* Hibernation needs battery sensing. The XIAO ESP32S3 has none, so it runs
  until the cell or the board's own protection cuts out. Its build prints a
  `#warning` to say so.
* After a cold boot (for example when a cell's protection circuit releases),
  the radio starts once the cell is above the cutoff, not the resume voltage.
  If the cell is still weak, the node runs briefly, hibernates again, and
  then waits for the resume voltage.
* The XIAO's 2 MB QSPI flash is not used yet, so the path table lives in
  internal flash: 25 records, like the other internal-flash nRF52 targets.
* Heltec T114 and ProMicro "Faketec" boards are good low-power candidates but
  have no release target yet. Their pin maps are recorded in
  [research/03-nrf52-board-definitions.md](research/03-nrf52-board-definitions.md).
* The radio listens continuously. SX126x receive duty cycling could cut
  receive current further, but it needs long preambles from every sender on
  the network, so it is not used.
