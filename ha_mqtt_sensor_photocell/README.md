# ha_mqtt_sensor_photocell - Home-Assistant
A simple example to get the brightness (0 - 100%) of the room every ten minutes from a photocell connected to a nodeMCU board (esp8266)

## Configuration
configuration.yaml :
```yaml
sensor 1:
  platform: mqtt
  state_topic: 'office/sensor1'
  name: 'Brightness'
  unit_of_measurement: '%'
  value_template: '{{ value_json.brightness }}'
```