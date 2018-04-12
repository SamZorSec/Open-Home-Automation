# Open Home Automation

## Introduction
Nowadays everything becomes connected to the Internet and gives us a glimpse of many new possibilities. Home automation is part of it and offers many advantages for their users.
This repository is dedicated to [Home Assistant](https://home-assistant.io), an open source project with an amazing community, ESP8266 and ESP32 modules, the MQTT protocol and much more [...].

### Home Assistant
> Home Assistant is a home automation platform running on Python 3. The goal of Home Assistant is to be able to track and control all devices at home and offer a platform for automating control [[Home-Assistant](https://github.com/home-assistant/home-assistant)].

### MQTT
> MQTT is a machine-to-machine (M2M)/"Internet of Things" connectivity protocol. It was designed as an extremely lightweight publish/subscribe messaging transport. It is useful for connections with remote locations where a small code footprint is required and/or network bandwidth is at a premium [[mqtt.org](http://mqtt.org)].

![Home-Assistant](openhome/images/Features.png)

## Content
### Requirements
Most of the examples below are using the MQTT protocol and so require to integrate MQTT into Home Assistant by defining a MQTT broker. More information can be found at the [MQTT component's page](https://home-assistant.io/components/mqtt/).

#### Stock Home Assistant
configuration.yaml :

```yaml
mqtt:
  broker: 127.0.0.1
  port: 1883                      
  username: '[Redacted]'
  password: '[Redacted]'
  discovery: true                 # optional
  discovery_prefix: homeassistant # optional
```

#### Hass.io
configuration.yaml :

```yaml
mqtt:
  broker: core-mosquitto
  username: '[Redacted]'
  password: '[Redacted]'
  discovery: true                 # optional
  discovery_prefix: homeassistant # optional
```

More options to connect the broker are available and described [here](https://home-assistant.io/docs/mqtt/broker/#embedded-broker).

### Sketches for ESP8266/ESP32 modules
Lights, sensors, switches and more can be built on top of MQTT. This section contains a few examples based on MQTT and on a NodeMCU board (ESP8266/ESP32).

| Title / link                                  | Description                                                           |
|-----------------------------------------------|-----------------------------------------------------------------------|
| [Light](/ha_mqtt_light)                       | A simple example to control a **led**                                 |
| [Light](/ha_mqtt_light_with_brightness)       | A simple example to control a **led** and its brightness              |
| [Light](/ha_mqtt_rgb_light)                   | A simple example to control a **RGB led**                             |
| [Light](/ha_mqtt_light_with_WiFiManager_mDNS_and_OTA) | A simple example to control a **RGB led** (with **OTA** and **mDNS**)|
| [Light](https://github.com/mertenats/Arilux_AL-LC03)| An alternative firmware for the **Arilux AL-LC0X LED controller** |
| [Light](/ha_mqtt_rgbw_ligh_with_discovery)    | A simple example to control a **RGBW led**, based on the **MQTT JSON Light** component (brightness, rgb, white, color temperature and effects) and including the **MQTT Discovery**                         |
| [Light](https://github.com/mertenats/AI-Thinker_RGBW_Bulb)    | An alternative firmware for **AI-Thinker RGBW bulbs**, based on the **MQTT JSON Light** component and including the **MQTT Discovery**  functionality                       |
| [Switch](/ha_mqtt_switch)                     | A simple example to control a **switch**    
| [Switch](https://github.com/mertenats/Itead_Sonoff/tree/master/Sonoff_Basic) | An alternative firmware for the **Sonoff Basic** switches                      |
| [Switch](https://github.com/mertenats/Itead_Sonoff/tree/master/Sonoff_TH) | An alternative firmware for the **Sonoff TH** switches, including the **MQTT Discovery**  functionality                 |
| [Sensor](/ha_mqtt_sensor_dht22)               | A simple example to measure the **temperature** and the **humidity** (DHT22 sensor)|
| [Sensor](/ha_mqtt_sensor_photocell)           | A simple example to measure the **brightness** (photocell)|
| [Binary Sensor](/ha_mqtt_binary_sensor_pir)   | A simple example to detect **motions** (PIR motion sensor)|
| [Binary Sensor](/ha_mqtt_binary_sensor_door)  | A simple example to monitor the **state** of a **window/door** |
| [Binary Sensor](/ha_mqtt_multisensor)         | A full example describing how to monitor your **environment**|
| [Binary Sensor](/ha_mqtt_binary_sensor_ble_scanner)| An example describing how to trigger an action when a specific **BLE device** is detected (ESP32)|
| [Binary Sensor](/ha_mqtt_binary_sensor_nfc_scanner)| An example describing how to trigger an action when a specific **NFC tag** is detected|

### Configuration examples for Home Assistant
This section contains a few configuration examples dedicated to Home Assistant.

| Title / link                                  | Description                                                           |
|-----------------------------------------------|-----------------------------------------------------------------------|
| [Alarm Clock](/ha_config_alarm_clock)         | An example describing how to create an **alarm clock** with the components provided by HA |
| [openhome](/openhome)                         | A school project based on Home Assistant and using only **open source** HW and SW |
| [Zigate & ZigBee devices](/ha_config_zigate)  | An example describing how to add and control **ZigBee** devices paired to a **Zigate** |

[![OpenHome with Home Assistant and MQTT](openhome/images/Youtube.png)](https://www.youtube.com/watch?v=Vh-vzFPCF2U "OpenHome with Home Assistant and MQTT")

## Miscelleneous

> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*If you like the content of this repo, please add a star! Thank you!*
