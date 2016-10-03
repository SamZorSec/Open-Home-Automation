/*
  Configuration for Home Assistant:
    mqtt:
      broker: 127.0.0.1
      port: 1883
      client_id: 'ha'
      username: 'ha'
      password: '!ha@OpenHome16'

    light:
      - platform: mqtt
        name: 'Lamp 1'
        state_topic: 'entrance/light1/status'
        command_topic: 'entrance/light1/switch'
        optimistic: false
      - platform: mqtt
        name: 'Lamp 2'
        state_topic: 'entrance/light2/status'
        command_topic: 'entrance/light2/switch'
        optimistic: false

    binary_sensor:
      - platform: mqtt
        name: 'Motion'
        state_topic: 'entrance/door/motion/status'
        sensor_class: motion

  Sources:
    - MQTT: File > Examples > PubSubClient > mqtt_esp8266
    - TLS:  https://io.adafruit.com/blog/security/2016/07/05/adafruit-io-security-esp8266/
    - OTA:  File > Examples > ArduinoOTA > BasicOTA
    - PIR:  https://learn.adafruit.com/pir-passive-infrared-proximity-motion-sensor/using-a-pir
*/

#include <ESP8266WiFi.h>    // https://github.com/esp8266/Arduino (GNUv2.1 licence)
#include <PubSubClient.h>   // https://github.com/knolleary/pubsubclient (no licence)
#include <ArduinoOTA.h>

//#define DEBUG
#define TLS
#define MQTT_VERSION MQTT_VERSION_3_1_1

// Wi-Fi: Access Point SSID and password
const char*       AP_SSID           = "[Redacted]";
const char*       AP_PASSWORD       = "[Redacted]";

// MQTT: client ID, broker IP address, port, username & password
const char*       MQTT_CLIENT_ID    = "entrance";
const char*       MQTT_SERVER_IP    = "192.168.1.10";
#ifdef TLS
const uint16_t    MQTT_SERVER_PORT  = 8883;
#else
const uint16_t    MQTT_SERVER_PORT  = 1883;
#endif
const char*       MQTT_USERNAME     = "entrance";
const char*       MQTT_PASSWORD     = "[Redacted]";

// MQTT: topics
// lamp 1
const char*       TOPIC_LIGHT1_STATUS   = "entrance/light1/status";
const char*       TOPIC_LIGHT1_COMMAND  = "entrance/light1/switch";
// lamp 2
const char*       TOPIC_LIGHT2_STATUS   = "entrance/light2/status";
const char*       TOPIC_LIGHT2_COMMAND  = "entrance/light2/switch";
// motion sensor
const char*       TOPIC_MOTION_STATUS   = "entrance/door/motion/status";

// MQTT: payloads
// Lamps 1 & 2 + motion sensor: "ON"/"OFF"
const char*       PAYLOAD_ON            = "ON";
const char*       PAYLOAD_OFF           = "OFF";

boolean           g_light1_status       = false; // turn off by default
boolean           g_light2_status       = false;
volatile boolean  g_motion_status       = false; // no motion by default
volatile boolean  g_motion_change       = false; // no publication to do by default


const uint8_t     LIGHT1_PIN            = D1;
const uint8_t     LIGHT2_PIN            = D2;
const uint8_t     MOTION_SENSOR_PIN     = D8; // connected before to D3, strange behaviour when D1/D2 was HIGH

// TLS: The fingerprint of the MQTT broker certificate (SHA1)
#ifdef TLS
// openssl x509 -fingerprint -in  <certificate>.crt
const char*       CA_FINGERPRINT    = "[Redacted]";
// openssl x509 -subject -in  <certificate>.crt
const char*       CA_SUBJECT        = "[Redacted]";
#endif

// Fixed IP address: IP address, IP gateway, subnet, dns
const IPAddress   IP                (192, 168,   1, 100);
const IPAddress   IP_GATEWAY        (192, 168,   1,   1);
const IPAddress   IP_SUBNET         (255, 255, 255,   0);
const IPAddress   IP_DNS            (192, 168,   1,   1);

// OTA: Hostname (MQTT_CLIENT_ID) & password
const char*       OTA_PASSWORD      = "[Redacted]";

// WiFiFlientSecure instead of WiFiClient, for SSL/TLS support
#ifdef TLS
WiFiClientSecure  g_wifiClient;
#else
WiFiClient  g_wifiClient;
#endif
PubSubClient      g_mqttClient(g_wifiClient);

///////////////////////////////////////////////////////////////////////////
//
// LIGHT 1
//
///////////////////////////////////////////////////////////////////////////

/*
  Function called to publish the status of the light 1
*/
void publishLight1Status() {
  if (g_light1_status) {
    if (g_mqttClient.publish(TOPIC_LIGHT1_STATUS, PAYLOAD_ON, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_LIGHT1_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_ON);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  } else {
    if (g_mqttClient.publish(TOPIC_LIGHT1_STATUS, PAYLOAD_OFF, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_LIGHT1_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_OFF);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  }
}

/*
  Function called to switch the status of the light 1
*/
void setLight1Status() {
  if (g_light1_status) {
    digitalWrite(LIGHT1_PIN, HIGH);
  } else {
    digitalWrite(LIGHT1_PIN, LOW);
  }
}

///////////////////////////////////////////////////////////////////////////
//
// LIGHT 2
//
///////////////////////////////////////////////////////////////////////////

/*
  Function called to publish the status of the light 2
*/
void publishLight2Status() {
  if (g_light2_status) {
    if (g_mqttClient.publish(TOPIC_LIGHT2_STATUS, PAYLOAD_ON, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_LIGHT2_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_ON);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  } else {
    if (g_mqttClient.publish(TOPIC_LIGHT2_STATUS, PAYLOAD_OFF, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_LIGHT2_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_OFF);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  }
}

/*
  Function called to switch the status of the light 2
*/
void setLight2Status() {
  if (g_light2_status) {
    digitalWrite(LIGHT2_PIN, HIGH);
  } else {
    digitalWrite(LIGHT2_PIN, LOW);
  }
}

///////////////////////////////////////////////////////////////////////////
//
// MOTION SENSOR
//
///////////////////////////////////////////////////////////////////////////

/*
  Function called when a motion is detected/ended
*/
void onMotionChanged() {
  g_motion_status = !g_motion_status;
  g_motion_change = true;
}

/*
  Function called to publish the status of the motion sensor
*/
void publishMotionStatus() {
  if (g_motion_status) {
    if (g_mqttClient.publish(TOPIC_MOTION_STATUS, PAYLOAD_ON, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_MOTION_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_ON);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  } else {
    if (g_mqttClient.publish(TOPIC_MOTION_STATUS, PAYLOAD_OFF, true)) {
#ifdef DEBUG
      Serial.print(F("INFO: MQTT message publish succeeded. Topic: "));
      Serial.print(TOPIC_MOTION_STATUS);
      Serial.print(F(". Payload: "));
      Serial.println(PAYLOAD_OFF);
#endif
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: MQTT message publish failed, either connection lost, or message too large"));
#endif
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//
// WIFI and TLS
//
///////////////////////////////////////////////////////////////////////////

/*
   Function called to setup the connection to the Wi-Fi Access Point
*/
void setupWifi() {
  delay(10);
  // attempt to connect to the Wi-Fi AP
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASSWORD);
  // define the fixed IP address
  WiFi.config(IP, IP_GATEWAY, IP_SUBNET, IP_DNS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

#ifdef DEBUG
  Serial.println(F("INFO: Client is now connected to the Wi-Fi AP"));
  Serial.print(F("INFO: IP address: "));
  Serial.println(WiFi.localIP());
#endif

#ifdef TLS
  verifyFingerprint();
#endif
}

/*
   Function called to verify the fingerprint of the MQTT server and establish a secure connection
*/
#ifdef TLS
void verifyFingerprint() {
  if (!g_wifiClient.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT)) {
#ifdef DEBUG
    Serial.println(F("ERROR: The connection failed to the secure MQTT server"));
#endif
    return;
  }

  if (g_wifiClient.verify(CA_FINGERPRINT, CA_SUBJECT)) {
#ifdef DEBUG
    Serial.println(F("INFO: The connection is secure"));
#endif
  } else {
#ifdef DEBUG
    Serial.println(F("ERROR: The given certificate does't match"));
#endif
  }
}
#endif

///////////////////////////////////////////////////////////////////////////
//
// MQTT
//
///////////////////////////////////////////////////////////////////////////

/*
   Function called when a MQTT message arrived
   @param p_topic   The topic of the MQTT message
   @param p_payload The payload of the MQTT message
   @param p_length  The length of the payload
*/
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
#ifdef DEBUG
  Serial.println(F("INFO: A new MQTT message arrived"));
  Serial.print(F("INFO: Topic: "));
  Serial.println(p_topic);
  Serial.print(F("INFO: Payload: "));
  for (int i = 0; i < p_length; i++) {
    Serial.print((char)p_payload[i]);
  }
  Serial.println();
  Serial.print(F("INFO: Length: "));
  Serial.println(p_length);
#endif
  // handle the MQTT topic of the received message
  if (String(TOPIC_LIGHT1_COMMAND).equals(p_topic)) {
    // concat the payload into a string
    String payload;
    for (uint8_t i = 0; i < p_length; i++) {
      payload.concat((char)p_payload[i]);
    }
    if (payload.equals(String(PAYLOAD_ON))) {
      g_light1_status = true;
      setLight1Status();
      publishLight1Status();
    } else if (payload.equals(String(PAYLOAD_OFF))) {
      g_light1_status = false;
      setLight1Status();
      publishLight1Status();
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: The payload of the MQTT message is not valid"));
#endif
    }
  } else if (String(TOPIC_LIGHT2_COMMAND).equals(p_topic)) {
    // concat the payload into a string
    String payload;
    for (uint8_t i = 0; i < p_length; i++) {
      payload.concat((char)p_payload[i]);
    }
    if (payload.equals(String(PAYLOAD_ON))) {
      g_light2_status = true;
      setLight2Status();
      publishLight2Status();
    } else if (payload.equals(String(PAYLOAD_OFF))) {
      g_light2_status = false;
      setLight2Status();
      publishLight2Status();
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: The payload of the MQTT message is not valid"));
#endif
    }
  } else {
    // do nothing.....
#ifdef DEBUG
    Serial.println(F("INFO: The received MQTT message was not used"));
#endif
  }
}

/*
   Function called to reconnect the client to the MQTT broker and publish/subscribe to/from some MQTT topics
*/
void reconnect() {
  while (!g_mqttClient.connected()) {
    if (g_mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
#ifdef DEBUG
      Serial.println(F("INFO: The client is successfully connected to the MQTT broker"));
#endif
      // subscribe to the light1 command topic
      if (g_mqttClient.subscribe(TOPIC_LIGHT1_COMMAND)) {
#ifdef DEBUG
        Serial.print(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
        Serial.println(TOPIC_LIGHT1_COMMAND);
#endif
      } else {
#ifdef DEBUG
        Serial.print(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
        Serial.println(TOPIC_LIGHT1_COMMAND);
#endif
      }

      // subscribe to the light2 command topic
      if (g_mqttClient.subscribe(TOPIC_LIGHT2_COMMAND)) {
#ifdef DEBUG
        Serial.print(F("INFO: Sending the MQTT subscribe succeeded. Topic: "));
        Serial.println(TOPIC_LIGHT2_COMMAND);
#endif
      } else {
#ifdef DEBUG
        Serial.print(F("ERROR: Sending the MQTT subscribe failed. Topic: "));
        Serial.println(TOPIC_LIGHT2_COMMAND);
#endif
      }

      // set the initial status of lights 1 & 2
      setLight1Status();
      setLight2Status();

      // publish the initial status of lights 1 & 2
      publishLight1Status();
      publishLight2Status();
    } else {
#ifdef DEBUG
      Serial.println(F("ERROR: The connection failed with the MQTT broker"));
      Serial.print("ERROR: rc: ");
      Serial.println(g_mqttClient.state());
      // wait 5 seconds before retrying
      delay(5000);
#endif
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//
// SETUP and LOOP
//
///////////////////////////////////////////////////////////////////////////

/*
   Function called once to initialize the board
*/
void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println(F("\nINFO: The Wi-Fi module is starting..."));
#endif

  // init the LEDs as output, the motion sensor as input
  pinMode(LIGHT1_PIN,         OUTPUT);
  pinMode(LIGHT2_PIN,         OUTPUT);
  pinMode(MOTION_SENSOR_PIN,  INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR_PIN), onMotionChanged, CHANGE);
  pinMode(BUILTIN_LED,        OUTPUT);  // the built-in LED is used to indicate a motion
  digitalWrite(BUILTIN_LED,   HIGH);    // turn off the built-in LED


  setupWifi();

  // set the MQTT broker IP address and port
  g_mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  // set the MQTT callback function
  g_mqttClient.setCallback(callback);

  // set the hostname & password for OTA
  ArduinoOTA.setHostname(MQTT_CLIENT_ID);
  ArduinoOTA.setPassword(OTA_PASSWORD);

#ifdef DEBUG
  ArduinoOTA.onStart([]() {
    Serial.println(F("INFO: OTA starts"));
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("INFO: OTA ends"));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.println(F("INFO: An error occurs during OTA"));
  });
#endif
  ArduinoOTA.begin();

  // blink the internal LED to indicate the end of the startup
  digitalWrite(BUILTIN_LED, LOW);
  delay(100);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(100);
  digitalWrite(BUILTIN_LED, LOW);
  delay(100);
  digitalWrite(BUILTIN_LED, HIGH);
}

/*
   Function called infinitely after the setup function
*/
void loop() {
  // keep the MQTT client connected to the broker
  if (!g_mqttClient.connected()) {
    reconnect();
  }
  g_mqttClient.loop();

  yield();

  ArduinoOTA.handle();

  yield();
  
  // test if the status of the motion sensor was changed by an interrupt
  if (g_motion_change) {
    publishMotionStatus();
    g_motion_change = false;
    if (g_motion_status) {
      digitalWrite(BUILTIN_LED, LOW); // turn on the built-in LED
#ifdef DEBUG
      Serial.println(F("INFO: Motion detected"));
#endif
    } else {
      digitalWrite(BUILTIN_LED, HIGH); // turn off the built-in LED
#ifdef DEBUG
      Serial.println(F("INFO: Motion ended"));
#endif
    }
  }
  
  yield();
}
