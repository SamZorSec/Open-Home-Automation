# Open Home Automation with Home-Assistant

## MQTT
A MQTT broker is mandatory. More information can be found at the [MQTT component's page](https://home-assistant.io/components/mqtt/)

configuration.yaml :
```yaml
mqtt:
  broker: [Redacted]
```
- [ha_mqtt_light](https://github.com/mertenats/open-home-automation/tree/master/ha_mqtt_light) : a simple example to control the built-in led of the nodeMCU board (esp8266)
- [ha_mqtt_rgb_light](https://github.com/mertenats/open-home-automation/tree/master/ha_mqtt_rgb_light) : a simple example to control a rgb led connected to a nodeMCU board (esp8266)

## MySensors
- HumTempPres : Humidity, temperature and pressure node

## Documentation
- [home-assistant.io](https://home-assistant.io)
- [mysensors.org](https://www.mysensors.org)
- [mqtt.org](http://mqtt.org)
