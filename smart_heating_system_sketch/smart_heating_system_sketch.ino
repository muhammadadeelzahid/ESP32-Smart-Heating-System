/**
 * @file smart_heating_system.ino
 * @author Patrik Stransky (@gmail.com)
 * @author Adeel Zahid (adeel.m.zahid@gmail.com)
 * @brief main implementation file for smart heating system based on ESP32 WIFI SCMI XIAOMI NodeRED
 * @version 1.0
 * @date 2023-11-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include <BLEDevice.h>                                        // Built-in BLE device library
#include <BLEUtils.h>                                         // Support library for BLE
#include <BLEScan.h>                                          // Library for BLE scan of nearby devices
#include <BLEAdvertisedDevice.h>                              // Library for BLE advertising messages
#include "sensor_data.h"
#include "ble_receive.h"
#include <rom/rtc.h>

/**
 * @brief wifi credentials, set password to "" for open networks
 * 
 */
const char *ssid = "FreeWifi";
const char *password = "banannaopici";
const char *mqtt_broker = "192.168.88.88";
const char *topic = "esp32/temp1";
const char *topic2 = "esp32/temp2";
const char *topic3 = "esp32/temp3";
const char *topic_vykon = "esp32/vykon";                      // topic for real_power power_value
const char *topic_skutecnyVykon = "esp32/skutecnyVykon";
const char *topic_reset_reason = "esp32/reset_reason";        // topic for publish of reset reason  - publish reset reason
const char *topic_reset_flag = "esp32/vykon";                 // topic for reset flag - receive reset command
const char *topic_stav = "esp32/stav";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature senzor(&oneWire);


/**
 * @brief task functions prototypes
 * 
 */
void mqtt_connection_monitoring_task(void *pv_params);

void mqtt_publish_task(void *pv_params);

void temperature_monitor_task(void *pv_params);

void real_power_publish_task(void *pv_params);

void ble_receive_task(void *pv_params);

/**
 * @brief global objects and variables
 * 
 */
sensor_data temp_sensor_one;

sensor_data temp_sensor_two;

sensor_data temp_sensor_xiaomi(24);

sensor_data heating_system_power;

sensor_data cpu0_reset_reason;

sensor_data cpu1_reset_reason;

char reset_reason_concatenated[20];
char status_string[8];

WiFiClient espClient;
PubSubClient client(espClient);

//Pointer to BLE scanner class
BLEScan *ble_scanner;

void setup_mqtt()
{

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the Wi-Fi network");

  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(mqtt_subscribe_callback);

  //duplicate code was found here with reconnect() function
  mqtt_connect();
}

void ble_init()
{
  BLEDevice::init("");
  ble_scanner = BLEDevice::getScan();
  ble_scanner->setAdvertisedDeviceCallbacks(new ble_receive(), true);
  ble_scanner->setInterval(625);
  ble_scanner->setWindow(625);
  ble_scanner->setActiveScan(true);
}

void heating_unit_init()
{
  EEPROM.begin(EEPROM_SIZE);

  pinMode(13, OUTPUT);
  senzor.begin();

  ledcSetup(0, 1000, 8);
  ledcAttachPin(13, 0);

  heating_system_power.set_value(EEPROM.read(0));         // load last value from EEPROM
  ledcWrite(0, heating_system_power.get_value());
}

void print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1 : Serial.println ("POWERON_RESET"); break;          /**<1, Vbat power on reset*/
    case 3 : Serial.println ("SW_RESET"); break;               /**<3, Software reset digital core*/
    case 4 : Serial.println ("OWDT_RESET"); break;             /**<4, Legacy watch dog reset digital core*/
    case 5 : Serial.println ("DEEPSLEEP_RESET"); break;        /**<5, Deep Sleep reset digital core*/
    case 6 : Serial.println ("SDIO_RESET"); break;             /**<6, Reset by SLC module, reset digital core*/
    case 7 : Serial.println ("TG0WDT_SYS_RESET"); break;       /**<7, Timer Group0 Watch dog reset digital core*/
    case 8 : Serial.println ("TG1WDT_SYS_RESET"); break;       /**<8, Timer Group1 Watch dog reset digital core*/
    case 9 : Serial.println ("RTCWDT_SYS_RESET"); break;       /**<9, RTC Watch dog Reset digital core*/
    case 10 : Serial.println ("INTRUSION_RESET"); break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : Serial.println ("TGWDT_CPU_RESET"); break;       /**<11, Time Group reset CPU*/
    case 12 : Serial.println ("SW_CPU_RESET"); break;          /**<12, Software reset CPU*/
    case 13 : Serial.println ("RTCWDT_CPU_RESET"); break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : Serial.println ("EXT_CPU_RESET"); break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : Serial.println ("RTCWDT_BROWN_OUT_RESET"); break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : Serial.println ("RTCWDT_RTC_RESET"); break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : Serial.println ("NO_MEAN");
  }
}

void publish_reset_reason()
{

  Serial.println("CPU0 reset reason: ");
  cpu0_reset_reason.set_value(rtc_get_reset_reason(0));
  print_reset_reason(rtc_get_reset_reason(0));

  Serial.println("CPU1 reset reason: ");
  cpu1_reset_reason.set_value(rtc_get_reset_reason(1));
  print_reset_reason(rtc_get_reset_reason(1));

  char* empty_space = {" "};
  strcat(reset_reason_concatenated,cpu0_reset_reason.get_string());
  strcat(reset_reason_concatenated,empty_space);
  strcat(reset_reason_concatenated,cpu1_reset_reason.get_string());

  if (!client.connected())
  {
    mqtt_connect();
  }
  client.publish(topic_reset_reason, reset_reason_concatenated);
}

void setup() 
{

  Serial.begin(115200);

  heating_unit_init();

  ///< connect to wifi
  setup_mqtt();

  //ble init
  ble_init();

  publish_reset_reason();

  //create tasks
  if (xTaskCreate(mqtt_connection_monitoring_task, "mqtt_monitoring_task",1024, NULL,4,NULL) != pdPASS)
  {
    Serial.println("Task creation failure: mqtt_monitoring_task");
  }

  if (xTaskCreate(mqtt_publish_task, "mqtt_publish_task",1024, NULL,4,NULL) != pdPASS)
  {
    Serial.println("Task creation failure: mqtt_publish_task");
  }

  if (xTaskCreate(temperature_monitor_task, "temp_monitoring_task",1024, NULL,4,NULL) != pdPASS)
  {
    Serial.println("Task creation failure: temperature_monitor_task");
  }

  if (xTaskCreate(real_power_publish_task, "power_publish_task",1024, NULL,4,NULL) != pdPASS)
  {
    Serial.println("Task creation failure: real_power_publish_task");
  }

  if (xTaskCreate(ble_receive_task, "ble_receive_task",1024, NULL,4,NULL) != pdPASS)
  {
    Serial.println("Task creation failure: ble_receive_task");
  }

}
void loop() 
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void mqtt_subscribe_callback(char *topic_name, byte *payload, unsigned int length) 
{

  String message = "";

  if (strcmp(topic_name,topic_vykon) == 0)
  {
    Serial.print("POWER:");

    for (int i = 0; i < length; i++) 
    {
      // Serial.print((char) payload[i]);
      message += (char)payload[i];
    }

    heating_system_power.set_value(message.toInt());
    ledcWrite(0, heating_system_power.get_value());

    EEPROM.write(0, heating_system_power.get_value());
    EEPROM.commit();

    Serial.println(heating_system_power.get_value());
    Serial.println("-----------------------");

  }
  else if (strcmp(topic_name,topic_reset_flag) == 0)
  {
    // Handle the reset flag
    message = "";
    for (int i = 0; i < length; i++)
    {
      message += (char)payload[i];
    }

    // Check if the payload is "1" to trigger the reset
    if (message == "1")
    {
      Serial.println("Resetting ESP32...");
      esp_restart();
    }
    else
    {
      Serial.println("Error: mqtt_subscribe_callback - Invalid payload for topic reset_flag"); 
    }  
  }
  else
  {
     Serial.println("Invalid topic data");
  }
}

void mqtt_connect() 
{

  if (!client.connected()) 
  {
    String client_id = "esp32";
    client_id += String(random(50000));
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) 
    {
      Serial.println("Public EMQX MQTT broker connected");
      client.subscribe(topic_vykon);
      client.subscribe(topic_reset_flag);
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.println(client.state());
    }
    Serial.println("Error: mqtt_connect - client connection failed");
  }
}


void mqtt_connection_monitoring_task(void *pv_params)
{

  Serial.println("Task started: mqtt_connection_monitoring_task");

  while(1)
  {    
    if (!client.connected()) 
    {
      mqtt_connect();
    }
    client.loop();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void mqtt_publish_task(void *pv_params)
{
  Serial.println("Task started: mqtt_publish_task");

  while(1)
  {
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    if (client.connected())
    {
      client.publish(topic, temp_sensor_one.get_string());
      delay(10);
      client.publish(topic2, temp_sensor_two.get_string());
      delay(10);
      client.publish(topic3, temp_sensor_xiaomi.get_string());
      delay(10);
    }
    else
    {
      Serial.println("Error: mqtt_publish_task - client disconnected");
    }

  }

}

void temperature_monitor_task(void *pv_params)
{
  Serial.println("Task started: temperature_monitor_task");

  while(1)
  {
    //task 2 get temperatures and error handling below
    senzor.requestTemperatures();
    temp_sensor_one.set_value(senzor.getTempCByIndex(0));
    temp_sensor_two.set_value(senzor.getTempCByIndex(1));

    if (temp_sensor_two.get_value() > 43) 
    {
      heating_system_power.set_value(0);
      ledcWrite(0, heating_system_power.get_value());
      EEPROM.write(0, heating_system_power.get_value());
      EEPROM.commit();

      strcpy(status_string, "HAVARIE");

      if (client.connected())
      {
        client.publish(topic_stav, status_string);
      }
      else
      {
        Serial.println("Error: temperature_monitor_task - mqtt connection failed");
      }

      //pause this task for 3 seconds
      vTaskDelay(pdMS_TO_TICKS(3000));
    }

    vTaskDelay(pdMS_TO_TICKS(100));    
  }


}

void real_power_publish_task(void *pv_params)
{
  Serial.println("Task started: real_power_publish_task");

  while(1)
  {
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    if (client.connected())
    {
      client.publish(topic_skutecnyVykon, heating_system_power.get_value_percentage_string());      
    }
    else
    {
      Serial.println("Error: real_power_publish_task - mqtt connection failed");
    }


    if (heating_system_power.get_value() > 0) 
    {
      strcpy(status_string, "TOPIM");
      // status_string="TOPIM";
    }

    if (heating_system_power.get_value() == 0) 
    {
      strcpy(status_string, "NETOPIM");
      // status_string="NETOPIM";
    }

    if (!client.connected())
    {
      client.publish(topic_stav, status_string);
    }
    else
    {
      Serial.println("Error: real_power_publish_task - mqtt connection failed");
    }
  
  }

}

void ble_receive_task(void *pv_params)
{
  Serial.println("Task started: ble_receive_task");

  while(1)
  {
    vTaskDelay(pdMS_TO_TICKS(60000));
  
    BLEScanResults foundDevices = ble_scanner->start(5, false);
    ble_scanner->stop();
    ble_scanner->clearResults();
  }  
}