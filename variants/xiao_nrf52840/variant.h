#pragma once

#ifndef _VARIANT_XIAO_NRF52840_
#define _VARIANT_XIAO_NRF52840_

// Seeed Studio XIAO nRF52840 (and XIAO nRF52840 Sense / Plus), as used in the
// "XIAO nRF52840 & Wio-SX1262 Kit". Pin map follows Seeed's schematic and
// matches the Meshtastic (seeed_xiao_nrf52840_kit) and MeshCore (xiao_nrf52)
// variants.

/** Master clock frequency */
#define VARIANT_MCK (64000000ul)

#define USE_LFXO // Board uses 32khz crystal for LF
// define USE_LFRC    // Board uses RC for LF

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define PINS_COUNT              (33)
#define NUM_DIGITAL_PINS        (33)
#define NUM_ANALOG_INPUTS       (8)
#define NUM_ANALOG_OUTPUTS      (0)

// LEDs (common anode, active LOW)
#define LED_RED                 (11)
#define LED_GREEN               (13)
#define LED_BLUE                (12)
#define PIN_LED1                (LED_GREEN)
#define PIN_LED2                (LED_BLUE)
#define PIN_LED                 (PIN_LED1)
#define LED_BUILTIN             (PIN_LED)
#define LED_CONN                (PIN_LED2)
#define LED_STATE_ON            (0)

// Buttons (the XIAO has no user button; D0 is free on the kit)
#define PIN_BUTTON1             (0)

// Digital pins
static const uint8_t D0  = 0;
static const uint8_t D1  = 1;
static const uint8_t D2  = 2;
static const uint8_t D3  = 3;
static const uint8_t D4  = 4;
static const uint8_t D5  = 5;
static const uint8_t D6  = 6;
static const uint8_t D7  = 7;
static const uint8_t D8  = 8;
static const uint8_t D9  = 9;
static const uint8_t D10 = 10;

// Battery sense. P0.14 enables the 1M/510k divider onto P0.31 (AIN7) when
// driven LOW. Seeed warns that P0.14 must not be driven HIGH while the
// battery is charging, since P0.31 would then see the full cell voltage
// (> 3.6 V). It is therefore held LOW permanently (≈ 2.8 uA divider drain).
#define VBAT_ENABLE             (14)
#define PIN_VBAT                (32)
#define PIN_CHARGING_CURRENT    (22)    // P0.13: LOW = 100 mA, input/HIGH = 50 mA
#define BAT_NOT_CHARGING        (23)    // P0.17: LOW while charging

// Analog pins
#define PIN_A0                  (0)
#define PIN_A1                  (1)
#define PIN_A2                  (2)
#define PIN_A3                  (3)
#define PIN_A4                  (4)
#define PIN_A5                  (5)

static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
static const uint8_t A3  = PIN_A3;
static const uint8_t A4  = PIN_A4;
static const uint8_t A5  = PIN_A5;

#define ADC_RESOLUTION          (12)

// Other pins
#define PIN_NFC1                (30)
#define PIN_NFC2                (31)

// Serial interfaces
#define PIN_SERIAL1_RX          (7)
#define PIN_SERIAL1_TX          (6)

// SPI interfaces. Only one is declared: the firmware drives the LoRa radio
// through its own SPIClass on NRF_SPIM2 (see Config.h), which the core would
// otherwise also hand to SPI1.
#define SPI_INTERFACES_COUNT    (1)

#define PIN_SPI_MISO            (9)
#define PIN_SPI_MOSI            (10)
#define PIN_SPI_SCK             (8)

static const uint8_t SS   = 4;
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// Wire interfaces (D4/D5 carry the SX1262 NSS/RXEN on the Wio-SX1262 kit)
#define WIRE_INTERFACES_COUNT   (1)
#define PIN_WIRE_SDA            (17)
#define PIN_WIRE_SCL            (16)

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

// On-board QSPI flash (P25Q16H)
#define PIN_QSPI_SCK            (24)
#define PIN_QSPI_CS             (25)
#define PIN_QSPI_IO0            (26)
#define PIN_QSPI_IO1            (27)
#define PIN_QSPI_IO2            (28)
#define PIN_QSPI_IO3            (29)

#ifdef __cplusplus
}
#endif

#endif
