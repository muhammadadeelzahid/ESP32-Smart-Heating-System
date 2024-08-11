# Smart Heating System for ESP32 with Xiaomi BLE and Node-RED Integration

This repository contains the code for a smart floor heating system based on ESP32, using Xiaomi BLE sensors and integrated with Node-RED. The system monitors temperature data from BLE sensors and controls heating based on the data received. It also features MQTT for communication with Node-RED and handles various tasks such as BLE scanning, temperature monitoring, and MQTT publishing.

## Directory structure

Here’s a directory structure diagram for the project:

```python
Smart-Heating-System/
├── README.md
├── LICENSE
├── ble_receive.h
├── ble_receive.cpp
├── sensor_data.h
├── sensor_data.cpp
└── smart_floor_heating_system.ino
```
This structure represents the core files of the project, where:

* README.md: Provides an overview and usage instructions for the project.
* ble_receive.h: Header file for the BLE data receiving class.
* ble_receive.cpp: Implementation file for the BLE data receiving class.
* sensor_data.h: Header file for managing sensor data.
* sensor_data.cpp: Implementation file for managing sensor data.
* smart_floor_heating_system.ino: Main implementation file for the smart heating system, including setup and control logic.

## System Overview

This system is designed to work seamlessly with Node-RED over MQTT for both analytics on sensor data and control of the heating system. The BLE sensors gather temperature data, which is communicated to the main ESP32. This data is then published to specific MQTT topics that can be accessed and monitored via Node-RED, which also hosts the MQTT broker. Another ESP32 device is used to manage the heating system, subscribing to control commands via MQTT.