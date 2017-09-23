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
#if defined(DHT22_SENSOR)
#define DHT22_TEMPERATURE_SENSOR_EVT  4
#define DHT22_HUMIDITY_SENSOR_EVT     5
#endif
#if defined(BUTTON_SENSOR)
#define BUTTON_SENSOR_EVT             6
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
    uint16_t getLdrValue(void);
#endif
#if defined(DHT22_SENSOR)
    float getTemperature(void);
    float getHumidity(void);
#endif
#if defined(BUTTON_SENSOR)
    bool getButtonState(void);
#endif
  private:
    void (*_callback)(uint8_t);
    void handleEvt(void);
#if defined(DOOR_SENSOR)
    bool _doorState = false;
#endif
#if defined(PIR_SENSOR)
    bool _pirState = false;
#endif
#if defined(LDR_SENSOR)
    uint16_t _ldrValue = 0;
#endif
#if defined(DHT22_SENSOR)
    float _temperature = 0;
    float _humidity = 0;
#endif
};
#endif
