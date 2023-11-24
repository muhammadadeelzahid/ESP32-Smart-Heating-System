/**
 * @file ble_receive.h
 * @author Patrik Stransky
 * @author Adeel Zahid (adeel.m.zahid@gmail.com)
 * @brief class implementation for ble data receiving for xiaomi sensor
 * @version 1.0
 * @date 2023-11-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ble_receive.h"
#include "sensor_data.h"
#include "Arduino.h"

extern sensor_data temp_sensor_xiaomi;
/**
 * @brief Private method of the class to find the service data in the overall report
 * @param data 
 * @param length 
 * @param delka 
 * @return *pointer 
 */
uint8_t* ble_receive::najdiData(uint8_t *data, size_t length, uint8_t *delka)
{
    uint8_t *pravyOkraj = data + length;
    Serial.println("service data function");
    while (data < pravyOkraj) 
    {
      uint8_t delkaBloku = *data + 1;
      if (delkaBloku < 5) 
      {
        data += delkaBloku;
        continue;
      }
      uint8_t typBloku = *(data + 1);
      uint16_t typSluzby = *(uint16_t *)(data + 2);
      // Pokud se jedna o korektni typ dat a tridu s UUID 0x181a,
      // vrat data
      if (typBloku == 0x16) 
      {
        if (typSluzby == 0x181a) 
        {
          *delka = delkaBloku;
          return data;
        }
      }
      data += delkaBloku;
      Serial.print("service data, while iteration - "); 
      Serial.print("*data: "); Serial.print(*data);
      Serial.print("*pravyOkraj: "); Serial.print(*pravyOkraj);
    }
    
    return nullptr;
}

/**
 * @brief Inherited method that will be processed when the message is received
 * 
 * @param zarizeni 
 */
void ble_receive::onResult(BLEAdvertisedDevice zarizeni)
{
    // Ziskej data
    uint8_t *data = zarizeni.getPayload();
    // Ziskej delku dat
    size_t delkaDat = zarizeni.getPayloadLength();
    uint8_t delkaDatSluzby = 0;
    // Vytahni ze surove zpravy data sluzby, ktera obsahuji nami hledane udaje
    uint8_t *dataSluzby = najdiData(data, delkaDat, &delkaDatSluzby);
    if (data == nullptr || delkaDatSluzby < 18)
    {
      return;
    }
    // Pokud maji data sluzby delku 19 bajtu, je to to, co hledame
    if (delkaDatSluzby > 18) 
    {

      temp_sensor_xiaomi.set_value(*(int16_t *)(data + 10) / 100.0);
      // Za teplotou nasleduje 16bitove cele cislo bez znamenka s rel. vlhkosti v procentech a opet znasobene 100
    }
    Serial.println("BLE data received");
}
