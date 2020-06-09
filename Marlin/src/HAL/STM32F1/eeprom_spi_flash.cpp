/**
 * eeprom_spi_flash.cpp
 * HAL for stm32duino and compatible (STM32F1)
 * stores EEPROM settings in SPI connected flash
 */

#ifdef __STM32F1__

#include "../../inc/MarlinConfig.h"

#if ENABLED(SPI_FLASH_EEPROM_EMULATION)

#include "../shared/eeprom_api.h"

#include <SPI.h>
#include "spi_flash.h"

#define SPI_FLASH_EEPROM_SIZE 0x1000 // 4K - do not change!

static uint8_t ram_eeprom[SPI_FLASH_EEPROM_SIZE] __attribute__((aligned(4))) = {0};
static bool eeprom_dirty = false;

SPIClass _spi(SPI_FLASH_DEVICE);
hal::SPIFlash _flash(_spi, SPI_FLASH_CS_PIN);

bool PersistentStore::access_start() {
  _flash.begin();
  _flash.read_array(SPI_FLASH_EEPROM_OFFSET,  ram_eeprom, SPI_FLASH_EEPROM_SIZE);
  _flash.end();
  eeprom_dirty = false;
  return true;
}

bool PersistentStore::access_finish() {
  if (eeprom_dirty) {
    _flash.begin();
    _flash.erase_4K(SPI_FLASH_EEPROM_OFFSET);
    _flash.write_array(SPI_FLASH_EEPROM_OFFSET,  ram_eeprom, SPI_FLASH_EEPROM_SIZE);
    _flash.end();
    eeprom_dirty = false;
  }
  return false;
}

bool PersistentStore::write_data(int &pos, const uint8_t *value, size_t size, uint16_t *crc) {
  for (size_t i = 0; i < size; i++) {
    ram_eeprom[pos + i] = value[i];
  }
  eeprom_dirty = true;
  crc16(crc, value, size);
  pos += size;
  return false;
}

bool PersistentStore::read_data(int &pos, uint8_t* value, const size_t size, uint16_t *crc, const bool writing/*=true*/) {
  for (size_t i = 0; i < size; i++) {
    uint8_t c = ram_eeprom[pos + i];
    if (writing) value[i] = c;
    crc16(crc, &c, 1);
  }
  pos += size;
  return false;
}

size_t PersistentStore::capacity() {
  return SPI_FLASH_EEPROM_SIZE;
}

#endif // SPI_FLASH_EEPROM_EMULATION
#endif // __STM32F1__
