# Configuration - Zigate and ZigBee devices - Home Assistant
A simple configuration example to add and control ZigBee devices paired to a [Zigate](https://zigate.fr), an open source ZigBee concentrator.

## Requirements
### Software
Using the Zigate with Home Assistant requires to install `pyzigate` ([GitHub](https://github.com/elric91/ZiGate)) and copy the `homeassistant_zigate` component within the `custom_components` in Home Assistant's configuration folder ([GitHub](https://github.com/elric91/homeassistant_zigate)).

### Hardware
- 1x [Zigate](https://zigate.fr)
- 2x Philips white bulbs
- 1x Xiaomi Aqara switch
- 1x Xiaomi Aqara door sensor
- 1x Xiaomi Aqara temperature/humidity/pressure sensor

## Configuration
configuration.yaml :
```yaml
homeassistant:

...

switch: !include switches.yaml
light: !include lights.yaml
sensor: !include sensors.yaml
automation: !include automations.yaml
```

Configuration for the Xiaomi Aqara door sensor :

switches.yaml :
```yaml
- platform: zigate
  name: 'Door'
  address: e49d01
  default_state: 'state'
  inverted: 'yes'
```

Configuration for the two white Philips Hue bulbs :

lights.yaml :
```yaml
- platform: zigate
  name: 'Bedside'
  address: 64880b
  light_type: 'white'
  default_state: 'event'
- platform: zigate
  name: 'Ceiling'
  address: 1e4c0b
  light_type: 'white'
  default_state: 'event'
```

Configuration for the Xiaomi Aqara temperature/humidity/pressure sensor and switch :

sensors.yaml :
```yaml
- platform: zigate
  name: 'Ceiling Switch'
  address: XXXX01
  default_state: 'state'
- platform: zigate
  name: 'Temperature'
  address: XXXX01
  default_state: temperature
  default_unit: '°C'
- platform: zigate
  name: 'Humidity'
  address: XXXX01
  default_state: humidity
  default_unit: '%'
- platform: zigate
  name: 'Pressure'
  address: XXXX01
  default_state: pressure
  default_unit: 'mb'
```

Automation example to handle a single/double click on the Xiaomi Aqara switch :

automations.yaml :
```yaml
- alias: 'Ceiling Switch - Single Click'
  hide_entity: True
  trigger:
    entity_id: sensor.ceiling_switch
    platform: state
    to: 'off-release'
  action:
    service: light.toggle
    data:
      entity_id: light.ceiling
- alias: 'Ceiling Switch - Double Click'
  hide_entity: True
  trigger:
    entity_id: sensor.ceiling_switch
    platform: state
    to: 'multi_2'
  action:
    service: light.toggle
    data:
      entity_id: light.bedside
```

## References
- [Instructions pour les ampoules Philips Hue](http://zigate.fr/837-2/)
- [Configuration examples](https://github.com/elric91/homeassistant_zigate/wiki/Configuration-examples)
