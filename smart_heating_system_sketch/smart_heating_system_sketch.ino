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
#include "value_string_pair.h"

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
const char *topic_vykon = "esp32/vykon";                    // topic for real_power power_value
const char *topic_skutecnyVykon = "esp32/skutecnyVykon";
const char *topic_stav = "esp32/stav";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature senzor(&oneWire);

/**
 * @brief global objects and variables
 * 
 */
value_string_pair temp_sensor_one;

value_string_pair temp_sensor_second;

value_string_pair temp_sensor_xiaomi(24);

value_string_pair heating_system_power;

long timer1 = 0;
long timer2 = 0;
long timer3 = 0;
char status_string[8];

int loop_count = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// Ukazatel na tridu BLE skeneru
BLEScan *sken;

// Vlastni odvolzena trida pro zpracovani advertizacni zpravy
class NovaAdvertizacniData : public BLEAdvertisedDeviceCallbacks 
{

  // Privatni metoda tridy pro nalezeni dat sluzby v celkove zprave
  uint8_t *najdiData(uint8_t *data, size_t length, uint8_t *delka) 
  {
    uint8_t *pravyOkraj = data + length;
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
    }
    
    return nullptr;
  }

  // Zdedena metoda, ktera se zpracuje pri obdrzeni zpravy
  void onResult(BLEAdvertisedDevice zarizeni) 
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
  }
};

void setup() 
{

  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  pinMode(13, OUTPUT);
  senzor.begin();

  ledcSetup(0, 1000, 8);
  ledcAttachPin(13, 0);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the Wi-Fi network");
  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) 
  {
    String client_id = "esp32";
    client_id += String(random(50000));
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) 
    {
      Serial.println("Public EMQX MQTT broker connected");
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
    }
  }

  client.subscribe(topic_vykon);

  heating_system_power.set_value(EEPROM.read(0));         // load last value from EEPROM
  ledcWrite(0, heating_system_power.get_value());

  BLEDevice::init("");
  sken = BLEDevice::getScan();
  sken->setAdvertisedDeviceCallbacks(new NovaAdvertizacniData(), true);
  sken->setInterval(625);
  sken->setWindow(625);
  sken->setActiveScan(true);
}

void callback(char *topic_vykon, byte *payload, unsigned int length) 
{

  String message = "";

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

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    delay(1000);
    String client_id = "esp32";
    client_id += String(random(50000));
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) 
    {
      Serial.println("Public EMQX MQTT broker connected");
      client.subscribe(topic_vykon);
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.println(client.state());
    }
  }
  Serial.println("client connection failed: reconnect()");
}

void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }

  client.loop();

  senzor.requestTemperatures();
  temp_sensor_one.set_value(senzor.getTempCByIndex(0));
  temp_sensor_second.set_value(senzor.getTempCByIndex(1));

  if ((millis() - timer2) > 3000) 
  {

    client.publish(topic, temp_sensor_one.get_string());
    delay(10);
    client.publish(topic2, temp_sensor_second.get_string());
    delay(10);
    client.publish(topic3, temp_sensor_xiaomi.get_string());
    delay(10);

    timer2 = millis();
  }

  if ((millis() - timer3) > 5000) 
  {        
    
    client.publish(topic_skutecnyVykon, heating_system_power.get_value_percentage_string());

    timer3 = millis();

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

    client.publish(topic_stav, status_string);
  }

  if (temp_sensor_second.get_value() > 43) 
  {
    heating_system_power.set_value(0);
    ledcWrite(0, heating_system_power.get_value());
    EEPROM.write(0, heating_system_power.get_value());
    EEPROM.commit();

    strcpy(status_string, "HAVARIE");
    client.publish(topic_stav, status_string);

    while (1) 
    {
      // INFINITE LOOP
    }
  }

  if ((millis() - timer1) > 60000) 
  {
    // client.disconnect();
    BLEScanResults foundDevices = sken->start(5, false);
    sken->stop();
    sken->clearResults();
    timer1 = millis();
  }

  Serial.print("Loop: ");
  loop_count++;
  Serial.println(loop_count);
}
