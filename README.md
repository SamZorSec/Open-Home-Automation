# Open Home Automation with Home-Assistant
## Home-Assitant
> Home Assistant is a home automation platform running on Python 3. The goal of Home Assistant is to be able to track and control all devices at home and offer a platform for automating control [[Home-Assistant](https://github.com/home-assistant/home-assistant)].

![Home-Assitant](https://github.com/home-assistant/home-assistant/blob/dev/docs/screenshots.png)

## MQTT
> MQTT is a machine-to-machine (M2M)/"Internet of Things" connectivity protocol. It was designed as an extremely lightweight publish/subscribe messaging transport. It is useful for connections with remote locations where a small code footprint is required and/or network bandwidth is at a premium [[mqtt.org](http://mqtt.org)].

### Configuration
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

### Examples
Lights, sensors, switches and more can be built on top of MQTT. This section contains a few examples based on MQTT and on a NodeMCU board (ESP8266).

| Title / link     								| Description        											| 
|-----------------------------------------------|---------------------------------------------------------------|
| [Light](/ha_mqtt_light)     					| A simple example to control a led     						|
| [Light](/ha_mqtt_rgb_light) 					| A simple example to control a RGB led 				  	    |
| [Switch](/ha_mqtt_switch)   					| A simple example to control a switch  					 	| 
| [Sensor](/ha_mqtt_sensor_dht22) 				| A simple example to measure the temperature and the humidity  |
| [Sensor](/ha_mqtt_sensor_photocell) 			| A simple example to measure the brightness 					|
| [Binary Sensor](/ha_mqtt_binary_sensor_pir) 	| A simple example to detect motions 							|