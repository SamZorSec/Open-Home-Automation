# Open Home Automation with Home-Assistant

## MQTT
A MQTT broker is mandatory. More information can be found at the [MQTT component's page](https://home-assistant.io/components/mqtt/)

### Configuration
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
- [ha_mqtt_light](/ha_mqtt_light) : a simple example to control the built-in led of the nodeMCU board (esp8266)
- [ha_mqtt_rgb_light](/ha_mqtt_rgb_light) : a simple example to control a rgb led connected to a nodeMCU board (esp8266)
- [ha_mqtt_switch](/ha_mqtt_switch) : a simple example to control a switch connected to a nodeMCU board (esp8266)
- [ha_mqtt_sensor_dht22](/ha_mqtt_sensor_dht22) : a simple example to get temperature and humidity every ten minutes from a DHT22 sensor connected to a nodeMCU board (esp8266)
- [ha_mqtt_sensor_photocell](/ha_mqtt_sensor_photocell) : a simple example to get the brightness (0 - 100%) of the room every ten minutes from a photocell connected to a nodeMCU board (esp8266)
- [ha_mqtt_binary_sensor_pir](/ha_mqtt_binary_sensor_pir) : A simple example to use a PIR motion sensor connected to a nodeMCU board (esp8266)


## MySensors
- HumTempPres : Humidity, temperature and pressure node

## Documentation
- [home-assistant.io](https://home-assistant.io)
- [mysensors.org](https://www.mysensors.org)
- [mqtt.org](http://mqtt.org)
