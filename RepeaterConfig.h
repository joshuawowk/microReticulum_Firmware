// Copyright (C) 2026, microReticulum_Firmware contributors

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Compile-time settings for the headless repeater profile. Everything here is
// inert unless the build defines HEADLESS_REPEATER (see the *-repeater envs
// in platformio.ini and docs/solar-repeater/README.md). Every value can be
// overridden with -D in build_flags.

#ifndef REPEATER_CONFIG_H
#define REPEATER_CONFIG_H

#if defined(HEADLESS_REPEATER)

  // Battery chemistry selects the low-voltage cutoff and recovery points.
  // Li-ion/LiPo (3.7 V nominal, 4.2 V full) or LiFePO4 (3.2 V nominal,
  // 3.6 V full). Many solar garden lights ship LiFePO4 cells.
  #define REPEATER_BATTERY_LIION   1
  #define REPEATER_BATTERY_LIFEPO4 2
  #ifndef REPEATER_BATTERY
    #define REPEATER_BATTERY REPEATER_BATTERY_LIION
  #endif

  #if REPEATER_BATTERY == REPEATER_BATTERY_LIFEPO4
    #ifndef REPEATER_SHUTDOWN_V
      #define REPEATER_SHUTDOWN_V 2.95
    #endif
    #ifndef REPEATER_RESUME_V
      #define REPEATER_RESUME_V   3.25
    #endif
    // Battery percentage scale reported over KISS / Provisioning / BLE
    #define BAT_PCT_V_MIN 2.90
    #define BAT_PCT_V_MAX 3.40
  #else
    #ifndef REPEATER_SHUTDOWN_V
      #define REPEATER_SHUTDOWN_V 3.30
    #endif
    #ifndef REPEATER_RESUME_V
      #define REPEATER_RESUME_V   3.70
    #endif
  #endif

  #if !HAS_PMU
    #warning "HEADLESS_REPEATER: this board has no battery sensing, so low-battery hibernation is disabled"
  #endif

  // Readings below this are treated as "no battery fitted" (USB or bench
  // supply only) and never trigger a shutdown.
  #ifndef REPEATER_NO_BATTERY_V
    #define REPEATER_NO_BATTERY_V 2.0
  #endif

  // Seconds the (already averaged) battery voltage must stay below
  // REPEATER_SHUTDOWN_V before the node hibernates, so transmit sags and
  // brief clouds do not trip it.
  #ifndef REPEATER_SHUTDOWN_DELAY_S
    #define REPEATER_SHUTDOWN_DELAY_S 60
  #endif

  // While hibernating, the battery is re-checked this often. On nRF52 the
  // MCU stays in System ON sleep and must wake to feed the 60 s watchdog,
  // so keep it below WDT_TIMEOUT. On ESP32 each check is a deep-sleep wake
  // and costs a short boot, so it is spaced further apart.
  #ifndef REPEATER_HIBERNATE_CHECK_S
    #if defined(NRF52840_XXAA)
      #define REPEATER_HIBERNATE_CHECK_S 30
    #else
      #define REPEATER_HIBERNATE_CHECK_S 300
    #endif
  #endif

  // Consecutive checks at or above REPEATER_RESUME_V needed to restart.
  #ifndef REPEATER_RESUME_CHECKS
    #define REPEATER_RESUME_CHECKS 3
  #endif

  // Longest time the main loop sleeps when there is nothing to do. A LoRa
  // receive interrupt ends the sleep immediately.
  #ifndef REPEATER_IDLE_MS
    #define REPEATER_IDLE_MS 50
  #endif

  // RNS log level. Logs go to the USB serial port as KISS frames; NOTICE
  // keeps startup, warnings and battery events without per-packet traces.
  #ifndef REPEATER_LOG_LEVEL
    #define REPEATER_LOG_LEVEL RNS::LOG_NOTICE
  #endif

  // ESP32 CPU clock. 80 MHz keeps APB at 80 MHz, so UART/SPI timing and
  // Bluetooth are unaffected, at roughly half the active current of 240 MHz.
  #ifndef REPEATER_CPU_MHZ
    #define REPEATER_CPU_MHZ 80
  #endif

  // Run the SX126x from its internal DC-DC converter instead of the LDO,
  // which roughly halves receive current. Every SX1262 module this firmware
  // supports has the DC-DC inductor fitted (MeshCore and RadioLib default to
  // DC-DC on all of them); set to 0 for a module that lacks it.
  #ifndef SX126X_USE_DCDC
    #define SX126X_USE_DCDC true
  #endif

  // Self-provisioning: product and model written to an unprovisioned
  // EEPROM so the node boots without rnodeconf. The defaults cover the
  // 868/915 MHz variants; override for 433 MHz hardware.
  #ifndef REPEATER_PRODUCT
    #if BOARD_MODEL == BOARD_RAK4631
      #define REPEATER_PRODUCT PRODUCT_RAK4631
      #define REPEATER_MODEL   MODEL_12
    #elif BOARD_MODEL == BOARD_RAK3401
      #define REPEATER_PRODUCT PRODUCT_RAK4631
      #define REPEATER_MODEL   MODEL_14
    #elif BOARD_MODEL == BOARD_XIAO_NRF52
      #define REPEATER_PRODUCT PRODUCT_HMBRW
      #define REPEATER_MODEL   MODEL_FE
    #elif BOARD_MODEL == BOARD_HELTEC_T114
      #define REPEATER_PRODUCT PRODUCT_HELTEC_T114
      #define REPEATER_MODEL   MODEL_C7
    #elif BOARD_MODEL == BOARD_TECHO
      #define REPEATER_PRODUCT PRODUCT_TECHO
      #define REPEATER_MODEL   MODEL_17
    #elif BOARD_MODEL == BOARD_HELTEC32_V3
      #define REPEATER_PRODUCT PRODUCT_H32_V3
      #define REPEATER_MODEL   MODEL_CA
    #elif BOARD_MODEL == BOARD_HELTEC32_V4
      #define REPEATER_PRODUCT PRODUCT_H32_V4
      #define REPEATER_MODEL   MODEL_C8
    #elif BOARD_MODEL == BOARD_LORA32_V2_1
      #define REPEATER_PRODUCT PRODUCT_T32_21
      #define REPEATER_MODEL   MODEL_B9
    #elif BOARD_MODEL == BOARD_XIAO_S3
      #define REPEATER_PRODUCT PRODUCT_XIAO_S3
      #define REPEATER_MODEL   MODEL_DD
    #else
      #error "HEADLESS_REPEATER: define REPEATER_PRODUCT and REPEATER_MODEL for this board"
    #endif
  #endif

  // Optional radio defaults, written only when the EEPROM holds no radio
  // configuration yet. Deliberately not set by default: frequency, bandwidth
  // and power must match your region and your network, so set all five
  // (e.g. -DREPEATER_LORA_FREQ=914875000 -DREPEATER_LORA_BW=125000
  // -DREPEATER_LORA_SF=8 -DREPEATER_LORA_CR=5 -DREPEATER_LORA_TXP=22) or
  // configure the node once with rnodeconf / the RNode Console instead.
  #if defined(REPEATER_LORA_FREQ) || defined(REPEATER_LORA_BW) || defined(REPEATER_LORA_SF) || defined(REPEATER_LORA_CR) || defined(REPEATER_LORA_TXP)
    #if !defined(REPEATER_LORA_FREQ) || !defined(REPEATER_LORA_BW) || !defined(REPEATER_LORA_SF) || !defined(REPEATER_LORA_CR) || !defined(REPEATER_LORA_TXP)
      #error "HEADLESS_REPEATER: set all of REPEATER_LORA_FREQ, _BW, _SF, _CR and _TXP, or none"
    #endif
    #define REPEATER_HAS_RADIO_DEFAULTS
  #endif

#endif // HEADLESS_REPEATER

#ifndef SX126X_USE_DCDC
  #define SX126X_USE_DCDC false
#endif

#endif // REPEATER_CONFIG_H
