In our home automation system we are running [[Home-Assistant](https://github.com/home-assistant/home-assistant)], a home automation platform running on Python 3. The goal of Home Assistant is to be able to track and control all devices at home and offer a platform for automating control. 

## MQTT
> MQTT is a machine-to-machine (M2M)/"Internet of Things" connectivity protocol. It was designed as an extremely lightweight publish/subscribe messaging transport. It is useful for connections with remote locations where a small code footprint is required and/or network bandwidth is at a premium [[mqtt.org](http://mqtt.org)].

## Configuration
To integrate MQTT into Home Assistant, a MQTT broker is mandatory. More information can be found at the [MQTT component's page](https://home-assistant.io/components/mqtt/).

configuration.yaml :
```yaml
mqtt:
  broker: 127.0.0.1
  port: 1883
  client_id: '[Redacted]'
  username: '[Redacted]'
  password: '[Redacted]'
```

## Nodes
Lights, sensors, switches and more can be built on top of MQTT. This section contains a few examples based on MQTT and on a NodeMCU board (ESP8266).

| Title / link     							              	| Description        											                              | 
|-----------------------------------------------|-----------------------------------------------------------------------|
| [Light](/ha_mqtt_light)     				        	| A simple example to control a **led**     					                	|
| [Light](/ha_mqtt_light_with_brightness)     	| A simple example to control a **led** and its brightness	          	|
| [Light](/ha_mqtt_rgb_light) 				        	| A simple example to control a **RGB led** 				              	    |
| [Switch](/ha_mqtt_switch)   				        	| A simple example to control a **switch**  	
| [Switch](https://github.com/mertenats/sonoff)   				        	| An alternative firmware for the **Sonoff switch**  					                 	| | 
| [Sensor](/ha_mqtt_sensor_dht22) 			      	| A simple example to measure the **temperature** and the **humidity** (DHT22 sensor)|
| [Sensor](/ha_mqtt_sensor_photocell) 		    	| A simple example to measure the **brightness** (photocell)|
| [Binary Sensor](/ha_mqtt_binary_sensor_pir) 	| A simple example to detect **motions** (PIR motion sensor)|
| [Binary Sensor](/ha_mqtt_pir_led) 				          	| A simple example to detect **motions** and turn on an **led**|


An advanced example, using WiFiManager, mDNS and OTA, is available [here](/ha_mqtt_light_with_WiFiManager_mDNS_and_OTA).

A school project based on Home Assistant, MQTT and NodeMCU modules is available [here](openhome) and the demo, [here](https://www.youtube.com/watch?v=Vh-vzFPCF2U).

*If you like the content of this repo, please add a star! Thank you!*
