# Research: nRF52840 + SX1262 board definitions (XIAO kit, ProMicro "Faketec")

Collected 2026-10-19 from source at pinned commits:

* Meshtastic `97bf6e48c09a045abd67ee762f92e199db521683` —
  `https://github.com/meshtastic/firmware/blob/97bf6e48c09a045abd67ee762f92e199db521683/`
* MeshCore `81dbae592ffad40b4212bc0b37e82b38da08f4bc` —
  `https://github.com/meshcore-dev/MeshCore/blob/81dbae592ffad40b4212bc0b37e82b38da08f4bc/`
* Adafruit nRF52 core `343ab5fe7fb3d5657d3176883dfb188d65b79f06`

## Findings that shaped the implementation

1. **No S140 v7 linker script for the nRF52840 exists in the Adafruit core**
   (`cores/nRF5/linker/` has only `nrf52832_s132_v6.ld`, `nrf52833_s140_v7.ld`,
   `nrf52840_s140_v6.ld`, `nrf52_common.ld`). The XIAO ships an S140 **7.3.0**
   bootloader, so both Meshtastic and MeshCore carry their own
   `nrf52840_s140_v7.ld`. This repo now does too (`boards/nrf52840_s140_v7.ld`).
2. **The Adafruit core never enables the nRF52840 DC/DC regulator.** Meshtastic
   doesn't either. MeshCore does (`NRF52BoardDCDC::begin()` in
   `src/helpers/NRF52Board.cpp`: `sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE)`
   when the SoftDevice is enabled, else `NRF_POWER->DCDCEN = 1`) for both the XIAO
   and ProMicro boards. Only safe when the DC/DC inductor is fitted.
3. Meshtastic's `nrf52_promicro_diy_xtal` variant was removed;
   `nrf52_promicro_diy_tcxo` handles both via `TCXO_OPTIONAL`.

## A) Seeed XIAO nRF52840 + Wio-SX1262 kit

### Build configuration (Meshtastic `variants/nrf52840/seeed_xiao_nrf52840_kit/platformio.ini`)

```ini
board = xiao_ble_sense
board_build.ldscript = src/platform/nrf52/nrf52840_s140_v7.ld
build_flags = ... -DCONFIG_NFCT_PINS_AS_GPIOS=1
```

Board JSON essentials: `mcu nrf52840`, `f_cpu 64000000L`,
`extra_flags -DARDUINO_MDBT50Q_RX -DNRF52840_XXAA`, softdevice
`{sd_flags -DS140, name s140, version 7.3.0, sd_fwid 0x0123}`,
`bootloader.settings_addr 0xFF000`, upload `nrfutil` with 1200 bps touch,
`maximum_size 815104`, `maximum_ram_size 248832`. Seeed bootloader USB ids
`0x2886:0x0045` (bootloader) / `0x2886:0x8045` (app); Meshtastic's JSON lists
`0x2886:0x0166`.

### Pin map (`g_ADigitalPinMap`, identical in Meshtastic and MeshCore)

| Arduino | GPIO | Port.pin | Use |
|---|---|---|---|
| D0 | 2 | P0.02 | |
| D1 | 3 | P0.03 | **LoRa DIO1** |
| D2 | 28 | P0.28 | **LoRa RESET** |
| D3 | 29 | P0.29 | **LoRa BUSY** |
| D4 | 4 | P0.04 | **LoRa NSS** |
| D5 | 5 | P0.05 | **LoRa RXEN** (RF switch) |
| D6 | 43 | P1.11 | |
| D7 | 44 | P1.12 | |
| D8 | 45 | P1.13 | SPI SCK |
| D9 | 46 | P1.14 | SPI MISO |
| D10 | 47 | P1.15 | SPI MOSI |
| 11 | 26 | P0.26 | LED red (active LOW) |
| 12 | 6 | P0.06 | LED blue (active LOW) |
| 13 | 30 | P0.30 | LED green (active LOW) |
| 14 | 14 | P0.14 | READ_BAT / VBAT_ENABLE (active LOW) |
| 15 | 40 | P1.08 | IMU power (Sense) |
| 16 | 27 | P0.27 | IMU SCL |
| 17 | 7 | P0.07 | IMU SDA |
| 18 | 11 | P0.11 | IMU INT1 |
| 19 | 42 | P1.10 | MIC power |
| 20 | 32 | P1.00 | PDM CLK |
| 21 | 16 | P0.16 | PDM DATA |
| 22 | 13 | P0.13 | HICHG (LOW = 100 mA charge) |
| 23 | 17 | P0.17 | ~CHG (charging, active LOW) |
| 24 | 21 | P0.21 | QSPI SCK |
| 25 | 25 | P0.25 | QSPI CS |
| 26 | 20 | P0.20 | QSPI IO0 |
| 27 | 24 | P0.24 | QSPI IO1 |
| 28 | 22 | P0.22 | QSPI IO2 |
| 29 | 23 | P0.23 | QSPI IO3 |
| 30 | 9 | P0.09 | NFC1 |
| 31 | 10 | P0.10 | NFC2 |
| 32 | 31 | P0.31 | VBAT sense (AIN7) |

Other variant facts: `USE_LFXO` (32 kHz crystal), `PINS_COUNT 33`,
`ADC_MULTIPLIER 3` (R17 = 1 MΩ, R18 = 510 kΩ → ×2.96), on-board P25Q16H QSPI flash
(drive CS P0.25 HIGH early), BQ25101 charger.

### SX1262 wiring (Wio-SX1262 on the kit's B2B connector)

NSS D4, DIO1 D1, BUSY D3, RESET D2, RXEN D5, TXEN none, SCK D8, MISO D9, MOSI
D10, **DIO2 as RF switch**, **DIO3 TCXO 1.8 V**, no radio power-enable pin.
Meshtastic and MeshCore agree exactly (MeshCore also sets
`SX126X_CURRENT_LIMIT=140`, `SX126X_RX_BOOSTED_GAIN=1`, `LORA_TX_POWER=22`).

### Power management notes

* VBAT divider enable P0.14 is **active LOW**. MeshCore holds it LOW permanently
  (`XiaoNrf52Board::begin()`), with `AR_INTERNAL_3_0`, 12-bit, 50 ms settle.
  The Seeed wiki warns against driving P0.14 HIGH while charging.
* MeshCore low-voltage shutdown keeps P0.14 LOW so LPCOMP can watch AIN7
  (`PWRMGT_LPCOMP_AIN 7`, `PWRMGT_LPCOMP_REFSEL 2` = 3/8 VDD,
  `PWRMGT_VOLTAGE_BOOTLOCK 3300`).

## B) ProMicro nRF52840 + SX1262 ("Faketec")

Not implemented in this work; recorded for a follow-up.

* Board JSON `promicro-nrf52840`: S140 **6.1.1** (fwid 0x00B6),
  `nrf52840_s140_v6.ld`, nice!nano-style UF2 bootloader.
* Identity pin map (Arduino n = GPIO n, P1.xx = 32 + xx). `USE_LFRC` (no 32 kHz crystal).
* SX1262: NSS P1.13 (45), DIO1 P0.10 (10), BUSY P0.29 (29), RESET P0.09 (9),
  RXEN P0.17 (17), TXEN none, SCK P1.11 (43), MOSI P1.15 (47), MISO P0.02 (2),
  DIO2 RF switch, DIO3 TCXO 1.8 V (optional), 3V3 rail enable P0.13 HIGH.
* LED P0.15 active HIGH; button P1.00; battery P0.31 (divider varies by build).

## Adafruit core behaviour relevant to low power

* `loop_task` (`cores/nRF5/main.cpp`): `while (1) { loop(); yield(); ... }`;
  `yield()` = `TinyUSB_Device_FlushCDC(); taskYIELD();` — **no sleep**.
* `delay()` = flush CDC + `vTaskDelay()` → lets the idle task run.
* `configUSE_TICKLESS_IDLE 1`, `configTICK_RATE_HZ 1024`;
  `vPortSuppressTicksAndSleep` calls `sd_app_evt_wait()` (SoftDevice enabled) or
  `__WFE()` — so blocking the loop task actually sleeps the CPU.

## Meshtastic / MeshCore reference behaviour

* Meshtastic main loop blocks on a FreeRTOS semaphore (`InterruptableDelay`);
  `loopCanSleep()` returns `!Serial` (no sleeping while USB is connected).
* Meshtastic low battery → `cpuDeepSleep` → System OFF, with optional LPCOMP
  wake only for variants defining `BATTERY_LPCOMP_INPUT` (XIAO/ProMicro don't).
* MeshCore `NRF52Board::enterSystemOff()` + `configureVoltageWake()` (LPCOMP UP)
  and `checkBootVoltage()` boot lock.
