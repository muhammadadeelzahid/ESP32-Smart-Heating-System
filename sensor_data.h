/**
 * @file sensor_data.h
 * @author Patrik Stransky
 * @author Adeel Zahid (adeel.m.zahid@gmail.com)
 * @brief include file for for ESP32 WIFI SCMI XIAOMI NodeRED project
 * @version 1.0
 * @date 2023-11-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <cstring> // for strcpy
#include <cstdio>  // for sprintf
#include <iostream>
using namespace std;

#define             EEPROM_SIZE         1
#define             ONE_WIRE_BUS        4
#define             MQTT_VERSION        MQTT_VERSION_3_1

/**
 * @brief class to store value of sensor variable as well as corresponding string and easily convert between them
 * 
 */
class sensor_data 
{
    private:
        float value;
        char string[10];
        char percentage_string[10];

        // Function to update the 'string' member based on the 'value' member
        void update_string_from_value();
    public:
        // Constructor
        sensor_data(float initial_value);

        sensor_data();

        // Getter for the 'value' member
        float get_value() const;

        // Setter for the 'value' member
        void set_value(float new_value);

        //function to convert value to anolge value percentage 0-255
        char* get_value_percentage_string();

        // Getter for the 'string' member
        const char* get_string() const;
        // Setter for the 'string' member
        void set_string(const char* new_string);
};