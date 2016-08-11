/*
   MQTT Light for Home-Assistant (esp8266)
   https://home-assistant.io/components/light.mqtt/

   Libraries :
    - ESP8266 core for Arduino : https://github.com/esp8266/Arduino
    - PubSubClient : https://github.com/knolleary/pubsubclient

   Sources :
    - File > Examples > ES8266WiFi > WiFiClient
    - File > Examples > PubSubClient > mqtt_auth
    - File > Examples > PubSubClient > mqtt_esp8266

   Samuel M. - 07.2016
   If you like this example, please add a star! Thank you!
   https://github.com/mertenats/open-home-automation
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi ssid and password
const char* WIFI_SSID = "[Redacted]";
const char* WIFI_PASSWORD = "[Redacted]";

// MQTT server IP and port, username and password
const char* MQTT_CLIENT_ID = "office_light1";
const char* MQTT_SERVER_IP = "[Redacted]";
const uint16_t MQTT_SERVER_PORT = 1883;
// const char* MQTT_USER = "";
// const char* MQTT_PASSWORD = "";

// MQTT topics
const char* MQTT_LIGHT_STATE_TOPIC = "office/light1/status";
const char* MQTT_LIGHT_COMMAND_TOPIC = "office/light1/switch";

// Light
// The payload that represents enabled/disabled state, by default
const char* LIGHT_ON = "ON";
const char* LIGHT_OFF = "OFF";
boolean light_state = false;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  if (String(MQTT_LIGHT_COMMAND_TOPIC).equals(topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if ((char)payload[0] == (char)LIGHT_ON[0] && (char)payload[1] == (char)LIGHT_ON[1]) {
      if (light_state != true) {
        light_state = true;
        digitalWrite(BUILTIN_LED, LOW);
        client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_ON);
      }
    } else if ((char)payload[0] == (char)LIGHT_OFF[0] && (char)payload[1] == (char)LIGHT_OFF[1] && (char)payload[2] == (char)LIGHT_OFF[2]) {
      if (light_state != false) {
        light_state = false;
        digitalWrite(BUILTIN_LED, HIGH);
        client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_OFF);
      }
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID/*, MQTT_USER, MQTT_PASSWORD*/)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      if (light_state) {
        client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_ON);
      } else {
        client.publish(MQTT_LIGHT_STATE_TOPIC, LIGHT_OFF);
      }
      // ... and resubscribe
      client.subscribe(MQTT_LIGHT_COMMAND_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // init the serial
  Serial.begin(115200);

  // init the build-in led
  pinMode(BUILTIN_LED, OUTPUT);
  if (light_state)
    digitalWrite(BUILTIN_LED, LOW);
  else
    digitalWrite(BUILTIN_LED, HIGH);

  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
