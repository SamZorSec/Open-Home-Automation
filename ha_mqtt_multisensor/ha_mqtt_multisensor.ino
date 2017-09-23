#include <ESP8266WiFi.h>
#include "MultiSensor.h"

MultiSensor ms;

void onMultiSensorEvent(uint8_t p_evt) {
  Serial.print("onMultiSensorEvent(): evt: ");
  Serial.println(p_evt);
  switch (p_evt) {
#if defined(DOOR_SENSOR)
    case DOOR_SENSOR_EVT:
      Serial.println(ms.getDoorState());
      break;
#endif
#if defined(PIR_SENSOR)
    case PIR_SENSOR_EVT:
      Serial.println(ms.getPirState());
      break;
#endif
#if defined(LDR_SENSOR)
    case LDR_SENSOR_EVT:
      Serial.println(ms.getLdrValue());
      break;
#endif
#if defined(DHT22_SENSOR)
    case DHT22_TEMPERATURE_SENSOR_EVT:
      Serial.println(ms.getTemperature());
      break;
    case DHT22_HUMIDITY_SENSOR_EVT:
      Serial.println(ms.getHumidity());
      break;
#endif
#if defined(BUTTON_SENSOR)
    case BUTTON_SENSOR_EVT:
      Serial.println(ms.getButtonState());
      break;
#endif
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("setup()");

  ms.init();
  ms.setCallback(onMultiSensorEvent);
}

void loop() {
  ms.loop();
}
