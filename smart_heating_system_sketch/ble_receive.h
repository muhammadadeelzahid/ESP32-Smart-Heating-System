/**
 * @file ble_receive.cpp
 * @author Patrik Stransky
 * @author Adeel Zahid (adeel.m.zahid@gmail.com)
 * @brief class definition for ble data receiving for xiaomi sensor
 * @version 1.0
 * @date 2023-11-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <BLEDevice.h>                                        // Built-in BLE device library
#include <BLEUtils.h>                                         // Support library for BLE
#include <BLEScan.h>                                          // Library for BLE scan of nearby devices
#include <BLEAdvertisedDevice.h>                              // Library for BLE advertising messages

/**
 * @brief Own dedicated class for advertising message processing
 * 
 */
class ble_receive : public BLEAdvertisedDeviceCallbacks
{

    public:

    /**
     * @brief Private method of the class to find the service data in the overall report
     * @param data 
     * @param length 
     * @param delka 
     * @return *ptr 
     */
    uint8_t* find_data(uint8_t *data, size_t length, uint8_t *delka);

    /**
     * @brief Inherited method that will be processed when the message is received
     * 
     * @param device 
     */
    void onResult(BLEAdvertisedDevice device);
};
