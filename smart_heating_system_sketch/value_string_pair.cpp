/**
 * @file value_string_pair.cpp
 * @author Adeel Zahid (adeel.m.zahid@gmail.com)
 * @brief implementation file for smart heating
 * @version 1.0
 * @date 2023-11-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "value_string_pair.h"

void value_string_pair::update_string_from_value()
{
    std::sprintf(string, "%.1f", value);
}

value_string_pair::value_string_pair(float initial_value) 
{
    set_value(initial_value);
}

value_string_pair::value_string_pair() 
{
    set_value(0);
}

// Getter for the 'value' member
float value_string_pair::get_value() const 
{
    return value;
}

// Setter for the 'value' member
void value_string_pair::set_value(float new_value) 
{
    value = new_value;
    update_string_from_value();
}

// Getter for the 'string' member
const char* value_string_pair::get_string() const 
{
    return string;
}

// Setter for the 'string' member
void value_string_pair::set_string(const char* new_string) 
{
    std::strncpy(string, new_string, sizeof(string));
    string[sizeof(string) - 1] = '\0'; // Ensure null-termination
}

//function to convert value to anolge value percentage 0-255
char* value_string_pair::get_value_percentage_string()
{
    int value_int = int(value);
    switch (value_int) 
    {
      case 0:
        strcpy(percentage_string, "0 %");
        break;
      case 7:
        strcpy(percentage_string, "5 %");
        break;
      case 25:
        strcpy(percentage_string, "10 %");
        break;
      case 50:
        strcpy(percentage_string, "20 %");
        break;
      case 100:
        strcpy(percentage_string, "40 %");
        break;
      case 153:
        strcpy(percentage_string, "60 %");
        break;
      default:
        break;
    }
}