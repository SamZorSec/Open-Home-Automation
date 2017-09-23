#include "Arduino.h"
#include "MultiSensor.h"

#if defined(DHT22_SENSOR)
#include "DHT.h"
#define DHTTYPE DHT22
DHT dht(DHT22_SENSOR, DHTTYPE);
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
  this->_doorState = digitalRead(DOOR_SENSOR);
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
#if defined(DHT22_SENSOR)
  dht.begin();
  this->_temperature = dht.readTemperature();
  this->_humidity = dht.readHumidity();
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
      if (digitalRead(DOOR_SENSOR) != this->_doorState) {
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
#if defined(DHT22_SENSOR)
    case DHT22_TEMPERATURE_SENSOR_EVT:
      this->_callback(DHT22_TEMPERATURE_SENSOR_EVT);
      evt = NO_SENSOR_EVT;
      break;
    case DHT22_HUMIDITY_SENSOR_EVT:
      this->_callback(DHT22_HUMIDITY_SENSOR_EVT);
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
    if (currentLdrValue <= this->_ldrValue - LDR_OFFSET_VALUE || currentLdrValue > this->_ldrValue + LDR_OFFSET_VALUE) {
      this->_ldrValue = currentLdrValue;
      evt = LDR_SENSOR_EVT;
      return;
    }
  }
#endif

#if defined(DHT22_SENSOR)
  static unsigned long lastDht22TemperatureSensorMeasure = 0;
  if (lastDht22TemperatureSensorMeasure + DHT22_MEASURE_INTERVAL <= millis()) {
    lastDht22TemperatureSensorMeasure = millis();
    float currentDht22Temperature = dht.readTemperature();
    if (currentDht22Temperature <= this->_temperature - DHT22_TEMPERATURE_OFFSET_VALUE || currentDht22Temperature > this->_temperature + DHT22_TEMPERATURE_OFFSET_VALUE) {
      this->_temperature = currentDht22Temperature;
      evt = DHT22_TEMPERATURE_SENSOR_EVT;
      return;
    }
  }
  
  static unsigned long lastDht22HumiditySensorMeasure = 0;
  if (lastDht22HumiditySensorMeasure + DHT22_MEASURE_INTERVAL <= millis()) {
    lastDht22HumiditySensorMeasure = millis();
    float currentDht22Humidity = dht.readHumidity();
    if (currentDht22Humidity <= this->_humidity - DHT22_HUMIDITY_OFFSET_VALUE || currentDht22Humidity > this->_humidity + DHT22_HUMIDITY_OFFSET_VALUE) {
      this->_humidity = currentDht22Humidity;
      evt = DHT22_HUMIDITY_SENSOR_EVT;
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
uint16_t MultiSensor::getLdrValue(void) {
  return this->_ldrValue;
}
#endif

#if defined(DHT22_SENSOR)
float MultiSensor::getTemperature(void) {
  return this->_temperature;
}

float MultiSensor::getHumidity(void) {
  return this->_humidity;
}
#endif

#if defined(BUTTON_SENSOR)
bool MultiSensor::getButtonState(void) {
  return true;
}
#endif
