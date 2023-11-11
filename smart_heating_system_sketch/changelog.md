### changed

## before

# 1.0
void setup() 
{

  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  pinMode(13, OUTPUT);
  senzor.begin();

  ledcSetup(0, 1000, 8);
  ledcAttachPin(13, 0);

  ///< connect to wifi
  setup_mqtt();

  heating_system_power.set_value(EEPROM.read(0));         // load last value from EEPROM
  ledcWrite(0, heating_system_power.get_value());

  //ble init
  ble_init();
}

## after

  heating_system_power.set_value(EEPROM.read(0));         // load last value from EEPROM
  ledcWrite(0, heating_system_power.get_value());
this portion moves above setup_mqtt() and packaged in a function call