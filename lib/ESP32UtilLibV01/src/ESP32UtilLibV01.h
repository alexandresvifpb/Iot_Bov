#ifndef ESP32UTILLIBV01_H 
#define ESP32UTILLIBV01_H

#include "ArduinoJson.h"
#include "LinkedList.h"
#include "TimeLib.h"
#include "EEPROM.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <time.h>
#include <sys/time.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <mbedtls/md.h>

// #define GATEWAY     0
// #define NODE        1

// #define BEACON      0
// #define JOIN        1
// #define JOIN_ACK    2
// #define REQUEST     3
// #define REPLAY      4

#ifdef __cplusplus
extern "C" {
#endif

class ESP32UtilLib
{
  public:
    ESP32UtilLib();

    String get_MAC(void);
    uint8_t hash(String str);

  private:
    uint16_t getFlashUInt16(uint8_t address);
    void setFlashUInt16(uint8_t address, uint16_t value);

};

#ifdef __cplusplus
}
#endif

#endif  // ESP32UTILLIBV01_H