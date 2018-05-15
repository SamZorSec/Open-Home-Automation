#include <ESP8266WiFi.h>
#include "MultiSensor.h"
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient
#if defined(OTA)
#include <ArduinoOTA.h>
#endif

#if defined(DEBUG_SERIAL)
#define     DEBUG_PRINT(x)    Serial.print(x)
#define     DEBUG_PRINTLN(x)  Serial.println(x)
#else
#define     DEBUG_PRINT(x)
#define     DEBUG_PRINTLN(x)
#endif

MultiSensor   ms;
WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);


///////////////////////////////////////////////////////////////////////////
//   WiFi
///////////////////////////////////////////////////////////////////////////
/*
   Function called to handle WiFi events
*/
void handleWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
      DEBUG_PRINTLN(F("INFO: Connection successful to the Wi-Fi AP"));
      DEBUG_PRINT(F("INFO: IP address: "));
      DEBUG_PRINTLN(WiFi.localIP());
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      DEBUG_PRINTLN(F("ERROR: Connection lost from the Wi-Fi AP"));
      /*
         TODO: Doing something smarter than rebooting the device
      */
      delay(5000);
      ESP.restart();
      break;
    default:
      DEBUG_PRINT(F("INFO: WiFi event: "));
      DEBUG_PRINTLN(event);
      break;
  }
}

/*
   Function called to setup the connection to the WiFi AP
*/
void setupWiFi() {
  DEBUG_PRINT(F("INFO: WiFi connecting to: "));
  DEBUG_PRINTLN(WIFI_SSID);

  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.onEvent(handleWiFiEvent);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  randomSeed(micros());
}


///////////////////////////////////////////////////////////////////////////
//   OTA
///////////////////////////////////////////////////////////////////////////
#if defined(OTA)
/*
   Function called to setup OTA updates
*/
void setupOTA() {
#if defined(OTA_HOSTNAME)
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  DEBUG_PRINT(F("INFO: OTA hostname sets to: "));
  DEBUG_PRINTLN(OTA_HOSTNAME);
#endif

#if defined(OTA_PORT)
  ArduinoOTA.setPort(OTA_PORT);
  DEBUG_PRINT(F("INFO: OTA port sets to: "));
  DEBUG_PRINTLN(OTA_PORT);
#endif

#if defined(OTA_PASSWORD)
  ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
  DEBUG_PRINT(F("INFO: OTA password sets to: "));
  DEBUG_PRINTLN(OTA_PASSWORD);
#endif

  ArduinoOTA.onStart([]() {
    DEBUG_PRINTLN(F("INFO: OTA starts"));
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN(F("INFO: OTA ends"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINT(F("INFO: OTA progresses: "));
    DEBUG_PRINT(progress / (total / 100));
    DEBUG_PRINTLN(F("%"));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PRINT(F("ERROR: OTA error: "));
    DEBUG_PRINTLN(error);
    if (error == OTA_AUTH_ERROR)
      DEBUG_PRINTLN(F("ERROR: OTA auth failed"));
    else if (error == OTA_BEGIN_ERROR)
      DEBUG_PRINTLN(F("ERROR: OTA begin failed"));
    else if (error == OTA_CONNECT_ERROR)
      DEBUG_PRINTLN(F("ERROR: OTA connect failed"));
    else if (error == OTA_RECEIVE_ERROR)
      DEBUG_PRINTLN(F("ERROR: OTA receive failed"));
    else if (error == OTA_END_ERROR)
      DEBUG_PRINTLN(F("ERROR: OTA end failed"));
  });
  ArduinoOTA.begin();
}

/*
   Function called to handle OTA updates
*/
void handleOTA() {
  ArduinoOTA.handle();
}
#endif



///////////////////////////////////////////////////////////////////////////
//   MQTT
///////////////////////////////////////////////////////////////////////////
volatile unsigned long lastMQTTConnection = 0;
char MQTT_CLIENT_ID[7] = {0};
char MQTT_PAYLOAD[8] = {0};
char MQTT_AVAILABILITY_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_AVAILABILITY_TOPIC_TEMPLATE) - 2] = {0};
#if defined(MOTION_SENSOR)
char MQTT_MOTION_SENSOR_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(MOTION_SENSOR_NAME) - 4] = {0};
#endif
#if defined(DOOR_SENSOR)
char MQTT_DOOR_SENSOR_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(DOOR_SENSOR_NAME) - 4] = {0};
#endif
#if defined(LDR_SENSOR)
char MQTT_LDR_SENSOR_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(LDR_SENSOR_NAME) - 4] = {0};
#endif
#if defined(DHT_SENSOR)
char MQTT_DHT_TEMPERATURE_SENSOR_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(DHT_TEMPERATURE_SENSOR_NAME) - 4] = {0};
char MQTT_DHT_HUMIDITY_SENSOR_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(DHT_HUMIDITY_SENSOR_NAME) - 4] = {0};
#endif
#if defined(SHT_SENSOR)
char MQTT_SHT_TEMPERATURE_SENSOR_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(SHT_TEMPERATURE_SENSOR_NAME) - 4] = {0};
char MQTT_SHT_HUMIDITY_SENSOR_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(SHT_HUMIDITY_SENSOR_NAME) - 4] = {0};
#endif
#if defined(BUTTON_SENSOR)
char MQTT_BUTTON_SENSOR_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(BUTTON_SENSOR_NAME) - 4] = {0};
#endif
/*
  Function called to publish to a MQTT topic with the given payload
*/
void publishToMQTT(char* p_topic, char* p_payload) {
  if (mqttClient.publish(p_topic, p_payload, true)) {
    DEBUG_PRINT(F("INFO: MQTT message published successfully, topic: "));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F(", payload: "));
    DEBUG_PRINTLN(p_payload);
  } else {
    DEBUG_PRINTLN(F("ERROR: MQTT message not published, either connection lost, or message too large. Topic: "));
    DEBUG_PRINT(p_topic);
    DEBUG_PRINT(F(" , payload: "));
    DEBUG_PRINTLN(p_payload);
  }
}

/*
  Function called to connect/reconnect to the MQTT broker
*/
void connectToMQTT() {
  if (!mqttClient.connected()) {
    if (lastMQTTConnection < millis()) {
      if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_AVAILABILITY_TOPIC, 0, 1, "offline")) {
        DEBUG_PRINTLN(F("INFO: The client is successfully connected to the MQTT broker"));
        publishToMQTT(MQTT_AVAILABILITY_TOPIC, "online");

        // publish sensors' states
#if defined(DOOR_SENSOR)
        if (!ms.getDoorState()) {
          publishToMQTT(MQTT_DOOR_SENSOR_TOPIC, MQTT_PAYLOAD_ON);
        } else {
          publishToMQTT(MQTT_DOOR_SENSOR_TOPIC, MQTT_PAYLOAD_OFF);
        }
#endif
#if defined(MOTION_SENSOR)
        if (ms.getMotionState()) {
          publishToMQTT(MQTT_MOTION_SENSOR_TOPIC, MQTT_PAYLOAD_ON);
        } else {
          publishToMQTT(MQTT_MOTION_SENSOR_TOPIC, MQTT_PAYLOAD_OFF);
        }
#endif
#if defined(LDR_SENSOR)
        itoa (ms.getLux(), MQTT_PAYLOAD, 10);
        publishToMQTT(MQTT_LDR_SENSOR_TOPIC, MQTT_PAYLOAD);
#endif
#if defined(DHT_SENSOR)
        dtostrf(ms.getDHTTemperature(), 2, 2, MQTT_PAYLOAD);
        publishToMQTT(MQTT_DHT_TEMPERATURE_SENSOR_TOPIC, MQTT_PAYLOAD);
        dtostrf(ms.getDHTHumidity(), 2, 2, MQTT_PAYLOAD);
        publishToMQTT(MQTT_DHT_HUMIDITY_SENSOR_TOPIC, MQTT_PAYLOAD);
#endif
#if defined(SHT_SENSOR)
        dtostrf(ms.getSHTTemperature(), 2, 2, MQTT_PAYLOAD);
        publishToMQTT(MQTT_SHT_TEMPERATURE_SENSOR_TOPIC, MQTT_PAYLOAD);
        dtostrf(ms.getSHTHumidity(), 2, 2, MQTT_PAYLOAD);
        publishToMQTT(MQTT_SHT_HUMIDITY_SENSOR_TOPIC, MQTT_PAYLOAD);
#endif
#if defined(BUTTON_SENSOR)
        if (ms.getButtonState()) {
          publishToMQTT(MQTT_BUTTON_SENSOR_TOPIC, MQTT_PAYLOAD_ON);
        } else {
          publishToMQTT(MQTT_BUTTON_SENSOR_TOPIC, MQTT_PAYLOAD_OFF);
        }
#endif
      } else {
        DEBUG_PRINTLN(F("ERROR: The connection to the MQTT broker failed"));
        DEBUG_PRINT(F("INFO: MQTT username: "));
        DEBUG_PRINTLN(MQTT_USERNAME);
        DEBUG_PRINT(F("INFO: MQTT password: "));
        DEBUG_PRINTLN(MQTT_PASSWORD);
        DEBUG_PRINT(F("INFO: MQTT broker: "));
        DEBUG_PRINTLN(MQTT_SERVER);
      }
      lastMQTTConnection = millis() + MQTT_CONNECTION_TIMEOUT;
    }
  }
}

void onMultiSensorEvent(uint8_t p_evt) {
  DEBUG_PRINT(F("INFO: onMultiSensorEvent(): evt: "));
  DEBUG_PRINTLN(p_evt);
  switch (p_evt) {
#if defined(DOOR_SENSOR)
    case DOOR_SENSOR_EVT:
      if (ms.getDoorState()) {
        // ON:  means that the door is opened (getDoorState() == 0)
        // OFF: means that the door is closed
        publishToMQTT(MQTT_DOOR_SENSOR_TOPIC, MQTT_PAYLOAD_OFF);
      } else {
        publishToMQTT(MQTT_DOOR_SENSOR_TOPIC, MQTT_PAYLOAD_ON);
      }
      break;
#endif
#if defined(MOTION_SENSOR)
    case MOTION_SENSOR_EVT:
      if (ms.getMotionState()) {
        publishToMQTT(MQTT_MOTION_SENSOR_TOPIC, MQTT_PAYLOAD_ON);
      } else {
        publishToMQTT(MQTT_MOTION_SENSOR_TOPIC, MQTT_PAYLOAD_OFF);
      }
      break;
#endif
#if defined(LDR_SENSOR)
    case LDR_SENSOR_EVT:
      itoa (ms.getLux(), MQTT_PAYLOAD, 10);
      publishToMQTT(MQTT_LDR_SENSOR_TOPIC, MQTT_PAYLOAD);
      break;
#endif
#if defined(DHT_SENSOR)
    case DHT_TEMPERATURE_SENSOR_EVT:
      dtostrf(ms.getDHTTemperature(), 2, 2, MQTT_PAYLOAD);
      publishToMQTT(MQTT_DHT_TEMPERATURE_SENSOR_TOPIC, MQTT_PAYLOAD);
      break;
    case DHT_HUMIDITY_SENSOR_EVT:
      dtostrf(ms.getDHTHumidity(), 2, 2, MQTT_PAYLOAD);
      publishToMQTT(MQTT_DHT_HUMIDITY_SENSOR_TOPIC, MQTT_PAYLOAD);
      break;
#endif
#if defined(SHT_SENSOR)
    case SHT_TEMPERATURE_SENSOR_EVT:
      dtostrf(ms.getSHTTemperature(), 2, 2, MQTT_PAYLOAD);
      publishToMQTT(MQTT_SHT_TEMPERATURE_SENSOR_TOPIC, MQTT_PAYLOAD);
      break;
    case SHT_HUMIDITY_SENSOR_EVT:
      dtostrf(ms.getSHTHumidity(), 2, 2, MQTT_PAYLOAD);
      publishToMQTT(MQTT_SHT_HUMIDITY_SENSOR_TOPIC, MQTT_PAYLOAD);
      break;
#endif
#if defined(BUTTON_SENSOR)
    case BUTTON_SENSOR_EVT:
      if (ms.getButtonState()) {
        publishToMQTT(MQTT_BUTTON_SENSOR_TOPIC, MQTT_PAYLOAD_ON);
      } else {
        publishToMQTT(MQTT_BUTTON_SENSOR_TOPIC, MQTT_PAYLOAD_OFF);
      }
      break;
#endif
  }
}

void setup() {
#if defined(DEBUG_SERIAL)
  Serial.begin(115200);
#endif

  setupWiFi();

#if defined(OTA)
  setupOTA();
#endif

  ms.init();
  ms.setCallback(onMultiSensorEvent);
  
  sprintf(MQTT_CLIENT_ID, "%06X", ESP.getChipId());
  sprintf(MQTT_AVAILABILITY_TOPIC, MQTT_AVAILABILITY_TOPIC_TEMPLATE, MQTT_CLIENT_ID);

  DEBUG_PRINT(F("INFO: MQTT availability topic: "));
  DEBUG_PRINTLN(MQTT_AVAILABILITY_TOPIC);
  
#if defined(MOTION_SENSOR)
  sprintf(MQTT_MOTION_SENSOR_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, MOTION_SENSOR_NAME);
  DEBUG_PRINT(F("INFO: MQTT motion sensor topic: "));
  DEBUG_PRINTLN(MQTT_MOTION_SENSOR_TOPIC);
#endif
#if defined(DOOR_SENSOR)
  sprintf(MQTT_DOOR_SENSOR_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME);
  DEBUG_PRINT(F("INFO: MQTT door sensor topic: "));
  DEBUG_PRINTLN(MQTT_DOOR_SENSOR_TOPIC);
#endif
#if defined(LDR_SENSOR)
  sprintf(MQTT_LDR_SENSOR_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, LDR_SENSOR_NAME);
  DEBUG_PRINT(F("INFO: MQTT luminosity sensor topic: "));
  DEBUG_PRINTLN(MQTT_LDR_SENSOR_TOPIC);
#endif
#if defined(DHT_SENSOR)
  sprintf(MQTT_DHT_TEMPERATURE_SENSOR_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DHT_TEMPERATURE_SENSOR_NAME);
  DEBUG_PRINT(F("INFO: MQTT DHT temperature sensor topic: "));
  DEBUG_PRINTLN(MQTT_DHT_TEMPERATURE_SENSOR_TOPIC);
  sprintf(MQTT_DHT_HUMIDITY_SENSOR_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DHT_HUMIDITY_SENSOR_NAME);
  DEBUG_PRINT(F("INFO: MQTT DHT humidity sensor topic: "));
  DEBUG_PRINTLN(MQTT_DHT_HUMIDITY_SENSOR_TOPIC);
#endif
#if defined(SHT_SENSOR)
  sprintf(MQTT_SHT_TEMPERATURE_SENSOR_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, SHT_TEMPERATURE_SENSOR_NAME);
  DEBUG_PRINT(F("INFO: MQTT SHT temperature sensor topic: "));
  DEBUG_PRINTLN(MQTT_SHT_TEMPERATURE_SENSOR_TOPIC);
  sprintf(MQTT_SHT_HUMIDITY_SENSOR_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, SHT_HUMIDITY_SENSOR_NAME);
  DEBUG_PRINT(F("INFO: MQTT SHT humidity sensor topic: "));
  DEBUG_PRINTLN(MQTT_SHT_HUMIDITY_SENSOR_TOPIC);
#endif
#if defined(BUTTON_SENSOR)
  sprintf(MQTT_BUTTON_SENSOR_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, BUTTON_SENSOR_NAME);
  DEBUG_PRINT(F("INFO: MQTT button sensor topic: "));
  DEBUG_PRINTLN(MQTT_BUTTON_SENSOR_TOPIC);
#endif

  mqttClient.setServer(MQTT_SERVER, MQTT_SERVER_PORT);

  connectToMQTT();
}

void loop() {
  connectToMQTT();
  mqttClient.loop();

  yield();
  
  ms.loop();

  yield();
  
#if defined(OTA)
  handleOTA();
  yield();
#endif
}
