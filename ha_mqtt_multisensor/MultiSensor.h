#ifndef MultiSensor_h
#define MultiSensor_h

#include "Arduino.h"
#include "config.h"

#define NO_SENSOR_EVT                 0
#if defined(DOOR_SENSOR)
#define DOOR_SENSOR_EVT               1
#endif
#if defined(PIR_SENSOR)
#define PIR_SENSOR_EVT                2
#endif
#if defined(LDR_SENSOR)
#define LDR_SENSOR_EVT                3
#endif
#if defined(DHT_SENSOR)
#define DHT_TEMPERATURE_SENSOR_EVT    4
#define DHT_HUMIDITY_SENSOR_EVT       5
#endif
#if defined(SHT_SENSOR)
#define SHT_TEMPERATURE_SENSOR_EVT    6
#define SHT_HUMIDITY_SENSOR_EVT       7
#endif
#if defined(BUTTON_SENSOR)
#define BUTTON_SENSOR_EVT             8
#endif

class MultiSensor {
  public:
    MultiSensor(void);
    void init(void);
    void loop(void);
    void setCallback(void (*callback)(uint8_t));
    
#if defined(DOOR_SENSOR)
    bool getDoorState(void);
#endif
#if defined(PIR_SENSOR)
    bool getPirState(void);
#endif
#if defined(LDR_SENSOR)
    uint16_t getLux(void);
#endif
#if defined(DHT_SENSOR)
    float getDHTTemperature(void);
    float getDHTHumidity(void);
#endif
#if defined(SHT_SENSOR)
    float getSHTTemperature(void);
    float getSHTHumidity(void);
#endif
#if defined(BUTTON_SENSOR)
    bool getButtonState(void);
#endif
  private:
    void (*_callback)(uint8_t);
    void handleEvt(void);
#if defined(DOOR_SENSOR)
    bool _readDoorState(void);
    bool _doorState = false;
#endif
#if defined(PIR_SENSOR)
    bool _pirState = false;
#endif
#if defined(LDR_SENSOR)
    uint16_t _ldrValue = 0;
#endif
#if defined(DHT_SENSOR)
    float _readDHTTemperature(void);
    float _readDHTHumidity(void);
    float _DHTTemperature = 0;
    float _DHTHumidity = 0;
#endif
#if defined(SHT_SENSOR)
    float _readSHTTemperature(void);
    float _readSHTHumidity(void);
    float _SHTTemperature = 0;
    float _SHTHumidity = 0;
#endif
#if defined(BUTTON_SENSOR)
    bool _buttonState = false;
#endif
};
#endif
