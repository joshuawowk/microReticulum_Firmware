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

// Headless repeater profile (-DHEADLESS_REPEATER).
//
// Turns a board into an unattended, battery/solar powered Reticulum
// transport node:
//
//  - Self-provisions an unprovisioned EEPROM, so a freshly flashed board
//    boots without rnodeconf (radio parameters still have to be set once,
//    unless the build supplies REPEATER_LORA_* defaults).
//  - Sleeps the CPU whenever the main loop has nothing to do. The loop task
//    blocks on a FreeRTOS task notification that the LoRa receive interrupt
//    gives, so FreeRTOS tickless idle can put the CPU to sleep
//    (sd_app_evt_wait on nRF52, waiti on ESP32) without adding receive
//    latency. It never sleeps while a host is attached over USB (nRF52), a
//    BLE client is connected, or packets are queued for transmission.
//  - Hibernates when the battery runs low and restarts by itself once it
//    has recovered, instead of browning out repeatedly or flattening the
//    cell. nRF52 stays in System ON sleep with the radio asleep and checks
//    the battery every REPEATER_HIBERNATE_CHECK_S; ESP32 uses timed deep
//    sleep. Neither needs a button press to come back.
//  - Keeps LEDs dark, lowers the ESP32 clock, enables the nRF52 DC/DC
//    regulator on boards that have the inductor fitted, and runs SX126x
//    radios from their DC-DC converter.
//
// Configuration lives in RepeaterConfig.h.

#ifndef REPEATER_H
#define REPEATER_H

#if defined(HEADLESS_REPEATER)

#if MCU_VARIANT == MCU_NRF52
  // The hibernation loop wakes to feed the watchdog between battery checks
  static_assert(REPEATER_HIBERNATE_CHECK_S < WDT_TIMEOUT, "REPEATER_HIBERNATE_CHECK_S must be shorter than WDT_TIMEOUT");
#endif

#if MCU_VARIANT == MCU_ESP32
  #include <esp_sleep.h>
  #include <driver/gpio.h>
  #define REPEATER_ISR_ATTR IRAM_ATTR
  // Survive deep sleep so a timer wake knows it is part of a hibernation
  RTC_DATA_ATTR bool repeater_rtc_hibernating = false;
  RTC_DATA_ATTR uint8_t repeater_rtc_good_checks = 0;
#else
  #define REPEATER_ISR_ATTR
#endif

static TaskHandle_t repeater_loop_task = NULL;
static uint16_t repeater_low_battery_s = 0;
static uint32_t repeater_last_battery_check = 0;

// Called from the LoRa drivers' DIO interrupt handlers
void REPEATER_ISR_ATTR repeater_wake_from_isr() {
  if (repeater_loop_task != NULL) {
    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(repeater_loop_task, &woken);
    portYIELD_FROM_ISR(woken);
  }
}

static uint8_t repeater_eeprom_read(uint8_t addr) {
  #if HAS_EEPROM
    return EEPROM.read(eeprom_addr(addr));
  #elif MCU_VARIANT == MCU_NRF52
    return eeprom_read(eeprom_addr(addr));
  #endif
}

// Write device identity (and optional radio defaults) to an unprovisioned
// EEPROM. A board already provisioned by rnodeconf keeps its identity.
static void repeater_provision() {
  if (!eeprom_lock_set()) {
    eeprom_update(eeprom_addr(ADDR_PRODUCT), REPEATER_PRODUCT);
    eeprom_update(eeprom_addr(ADDR_MODEL), REPEATER_MODEL);
    eeprom_update(eeprom_addr(ADDR_HW_REV), 0x01);
    for (uint8_t i = 0; i < 4; i++) {
      eeprom_update(eeprom_addr(ADDR_SERIAL+i), device_uid[DEVICE_UID_LEN-4+i]);
      eeprom_update(eeprom_addr(ADDR_MADE+i), 0x00);
    }

    char info[CHECKSUMMED_SIZE];
    for (uint8_t i = 0; i < CHECKSUMMED_SIZE; i++) { info[i] = repeater_eeprom_read(i); }
    unsigned char *hash = MD5::make_hash(info, CHECKSUMMED_SIZE);
    for (uint8_t i = 0; i < 16; i++) { eeprom_update(eeprom_addr(ADDR_CHKSUM+i), hash[i]); }
    free(hash);

    eeprom_update(eeprom_addr(ADDR_INFO_LOCK), INFO_LOCK_BYTE);
    printf("[repeater] Self-provisioned device identity (product %02x, model %02x)\n", REPEATER_PRODUCT, REPEATER_MODEL);
  }

  #if defined(REPEATER_HAS_RADIO_DEFAULTS)
    if (!eeprom_have_conf()) {
      const uint32_t freq = REPEATER_LORA_FREQ;
      const uint32_t bw = REPEATER_LORA_BW;
      eeprom_update(eeprom_addr(ADDR_CONF_SF), REPEATER_LORA_SF);
      eeprom_update(eeprom_addr(ADDR_CONF_CR), REPEATER_LORA_CR);
      eeprom_update(eeprom_addr(ADDR_CONF_TXP), REPEATER_LORA_TXP);
      for (uint8_t i = 0; i < 4; i++) {
        eeprom_update(eeprom_addr(ADDR_CONF_BW)+i, (bw >> (24-8*i)) & 0xFF);
        eeprom_update(eeprom_addr(ADDR_CONF_FREQ)+i, (freq >> (24-8*i)) & 0xFF);
      }
      eeprom_update(eeprom_addr(ADDR_CONF_OK), CONF_OK_BYTE);
      printf("[repeater] Applied build-time radio defaults (%lu Hz, %lu Hz BW, SF%d, CR%d, %d dBm)\n",
        (unsigned long)freq, (unsigned long)bw, REPEATER_LORA_SF, REPEATER_LORA_CR, REPEATER_LORA_TXP);
    }
  #endif

  #if !HAS_EEPROM && MCU_VARIANT == MCU_NRF52
    eeprom_flush();
  #endif
}

// Early, board-level power setup. Runs once EEPROM is available, before
// the radio, display and Bluetooth are initialised.
void repeater_setup() {
  repeater_loop_task = xTaskGetCurrentTaskHandle();

  #if MCU_VARIANT == MCU_ESP32
    setCpuFrequencyMhz(REPEATER_CPU_MHZ);
  #endif

  #if MCU_VARIANT == MCU_NRF52 && HAS_NRF_DCDC
    // SoftDevice is not enabled yet, so the register can be written directly
    NRF_POWER->DCDCEN = 1;
  #endif

  #if BOARD_MODEL == BOARD_HELTEC32_V4
    // Vext feeds the OLED and GNSS header, neither of which a repeater uses
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, HIGH);
  #endif

  repeater_provision();
}

// Sample the battery BAT_SAMPLES times so the averaged reading is fresh
static float repeater_sample_battery() {
  #if HAS_PMU
    #if defined(BAT_SAMPLES)
      for (uint8_t i = 0; i < BAT_SAMPLES; i++) { measure_battery(); delay(2); }
    #else
      measure_battery(); // PMIC boards report an already filtered voltage
    #endif
    if (battery_ready) { return battery_voltage; }
  #endif
  return 0.0;
}

static bool repeater_battery_low(float voltage, float threshold) {
  return voltage > REPEATER_NO_BATTERY_V && voltage < threshold;
}

// A host on USB (nRF52 native CDC) means someone is at the bench: stay
// fully awake and never hibernate.
static bool repeater_host_attached() {
  #if MCU_VARIANT == MCU_NRF52
    return (bool)Serial;
  #else
    return false;
  #endif
}

#if MCU_VARIANT == MCU_ESP32
static void repeater_deep_sleep() {
  #if BOARD_MODEL == BOARD_HELTEC32_V4
    digitalWrite(LORA_PA_CPS, LOW);
    digitalWrite(LORA_PA_CSD, LOW);
    digitalWrite(LORA_PA_PWR_EN, LOW);
    digitalWrite(Vext, HIGH);
  #endif
  #if defined(LORA_TRANSPORT)
    // GPIOs float in deep sleep, and a falling edge on NSS wakes a sleeping
    // SX126x into standby. Hold NSS high until the next boot releases it.
    pinMode(pin_cs, OUTPUT);
    digitalWrite(pin_cs, HIGH);
    gpio_hold_en((gpio_num_t)pin_cs);
    gpio_deep_sleep_hold_en();
  #endif
  Serial.flush();
  esp_sleep_enable_timer_wakeup((uint64_t)REPEATER_HIBERNATE_CHECK_S * 1000000ULL);
  esp_deep_sleep_start();
}
#endif

// Stop radio and Bluetooth, then sleep until the battery recovers. Never
// returns: recovery restarts the MCU.
void repeater_hibernate(float voltage) {
  printf("[repeater] Battery at %.2f V (cutoff %.2f V), hibernating until %.2f V\n", voltage, REPEATER_SHUTDOWN_V, REPEATER_RESUME_V);

  #ifdef HAS_RNS
    // Save the path table while there is still charge to write flash
    if (reticulum) {
      try { reticulum.persist_data(); }
      catch (std::exception& e) { }
    }
  #endif

  #if defined(LORA_TRANSPORT)
    if (radio_online) { stopRadio(); }
    else if (modem_installed) { LoRa->sleep(); }
  #endif

  #if HAS_BLUETOOTH || HAS_BLE == true
    if (bt_state != BT_STATE_OFF && bt_state != BT_STATE_NA) { bt_stop(); }
    #if MCU_VARIANT == MCU_NRF52
      // bt_stop() only updates the state on nRF52; also stop advertising
      // and drop any client so the SoftDevice radio goes quiet.
      if (bt_ready) {
        Bluefruit.Advertising.restartOnDisconnect(false);
        Bluefruit.Advertising.stop();
        for (uint16_t conn = 0; conn < BLE_MAX_CONNECTION; conn++) {
          if (Bluefruit.connected(conn)) { Bluefruit.disconnect(conn); }
        }
      }
    #endif
  #endif

  led_rx_off();
  led_tx_off();

  #if MCU_VARIANT == MCU_ESP32
    repeater_rtc_hibernating = true;
    repeater_rtc_good_checks = 0;
    repeater_deep_sleep();
  #elif MCU_VARIANT == MCU_NRF52
    uint8_t good_checks = 0;
    while (true) {
      NRF_WDT->RR[0] = WDT_RR_RR_Reload;
      // vTaskDelay: tickless idle keeps the CPU in System ON sleep
      delay((uint32_t)REPEATER_HIBERNATE_CHECK_S * 1000);
      NRF_WDT->RR[0] = WDT_RR_RR_Reload;

      float v = repeater_sample_battery();
      if (repeater_host_attached() || !repeater_battery_low(v, REPEATER_RESUME_V)) {
        if (repeater_host_attached() || ++good_checks >= REPEATER_RESUME_CHECKS) {
          printf("[repeater] Battery recovered (%.2f V), restarting\n", v);
          delay(100);
          hard_reset();
        }
      } else {
        good_checks = 0;
      }
    }
  #endif
}

// ESP32: called right after Serial.begin(). If this boot is a timer wake
// from hibernation, check the battery before doing anything expensive and
// go straight back to deep sleep if it has not recovered yet.
void repeater_wake_check() {
  #if MCU_VARIANT == MCU_ESP32
    if (!repeater_rtc_hibernating || esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER) {
      // Normal boot, or woken by a button or reset (a person is here)
      repeater_rtc_hibernating = false;
      #if defined(LORA_TRANSPORT)
        gpio_hold_dis((gpio_num_t)pin_cs);
      #endif
      return;
    }
    #if HAS_PMU
      init_pmu();
    #endif
    float v = repeater_sample_battery();
    if (!repeater_battery_low(v, REPEATER_RESUME_V)) {
      if (++repeater_rtc_good_checks >= REPEATER_RESUME_CHECKS) {
        repeater_rtc_hibernating = false;
        #if defined(LORA_TRANSPORT)
          gpio_hold_dis((gpio_num_t)pin_cs);
        #endif
        printf("[repeater] Battery recovered (%.2f V), resuming\n", v);
        return;
      }
    } else {
      repeater_rtc_good_checks = 0;
    }
    repeater_deep_sleep();
  #endif
}

// Called just before the radio is started. Refuse to bring the radio up on
// a battery that is already below the cutoff, which would otherwise end in
// a brown-out boot loop at dawn.
void repeater_boot_check() {
  #if HAS_PMU
    if (!pmu_ready || repeater_host_attached()) { return; }
    float v = repeater_sample_battery();
    if (repeater_battery_low(v, REPEATER_SHUTDOWN_V)) { repeater_hibernate(v); }
  #endif
}

// Called at the end of each top-level loop() pass
void repeater_loop() {
  #if HAS_PMU
    if (pmu_ready && millis() - repeater_last_battery_check >= 1000) {
      repeater_last_battery_check = millis();
      if (battery_ready && repeater_battery_low(battery_voltage, REPEATER_SHUTDOWN_V) && !repeater_host_attached()) {
        if (++repeater_low_battery_s >= REPEATER_SHUTDOWN_DELAY_S) { repeater_hibernate(battery_voltage); }
      } else {
        repeater_low_battery_s = 0;
      }
    }
  #endif

  // Stay awake while there is work that needs prompt polling
  if (queue_height > 0) { return; }
  if (!fifo_isempty(&serialFIFO)) { return; }
  if (console_active) { return; }
  if (repeater_host_attached()) { return; }
  #if HAS_BLUETOOTH || HAS_BLE == true
    if (bt_state == BT_STATE_CONNECTED || bt_state == BT_STATE_PAIRING) { return; }
  #endif
  #if HAS_WIFI
    if (wifi_initialized) { return; }
  #endif

  ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(REPEATER_IDLE_MS));
}

#endif // HEADLESS_REPEATER

#endif // REPEATER_H
