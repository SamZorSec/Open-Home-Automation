///////////////////////////////////////////////////////////////////////////
//  CONFIGURATION - SOFTWARE
///////////////////////////////////////////////////////////////////////////
#define NB_OF_BLE_TRACKED_DEVICES 1
BLETrackedDevice BLETrackedDevices[NB_OF_BLE_TRACKED_DEVICES] = {
  {"11:22:33:44:55:66", false, 0, false, {0}}
};

#define BLE_SCANNING_PERIOD   5
#define MAX_NON_ADV_PERIOD    10000

// Location of the BLE scanner
#define LOCATION "Bedroom"

// Debug output
#define DEBUG_SERIAL

// Wi-Fi credentials
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""

// Over-the-Air update
// Not implemented yet
//#define OTA
//#define OTA_HOSTNAME  ""    // hostname esp8266-[ChipID] by default
//#define OTA_PASSWORD  ""    // no password by default
//#define OTA_PORT      8266  // port 8266 by default

// MQTT
#define MQTT_USERNAME     ""
#define MQTT_PASSWORD     ""
#define MQTT_SERVER       ""
#define MQTT_SERVER_PORT  1883

#define MQTT_CONNECTION_TIMEOUT 5000 // [ms]

// MQTT availability: available/unavailable
#define MQTT_AVAILABILITY_TOPIC_TEMPLATE  "%s/availability" 
// MQTT binary sensor: <CHIP_ID>/sensor/<LOCATION>/<BLE_ADDRESS>
#define MQTT_SENSOR_TOPIC_TEMPLATE        "%s/sensor/%s/%s/state"

#define MQTT_PAYLOAD_ON   "ON"
#define MQTT_PAYLOAD_OFF  "OFF"

#define MQTT_PAYLOAD_AVAILABLE    "available"
#define MQTT_PAYLOAD_UNAVAILABLE  "unavailable"
