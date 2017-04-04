/*
  MQTT Discovery and MQTT JSON Light for Home Assistant
  
  Samuel Mertenat
  04.2017
*/

#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <PubSubClient.h>         // https://github.com/knolleary/pubsubclient
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson
#include <ArduinoOTA.h>
#include "ha_mqtt_rgbw_light_with_discovery.h"

#if defined(DEBUG_TELNET)
WiFiServer  telnetServer(23);
WiFiClient  telnetClient;
#define     DEBUG_PRINT(x)    telnetClient.print(x)
#define     DEBUG_PRINTLN(x)  telnetClient.println(x)
#elif defined(DEBUG_SERIAL)
#define     DEBUG_PRINT(x)    Serial.print(x)
#define     DEBUG_PRINTLN(x)  Serial.println(x)
#else
#define     DEBUG_PRINT(x)
#define     DEBUG_PRINTLN(x)
#endif

#if defined(MQTT_HOME_ASSISTANT_SUPPORT)
StaticJsonBuffer<256> staticJsonBuffer;
char jsonBuffer[256] = {0};
#endif

volatile uint8_t cmd = CMD_NOT_DEFINED;

AIRGBWBulb    bulb;
WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);

///////////////////////////////////////////////////////////////////////////
//   TELNET
///////////////////////////////////////////////////////////////////////////
/*
   Function called to handle Telnet clients
   https://www.youtube.com/watch?v=j9yW10OcahI
*/
#if defined(DEBUG_TELNET)
void handleTelnet(void) {
  if (telnetServer.hasClient()) {
    if (!telnetClient || !telnetClient.connected()) {
      if (telnetClient) {
        telnetClient.stop();
      }
      telnetClient = telnetServer.available();
    } else {
      telnetServer.available().stop();
    }
  }
}
#endif

///////////////////////////////////////////////////////////////////////////
//   WiFi
///////////////////////////////////////////////////////////////////////////
/*
   Function called to handle WiFi events
*/
void handleWiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
      DEBUG_PRINTLN(F("INFO: WiFi connected"));
      DEBUG_PRINT(F("INFO: IP address: "));
      DEBUG_PRINTLN(WiFi.localIP());
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      DEBUG_PRINTLN(F("ERROR: WiFi losts connection"));
      /*
         TODO: Do something smarter than rebooting the device
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

char MQTT_CLIENT_ID[7] = {0};
#if defined(MQTT_HOME_ASSISTANT_SUPPORT)
char MQTT_CONFIG_TOPIC[sizeof(MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX) + sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_CONFIG_TOPIC_TEMPLATE) - 4] = {0};
#else

#endif

char MQTT_STATE_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_STATE_TOPIC_TEMPLATE) - 2] = {0};
char MQTT_COMMAND_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_COMMAND_TOPIC_TEMPLATE) - 2] = {0};
char MQTT_STATUS_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_STATUS_TOPIC_TEMPLATE) - 2] = {0};

volatile unsigned long lastMQTTConnection = MQTT_CONNECTION_TIMEOUT;
/*
   Function called when a MQTT message has arrived
   @param p_topic   The topic of the MQTT message
   @param p_payload The payload of the MQTT message
   @param p_length  The length of the payload
*/
void handleMQTTMessage(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concatenates the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }

  DEBUG_PRINTLN(F("INFO: New MQTT message received"));
  DEBUG_PRINT(F("INFO: MQTT topic: "));
  DEBUG_PRINTLN(p_topic);
  DEBUG_PRINT(F("INFO: MQTT payload: "));
  DEBUG_PRINTLN(payload);
  
  if (String(MQTT_COMMAND_TOPIC).equals(p_topic)) {
    DynamicJsonBuffer dynamicJsonBuffer;
    JsonObject& root = dynamicJsonBuffer.parseObject(p_payload);
    if (!root.success()) {
      DEBUG_PRINTLN(F("ERROR: parseObject() failed"));
      return;
    }

    if (root.containsKey("state")) {
      if (strcmp(root["state"], MQTT_STATE_ON_PAYLOAD) == 0) {
        if (bulb.setState(true)) {
          DEBUG_PRINT(F("INFO: State changed to: "));
          DEBUG_PRINTLN(bulb.getState());
          cmd = CMD_STATE_CHANGED;
        }
      } else if (strcmp(root["state"], MQTT_STATE_OFF_PAYLOAD) == 0) {
        // stops the possible current effect
        bulb.setEffect(EFFECT_NOT_DEFINED_NAME);
        
        if (bulb.setState(false)) {
          DEBUG_PRINT(F("INFO: State changed to: "));
          DEBUG_PRINTLN(bulb.getState());
          cmd = CMD_STATE_CHANGED;
        }
      }
    }
    
    if (root.containsKey("color")) {
      // stops the possible current effect
      bulb.setEffect(EFFECT_NOT_DEFINED_NAME);
      
      uint8_t r = root["color"]["r"];
      uint8_t g = root["color"]["g"];
      uint8_t b = root["color"]["b"];

      if (bulb.setColor(r, g, b)) {
        DEBUG_PRINT(F("INFO: Color changed to: "));
        DEBUG_PRINT(bulb.getColor().red);
        DEBUG_PRINT(F(", "));
        DEBUG_PRINT(bulb.getColor().green);
        DEBUG_PRINT(F(", "));
        DEBUG_PRINTLN(bulb.getColor().blue);
        cmd = CMD_STATE_CHANGED;
      }
    }

    if (root.containsKey("brightness")) {
      if (bulb.setBrightness(root["brightness"])) {
        DEBUG_PRINT(F("INFO: Brightness changed to: "));
        DEBUG_PRINTLN(bulb.getBrightness());
        cmd = CMD_STATE_CHANGED;
      }
    }

    if (root.containsKey("white_value")) {
      // stops the possible current effect
      bulb.setEffect(EFFECT_NOT_DEFINED_NAME);
      
      if (bulb.setWhite(root["white_value"])) {
        DEBUG_PRINT(F("INFO: White changed to: "));
        DEBUG_PRINTLN(bulb.getColor().white);
        cmd = CMD_STATE_CHANGED;
      }
    }

    if (root.containsKey("color_temp")) {
      // stops the possible current effect
      bulb.setEffect(EFFECT_NOT_DEFINED_NAME);
      
      if (bulb.setColorTemperature(root["color_temp"])) {
        DEBUG_PRINT(F("INFO: Color temperature changed to: "));
        DEBUG_PRINTLN(bulb.getColorTemperature());
        cmd = CMD_STATE_CHANGED;
      }
    }
    
    if (root.containsKey("effect")) {
      const char* effect = root["effect"];
      if (bulb.setEffect(effect)) {
        DEBUG_PRINTLN(F("INFO: Effect started"));
        cmd = CMD_NOT_DEFINED;
      }
    }
  }
}

/*
  Function called to subscribe to a MQTT topic
*/
void subscribeToMQTT(char* p_topic) {
  if (mqttClient.subscribe(p_topic)) {
    DEBUG_PRINT(F("INFO: Sending the MQTT subscribe succeeded for topic: "));
    DEBUG_PRINTLN(p_topic);
  } else {
    DEBUG_PRINT(F("ERROR: Sending the MQTT subscribe failed for topic: "));
    DEBUG_PRINTLN(p_topic);
  }
}

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
    if (lastMQTTConnection + MQTT_CONNECTION_TIMEOUT < millis()) {
      if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_STATUS_TOPIC, 0, 1, "dead")) {
        DEBUG_PRINTLN(F("INFO: The client is successfully connected to the MQTT broker"));
        publishToMQTT(MQTT_STATUS_TOPIC, "alive");

#if defined(MQTT_HOME_ASSISTANT_SUPPORT)
        // MQTT discovery for Home Assistant
        JsonObject& root = staticJsonBuffer.createObject();
        root["name"] = FRIENDLY_NAME;
        root["platform"] = "mqtt_json";
        root["state_topic"] = MQTT_STATE_TOPIC;
        root["command_topic"] = MQTT_COMMAND_TOPIC;
        root["brightness"] = true;
        root["rgb"] = true;
        root["white_value"] = true;
        root["color_temp"] = true;
        root["effect"] = true;
        root["effect_list"] = EFFECT_LIST;
        root.printTo(jsonBuffer, sizeof(jsonBuffer));
        publishToMQTT(MQTT_CONFIG_TOPIC, jsonBuffer);
#endif

        subscribeToMQTT(MQTT_COMMAND_TOPIC);
      } else {
        DEBUG_PRINTLN(F("ERROR: The connection to the MQTT broker failed"));
        DEBUG_PRINT(F("INFO: MQTT username: "));
        DEBUG_PRINTLN(MQTT_USERNAME);
        DEBUG_PRINT(F("INFO: MQTT password: "));
        DEBUG_PRINTLN(MQTT_PASSWORD);
        DEBUG_PRINT(F("INFO: MQTT broker: "));
        DEBUG_PRINTLN(MQTT_SERVER);
      }
        lastMQTTConnection = millis();
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//  CMD
///////////////////////////////////////////////////////////////////////////

void handleCMD() {
  switch(cmd) {
    case CMD_NOT_DEFINED:
      break;
    case CMD_STATE_CHANGED:
      cmd = CMD_NOT_DEFINED;
      DynamicJsonBuffer dynamicJsonBuffer;
      JsonObject& root = dynamicJsonBuffer.createObject();
      root["state"] = bulb.getState() ? MQTT_STATE_ON_PAYLOAD : MQTT_STATE_OFF_PAYLOAD;
      root["brightness"] = bulb.getBrightness();
      JsonObject& color = root.createNestedObject("color");
      color["r"] = bulb.getColor().red;
      color["g"] = bulb.getColor().green;
      color["b"] = bulb.getColor().blue;
      root["white_value"] = bulb.getColor().white;
      root["color_temp"] = bulb.getColorTemperature();
      root.printTo(jsonBuffer, sizeof(jsonBuffer));
      publishToMQTT(MQTT_STATE_TOPIC, jsonBuffer);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////
//  SETUP() AND LOOP()
///////////////////////////////////////////////////////////////////////////

void setup() {
#if defined(DEBUG_SERIAL)
  Serial.begin(115200);
#elif defined(DEBUG_TELNET)
  telnetServer.begin();
  telnetServer.setNoDelay(true);
#endif

  setupWiFi();

  sprintf(MQTT_CLIENT_ID, "%06X", ESP.getChipId());
#if defined(MQTT_HOME_ASSISTANT_SUPPORT)
  sprintf(MQTT_CONFIG_TOPIC, MQTT_CONFIG_TOPIC_TEMPLATE, MQTT_HOME_ASSISTANT_DISCOVERY_PREFIX, MQTT_CLIENT_ID);
  DEBUG_PRINT(F("INFO: MQTT config topic: "));
  DEBUG_PRINTLN(MQTT_CONFIG_TOPIC);
#else

#endif

  sprintf(MQTT_STATE_TOPIC, MQTT_STATE_TOPIC_TEMPLATE, MQTT_CLIENT_ID);
  sprintf(MQTT_COMMAND_TOPIC, MQTT_COMMAND_TOPIC_TEMPLATE, MQTT_CLIENT_ID);
  sprintf(MQTT_STATUS_TOPIC, MQTT_STATUS_TOPIC_TEMPLATE, MQTT_CLIENT_ID);

  DEBUG_PRINT(F("INFO: MQTT state topic: "));
  DEBUG_PRINTLN(MQTT_STATE_TOPIC);
  DEBUG_PRINT(F("INFO: MQTT command topic: "));
  DEBUG_PRINTLN(MQTT_COMMAND_TOPIC);
  DEBUG_PRINT(F("INFO: MQTT status topic: "));
  DEBUG_PRINTLN(MQTT_STATUS_TOPIC);

  mqttClient.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
  mqttClient.setCallback(handleMQTTMessage);

  connectToMQTT();

#if defined(OTA)
  setupOTA();
#endif

  bulb.init();

  cmd = CMD_STATE_CHANGED;
}

void loop() {
#if defined(DEBUG_TELNET)
  // handle the Telnet connection
  handleTelnet();
#endif

  yield();

#if defined(OTA)
  handleOTA();
#endif

  yield();

  connectToMQTT();
  mqttClient.loop();

  yield();

  handleCMD();

  yield();

  bulb.loop();
}
