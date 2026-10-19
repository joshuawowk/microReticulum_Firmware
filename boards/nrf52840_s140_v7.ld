/* Linker script to configure memory regions.
 *
 * nRF52840 with SoftDevice S140 7.3.0 (e.g. the Seeed XIAO nRF52840 factory
 * bootloader). Identical to the Adafruit core's nrf52840_s140_v6.ld except
 * that S140 7.x occupies one more 4 KB page, so the application starts at
 * 0x27000 instead of 0x26000. The Adafruit core does not ship this file.
 */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx)     : ORIGIN = 0x27000, LENGTH = 0xED000 - 0x27000

  /* SRAM required by Softdevice depend on
   * - Attribute Table Size (Number of Services and Characteristics)
   * - Vendor UUID count
   * - Max ATT MTU
   * - Concurrent connection peripheral + central + secure links
   * - Event Len, HVN queue, Write CMD queue
   */
  RAM (rwx) :  ORIGIN = 0x20006000, LENGTH = 0x20040000 - 0x20006000
}

SECTIONS
{
  . = ALIGN(4);
  .svc_data :
  {
    PROVIDE(__start_svc_data = .);
    KEEP(*(.svc_data))
    PROVIDE(__stop_svc_data = .);
  } > RAM

  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
} INSERT AFTER .data;

INCLUDE "nrf52_common.ld"
