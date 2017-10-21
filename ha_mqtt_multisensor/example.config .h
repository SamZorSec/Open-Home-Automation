///////////////////////////////////////////////////////////////////////////
//  CONFIGURATION - HARDWARE
///////////////////////////////////////////////////////////////////////////
// Door sensor
#define DOOR_SENSOR                     D8
#if defined(DOOR_SENSOR)
#define DOOR_SENSOR_NAME                "door" // used for the MQTT topic
#endif

// Motion sensor
#define PIR_SENSOR                      D7
#if defined(PIR_SENSOR)
#define PIR_SENSOR_NAME                 "motion"
#endif

// Photoresistor sensor
#define LDR_SENSOR                      A0
#if defined(LDR_SENSOR)
#define LDR_SENSOR_NAME                 "lux"
#define LDR_OFFSET_VALUE                25
#define LDR_MEASURE_INTERVAL            1000  // [ms]
#define LDR_REFERENCE_VOLTAGE           3.3   // [v]
#define LDR_ADC_PRECISION               1024  // 10 bits
#define LDR_VOLTAGE_PER_ADC_PRECISION   LDR_REFERENCE_VOLTAGE / LDR_ADC_PRECISION
#define LDR_RESISTOR_VALUE              10.0  // [kOhms]
#endif

// Temperature and humidity sensor (DHT22)
#define DHT22_SENSOR                  D2
#if defined(DHT22_SENSOR)
#define DHT22_TEMPERATURE_SENSOR_NAME   "temperature"
#define DHT22_HUMIDITY_SENSOR_NAME      "humidity"
#define DHT22_TEMPERATURE_OFFSET_VALUE  0.5   // [°C]
#define DHT22_HUMIDITY_OFFSET_VALUE     1     // [%]
#define DHT22_MEASURE_INTERVAL          5000  // [ms]
#endif

// Button
#define BUTTON_SENSOR                   D1
#if defined(BUTTON_SENSOR)
#define BUTTON_SENSOR_NAME              "button"
#endif

///////////////////////////////////////////////////////////////////////////
//  CONFIGURATION - SOFTWARE
///////////////////////////////////////////////////////////////////////////
// Debug output
#define DEBUG_SERIAL

// Wi-Fi credentials
#define WIFI_SSID     ""
#define WIFI_PASSWORD ""

// Over-the-Air update
#define OTA
#define OTA_HOSTNAME  "MultiSensor"  // hostname esp8266-[ChipID] by default
//#define OTA_PASSWORD  "password"  // no password by default
//#define OTA_PORT      8266        // port 8266 by default

// MQTT
#define MQTT_USERNAME     ""
#define MQTT_PASSWORD     ""
#define MQTT_SERVER       ""
#define MQTT_SERVER_PORT  1883

#define MQTT_CONNECTION_TIMEOUT 5000 // [ms]

#define MQTT_AVAILABILITY_TOPIC_TEMPLATE  "%s/status" // MQTT availability: online/offline
#define MQTT_SENSOR_TOPIC_TEMPLATE        "%s/sensor/%s"

#define MQTT_PAYLOAD_ON   "ON"
#define MQTT_PAYLOAD_OFF  "OFF"
