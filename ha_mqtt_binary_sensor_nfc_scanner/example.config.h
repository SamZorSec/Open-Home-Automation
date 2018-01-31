///////////////////////////////////////////////////////////////////////////
//  CONFIGURATION - SOFTWARE
///////////////////////////////////////////////////////////////////////////
#define NB_OF_NFC_TRACKED_TAGS 2
NFCTag NFCTags[NB_OF_NFC_TRACKED_TAGS] = {
  {{0x11, 0x22, 0x33, 0x44, 0x0, 0x0, 0x0}, "MifareTag", false, 0, false, {0}},
  {{0x12, 0x23, 0x34, 0x45, 0x0, 0x0, 0x0}, "MifareCard", false, 0, false, {0}},
};

// send OFF payload after x ms
#define MAX_NON_DISCOVERED_PERIOD 1000

// Location of the BLE scanner
#define LOCATION "office"

// Debug output
#define DEBUG_SERIAL

// Wi-Fi credentials
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""

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

#define MQTT_PAYLOAD_AVAILABLE    "online"
#define MQTT_PAYLOAD_UNAVAILABLE  "offline"
