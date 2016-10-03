# Home automation with open hardware and software


## Table of contents
1. [Introduction](#introduction)
2. [Automation](#automation)
3. [Parts list](#partslist)
4. [Installation of the controller](#installationofthecontroller)
5. [Implementation of the actuators/sensors](#implementationoftheactuators/sensors)
6. [Creation of the automation rules](#creationoftheautomationrules)
7. [Demonstration](#demonstration)


## Introduction
### Context
### Architecture

## Automation

## Parts list
### Hardware
### Software

## Installation of the controller
### Home Assistant
First, we need to update the system and install some Python dependencies.

```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install python-pip python3-dev
sudo pip install --upgrade virtualenv
```

Then, we create a new system user, `hass`, to execute the service for Home Assistant. This is a good practice to reduce the exposure of the rest of the system and provide a more granular control over permissions.

```
sudo adduser --system hass
```

We create a new directory for Home Assistant and we change its ownership to the new created user.

```
sudo mkdir /srv/hass
sudo chown hass /srv/hass
```
We become the new user and we set up a virtual Python environment in the directory we just created. A virtual Python environment is a good practice to avoid interaction with the Python packages used by the system or by others applications. Then, we activate it.

```
sudo su -s /bin/bash hass
virtualenv -p python3 /srv/hass
source /srv/hass/bin/activate
```
And now, we are ready to install Home Assistant.

```
pip3 install --upgrade homeassistant
```
Finally, we can run Home assitant by typing the command below.

```
sudo -u hass -H /srv/hass/bin/hass
```

For starting Home Assistant on boot, we need to create a service for `systemd`. Someone has already created one and we can just download it.

```
sudo wget https://raw.githubusercontent.com/home-assistant/home-assistant/master/script/home-assistant%40.service -O /etc/systemd/system/home-assistant@hass.service
```

This service needs a little modification. We have to replace `/usr/bin/hass`with `/srv/hass/bin/hass`. The line in question should look like this now `ExecStart=/srv/hass/bin/hass --runner`.

```
sudo nano /etc/systemd/system/home-assistant@hass.service
```

We need to reload `systemd`to make the daemon aware of the new configuration.

```
sudo systemctl --system daemon-reload
sudo systemctl enable home-assistant@hass
sudo systemctl start home-assistant@hass
```

To upgrade the system in the future, we just need to type the commands below.

```
sudo su -s /bin/bash hass
source /srv/hass/bin/activate
pip3 install --upgrade homeassistant
```

Sources

- [Installation in Virtualenv](https://home-assistant.io/getting-started/installation-virtualenv/)
- [Autostart Using Systemd](https://home-assistant.io/getting-started/autostart-systemd/)

### Mosquitto MQTT broker

### Homebridge

## Implementation of the actuators/sensors
### Entrance
### Living room
### Bedroom

## Creation of the automation rules
### Entrance
### Living room
### Bedroom
### Presence simulation

## Demonstration

