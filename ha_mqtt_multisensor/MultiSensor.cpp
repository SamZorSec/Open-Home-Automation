#include "Arduino.h"
#include "MultiSensor.h"

#if defined(DHT_SENSOR)
// https://github.com/adafruit/Adafruit_Sensor
// https://github.com/adafruit/DHT-sensor-library
#include "DHT.h"
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);
#endif

#if defined(SHT_SENSOR)
// https://github.com/Sensirion/arduino-sht
#include <Wire.h>
#include "SHTSensor.h"
SHTSensor sht;
#endif

volatile uint8_t evt = NO_SENSOR_EVT;    

///////////////////////////////////////////////////////////////////////////
//  ISRs
///////////////////////////////////////////////////////////////////////////

#if defined(DOOR_SENSOR)
void doorSensorISR(void) {
  evt = DOOR_SENSOR_EVT;
}
#endif

#if defined(PIR_SENSOR)
void pirSensorISR(void) {
  evt = PIR_SENSOR_EVT;
}
#endif

#if defined(BUTTON_SENSOR)
void buttonSensorISR(void) {
  static unsigned long lastButtonSensorInterrupt = 0;
  unsigned long currentButtonSensorInterrupt = millis();
  if (currentButtonSensorInterrupt - lastButtonSensorInterrupt > 500)
    evt = BUTTON_SENSOR_EVT;
  lastButtonSensorInterrupt = currentButtonSensorInterrupt;
}
#endif


///////////////////////////////////////////////////////////////////////////
//  Constructor, init(), handleEvt() & loop()
///////////////////////////////////////////////////////////////////////////
MultiSensor::MultiSensor(void) {}

void MultiSensor::init(void) {
#if defined(DOOR_SENSOR)
  pinMode(DOOR_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(DOOR_SENSOR), doorSensorISR, CHANGE);
  this->_doorState = this->_readDoorState();
#endif
#if defined(PIR_SENSOR)
  pinMode(PIR_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIR_SENSOR), pirSensorISR, CHANGE);
  this->_pirState = digitalRead(PIR_SENSOR);
#endif
#if defined(LDR_SENSOR)
  pinMode(LDR_SENSOR, INPUT);
  this->_ldrValue = analogRead(LDR_SENSOR);
#endif
#if defined(DHT_SENSOR)
  dht.begin();
  delay(2000);
  this->_readDHTTemperature();
  this->_readDHTHumidity();
#endif
#if defined(SHT_SENSOR)
  Wire.begin(SHT_SCL_PIN, SHT_SDA_PIN);
  sht.init();
  sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM); 
  this->_readSHTTemperature();
  this->_readSHTHumidity();
#endif
#if defined(BUTTON_SENSOR)
  pinMode(BUTTON_SENSOR, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_SENSOR), buttonSensorISR, CHANGE);
#endif
}

void MultiSensor::handleEvt(void) {
  switch(evt) {
    case NO_SENSOR_EVT:
      break;
#if defined(DOOR_SENSOR)
    case DOOR_SENSOR_EVT:
      if (this->_readDoorState() != this->_doorState) {
        this->_doorState = !this->_doorState;
        this->_callback(DOOR_SENSOR_EVT);
      }
      evt = NO_SENSOR_EVT;
      break;
#endif
#if defined(PIR_SENSOR)
    case PIR_SENSOR_EVT:
      if (digitalRead(PIR_SENSOR) != this->_pirState) {
        this->_pirState = !this->_pirState;
        this->_callback(PIR_SENSOR_EVT);
      }
      evt = NO_SENSOR_EVT;
      break;
#endif
#if defined(BUTTON_SENSOR)
    case BUTTON_SENSOR_EVT:
      this->_buttonState = !this->_buttonState;
      this->_callback(BUTTON_SENSOR_EVT);
      evt = NO_SENSOR_EVT;
      break;
#endif
#if defined(LDR_SENSOR)
    case LDR_SENSOR_EVT:
      this->_callback(LDR_SENSOR_EVT);
      evt = NO_SENSOR_EVT;
      break;
#endif
#if defined(DHT_SENSOR)
    case DHT_TEMPERATURE_SENSOR_EVT:
      this->_callback(DHT_TEMPERATURE_SENSOR_EVT);
      evt = NO_SENSOR_EVT;
      break;
    case DHT_HUMIDITY_SENSOR_EVT:
      this->_callback(DHT_HUMIDITY_SENSOR_EVT);
      evt = NO_SENSOR_EVT;
      break;
#endif
#if defined(SHT_SENSOR)
    case SHT_TEMPERATURE_SENSOR_EVT:
      this->_callback(SHT_TEMPERATURE_SENSOR_EVT);
      evt = NO_SENSOR_EVT;
      break;
    case SHT_HUMIDITY_SENSOR_EVT:
      this->_callback(SHT_HUMIDITY_SENSOR_EVT);
      evt = NO_SENSOR_EVT;
      break;
#endif
  }
}

void MultiSensor::loop(void) {
  this->handleEvt();
  
#if defined(LDR_SENSOR)
  static unsigned long lastLdrSensorMeasure = 0;
  if (lastLdrSensorMeasure + LDR_MEASURE_INTERVAL <= millis()) {
    lastLdrSensorMeasure = millis();
    uint16_t currentLdrValue = analogRead(LDR_SENSOR);
    if (currentLdrValue <= this->_ldrValue - LDR_OFFSET_VALUE || currentLdrValue >= this->_ldrValue + LDR_OFFSET_VALUE) {
      this->_ldrValue = currentLdrValue;
      evt = LDR_SENSOR_EVT;
      return;
    }
  }
#endif

#if defined(DHT_SENSOR)
  static unsigned long lastDHTTemperatureSensorMeasure = 0;
  if (lastDHTTemperatureSensorMeasure + DHT_MEASURE_INTERVAL <= millis()) {
    lastDHTTemperatureSensorMeasure = millis();
    float currentDHTTemperature = this->_readDHTTemperature();
    if (currentDHTTemperature <= this->_DHTTemperature - DHT_TEMPERATURE_OFFSET_VALUE || currentDHTTemperature >= this->_DHTTemperature + DHT_TEMPERATURE_OFFSET_VALUE) {
      this->_DHTTemperature = currentDHTTemperature;
      evt = DHT_TEMPERATURE_SENSOR_EVT;
      return;
    }
  }
  
  static unsigned long lastDHTHumiditySensorMeasure = 0;
  if (lastDHTHumiditySensorMeasure + DHT_MEASURE_INTERVAL <= millis()) {
    lastDHTHumiditySensorMeasure = millis();
    float currentDHTHumidity = this->_readDHTHumidity();
    if (currentDHTHumidity <= this->_DHTHumidity - DHT_HUMIDITY_OFFSET_VALUE || currentDHTHumidity >= this->_DHTHumidity + DHT_HUMIDITY_OFFSET_VALUE) {
      this->_DHTHumidity = currentDHTHumidity;
      evt = DHT_HUMIDITY_SENSOR_EVT;
      return;
    }
  }
#endif
#if defined(SHT_SENSOR)
  static unsigned long lastSHTTemperatureSensorMeasure = 0;
  if (lastSHTTemperatureSensorMeasure + SHT_MEASURE_INTERVAL <= millis()) {
    lastSHTTemperatureSensorMeasure = millis();
    float currentSHTTemperature = this->_readSHTTemperature();
    if (currentSHTTemperature <= this->_SHTTemperature - SHT_TEMPERATURE_OFFSET_VALUE || currentSHTTemperature >= this->_SHTTemperature + SHT_TEMPERATURE_OFFSET_VALUE) {
      this->_SHTTemperature = currentSHTTemperature;
      evt = SHT_TEMPERATURE_SENSOR_EVT;
      return;
    }
  }
  
  static unsigned long lastSHTHumiditySensorMeasure = 0;
  if (lastSHTHumiditySensorMeasure + SHT_MEASURE_INTERVAL <= millis()) {
    lastSHTHumiditySensorMeasure = millis();
    float currentSHTHumidity = this->_readSHTHumidity();
    if (currentSHTHumidity <= this->_SHTHumidity - SHT_HUMIDITY_OFFSET_VALUE || currentSHTHumidity >= this->_SHTHumidity + SHT_HUMIDITY_OFFSET_VALUE) {
      this->_SHTHumidity = currentSHTHumidity;
      evt = SHT_HUMIDITY_SENSOR_EVT;
      return;
    }
  }
#endif
}


///////////////////////////////////////////////////////////////////////////
//  setCallback()
///////////////////////////////////////////////////////////////////////////
void MultiSensor::setCallback(void (*callback)(uint8_t)) {
  this->_callback = callback;
}


///////////////////////////////////////////////////////////////////////////
//  Getters 
///////////////////////////////////////////////////////////////////////////
#if defined(DOOR_SENSOR)
bool MultiSensor::_readDoorState(void) {
  return digitalRead(DOOR_SENSOR);
}

bool MultiSensor::getDoorState(void) {
  return this->_doorState;
}
#endif

#if defined(PIR_SENSOR)
bool MultiSensor::getPirState(void) {
  return this->_pirState;
}
#endif

#if defined(LDR_SENSOR)
uint16_t MultiSensor::getLux(void) {
  // http://forum.arduino.cc/index.php?topic=37555.0
  float voltage = this->_ldrValue * LDR_VOLTAGE_PER_ADC_PRECISION;   
  return 500 / (LDR_RESISTOR_VALUE * ((LDR_REFERENCE_VOLTAGE - voltage) / voltage));
}
#endif


#if defined(DHT_SENSOR)
float MultiSensor::_readDHTTemperature(void) {
  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    return this->_DHTTemperature;
  }
  return temperature;
}

float MultiSensor::_readDHTHumidity(void) {
  float humidity = dht.readHumidity();

  if (isnan(humidity)) {
    return this->_DHTHumidity;
  }
  return humidity;
}

float MultiSensor::getDHTTemperature(void) {
  return this->_DHTTemperature;
}

float MultiSensor::getDHTHumidity(void) {
  return this->_DHTHumidity;
}
#endif


#if defined(SHT_SENSOR)
float MultiSensor::_readSHTTemperature(void) {
  float temperature = sht.getTemperature();
  
  if (isnan(temperature)) {
    return this->_SHTTemperature;
  }
  return temperature;
}

float MultiSensor::_readSHTHumidity(void) {
  float humidity = sht.getHumidity();

  if (isnan(humidity)) {
    return this->_SHTHumidity;
  }
  return humidity;
}

float MultiSensor::getSHTTemperature(void) {
  return this->_SHTTemperature;
}

float MultiSensor::getSHTHumidity(void) {
  return this->_SHTHumidity;
}
#endif


#if defined(BUTTON_SENSOR)
bool MultiSensor::getButtonState(void) {
  return this->_buttonState;
}
#endif
