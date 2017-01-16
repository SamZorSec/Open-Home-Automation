/*
   MQTT Switch for Home-Assistant - NodeMCU (ESP8266)
   https://home-assistant.io/components/switch.mqtt/

   Libraries :
    - ESP8266 core for Arduino : https://github.com/esp8266/Arduino
    - PubSubClient : https://github.com/knolleary/pubsubclient
    - OneButton : https://github.com/mathertel/OneButton

   Sources :
    - File > Examples > ES8266WiFi > WiFiClient
    - File > Examples > PubSubClient > mqtt_auth
    - File > Examples > PubSubClient > mqtt_esp8266
    - OneButton : http://www.mathertel.de/Arduino/OneButtonLibrary.aspx

   Schematic :
    - https://github.com/mertenats/open-home-automation/blob/master/ha_mqtt_switch/Schematic.png
    - Switch leg 1 - VCC
    - Switch leg 2 - D1/GPIO5 - Resistor 10K Ohms - GND

   Configuration (HA) :
    switch:
      platform: mqtt
      name: 'Office Switch'
      state_topic: 'office/switch1/status'
      command_topic: 'office/switch1/set'
      retain: true
      optimistic: false

   Samuel M. - v1.1 - 08.2016
   If you like this example, please add a star! Thank you!
   https://github.com/mertenats/open-home-automation
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneButton.h>

#define MQTT_VERSION MQTT_VERSION_3_1_1

// Wifi: SSID and password
const char* WIFI_SSID = "[Redacted]";
const char* WIFI_PASSWORD = "[Redacted]";

// MQTT: ID, server IP, port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "office_switch1";
const PROGMEM char* MQTT_SERVER_IP = "[Redacted]";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "[Redacted]";
const PROGMEM char* MQTT_PASSWORD = "[Redacted]";

// MQTT: topics
const PROGMEM char* MQTT_SWITCH_STATUS_TOPIC = "office/switch1/status";
const PROGMEM char* MQTT_SWITCH_COMMAND_TOPIC = "office/switch1/set";

// default payload
const PROGMEM char* SWITCH_ON = "ON";
const PROGMEM char* SWITCH_OFF = "OFF";

// store the state of the switch
boolean m_switch_state = false;

// D1/GPIO5
const PROGMEM uint8_t BUTTON_PIN = 5;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
OneButton button(BUTTON_PIN, false); // false : active HIGH

// function called on button press
// toggle the state of the switch
void click() {
  if (m_switch_state) {
    m_switch_state = false;
  } else {
    m_switch_state = true;
  }
  publishSwitchState();
}

// function called to publish the state of the switch (on/off)
void publishSwitchState() {
  if (m_switch_state) {
    client.publish(MQTT_SWITCH_STATUS_TOPIC, SWITCH_ON, true);
  } else {
    client.publish(MQTT_SWITCH_STATUS_TOPIC, SWITCH_OFF, true);
  }
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }
  // handle message topic
  if (String(MQTT_SWITCH_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(SWITCH_ON))) {
      if (m_switch_state != true) {
        m_switch_state = true;
        publishSwitchState();
      }
    } else if (payload.equals(String(SWITCH_OFF))) {
      if (m_switch_state != false) {
        m_switch_state = false;
        publishSwitchState();
      }
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("INFO: connected");
      
      // Once connected, publish an announcement...
      // publish the initial values
      publishSwitchState();

      // ... and resubscribe
      client.subscribe(MQTT_SWITCH_COMMAND_TOPIC);
    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // init the serial
  Serial.begin(115200);
  
  // link the click function to be called on a single click event.   
  button.attachClick(click);

  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.println("INFO: IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  // keep watching the push button:
  button.tick();
  delay(10);
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
