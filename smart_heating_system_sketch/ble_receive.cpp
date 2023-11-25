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
uint8_t* ble_receive::find_data(uint8_t *data, size_t length, uint8_t *delka)
{
    uint8_t *max_data = data + length;
    Serial.println("service data function");
    while (data < max_data) 
    {
      uint8_t length_block = *data + 1;
      if (length_block < 5) 
      {
        data += length_block;
        continue;
      }
      uint8_t block_type = *(data + 1);
      uint16_t type_of_service = *(uint16_t *)(data + 2);
      //If it is a correct data type and class with UUID 0x181a,
      //return the data
      if (block_type == 0x16) 
      {
        if (type_of_service == 0x181a) 
        {
          *delka = length_block;
          return data;
        }
      }
      data += length_block;
      Serial.print("service data, while iteration\n"); 
    }
    
    return nullptr;
}

/**
 * @brief Inherited method that will be processed when the message is received
 * 
 * @param device 
 */
void ble_receive::onResult(BLEAdvertisedDevice device)
{
    //Get the data
    uint8_t *data = device.getPayload();

    // Get the data length
    size_t data_length = device.getPayloadLength();
    uint8_t service_data_length = 0;
    //Extract the service data containing the data we are looking for from the raw report
    uint8_t *data_services = find_data(data, data_length, &service_data_length);
    if (data == nullptr || service_data_length < 18)
    {
      return;
    }
    //If the service data is 19 bytes long, that's what we're looking for
    if (service_data_length > 18) 
    {

      temp_sensor_xiaomi.set_value(*(int16_t *)(data + 10) / 100.0);
      //The temperature is followed by a 16-bit unsigned integer with rel. humidity in percent and again multiplied by 100
    }
    Serial.println("BLE data received");
}
