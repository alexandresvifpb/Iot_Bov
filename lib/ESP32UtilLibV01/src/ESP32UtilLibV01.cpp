#include "ESP32UtilLibV01.h"

esp_adc_cal_characteristics_t adc_cal;

// function constructor of the class ESP32Lib
ESP32UtilLib::ESP32UtilLib() {}

// uses the MAC of the ESP32 for generate the device ID
String ESP32UtilLib::get_MAC(void) {
    char chipID_0[4];
    char chipID_1[4];
    char chipID_2[4];
    uint64_t chipID = ESP.getEfuseMac();
    Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipID>>32));
    Serial.printf("%08X\n",(uint32_t)chipID);

    uint8_t id8 = (uint8_t)(chipID>>24);
    (id8 > 9) ? sprintf(chipID_0, "%0X", id8) : sprintf(chipID_0, "0%0X", id8);

    uint16_t id16 = (uint8_t)(chipID>>32);
    (id16 > 9) ? sprintf(chipID_1, "%0X", id16) : sprintf(chipID_1, "0%0X", id16);

    uint32_t id32 = (uint8_t)(chipID>>40);
    (id32 > 9) ? sprintf(chipID_2, "%0X", id32) : sprintf(chipID_2, "0%0X", id32);

    return String(chipID_0) + String(chipID_1) + String(chipID_2);
}

// Hello SHA 256 from ESP32learning
uint8_t ESP32UtilLib::hash(String str) {
  char payload[32];
  str.substring(1,32).toCharArray(payload, 32);

  uint8_t _hash = 0;
  for (uint8_t i = 0; i < 32; i++) {
    _hash += int(payload[i]);
  }
  
  return _hash;
}

// Read from flash memory
uint16_t ESP32UtilLib::getFlashUInt16(uint8_t address) {
    return word(EEPROM.read(address), EEPROM.read(address + 1));
}

// Writes to flash memory
void ESP32UtilLib::setFlashUInt16(uint8_t address, uint16_t value) {
    EEPROM.write(address,highByte(value));
    EEPROM.write(address + 1,lowByte(value));
}