# lightcam-application

Embedded application developped in c to expose/manage light extension card.

## Introduction

The application have the following functionnalities:

- Manage YAML file
  - configuration for MQTT server
  - configuration for Serial port

- Manage Serial communication with extention card
  - Send configuration using Bytes array

- Manage register configuration
  - Read and Write GPIO (Read Value, Write Value, Write Direction)
  - Configure register values

- Manage Leds
  - Switch On and Off Led with application Status

- Manage Alarm (detector) status
  - Retrive detector status

- Integrate a MQTT Client
  - Send Watchdog to MQTT Server
  - Receive Light Configuration from MQTT Server and send it to serial
  - Send detector status to MQTT Server

## Prepare developpment environement

The development environement is a cross compilation environement. In this case, it is based on:
- Ubuntu 22.04 WSL on Windows 11
- Toolchain for OpenIPC to be able to compile c code for OpenIPC

```
$ wget https://github.com/OpenIPC/firmware/releases/download/latest/cortex_a7_thumb2-gcc8.4.0-musl-4_9-1e913a17.tgz

$ tar zxvf cortex_a7_thumb2-gcc8.4.0-musl-4_9-1e913a17.tgz

$ cd arm-openipc-linux-musleabi_sdk-buildroot/

$ ./relocate-sdk.sh
```

## Build application

Application can be compiled using the following instructions

```
$ git clone https://github.com/dirandad/lightcamera-openipc/

$ cd ~/lightcamera-openipc/lightcam-application/src/

$ chmod +x build_lightcam.sh
```

Edit `build_lightcam.sh` and set variables

```
TOOLCHAIN_PATH="/arm-openipc-linux-musleabi_sdk-buildroot/bin/"
CAMIP="192.168.1.1"
```

Then build the application with

```
$ ./build_lightcam.sh
```

## Installation

lightcam can be installed on a fresh OpenIPC camera by copiing the following files:
- `lightcam` 		--> `/usr/sbin/`
- `S50lightcam`		--> `/etc/init.d/`
- `lightcam.yml`	--> `/etc/`

## Usage

To start lightcam manually enter the following command (adapt configuration file path if needed):

```
/usr/sbin/lightcam /etc/lightcam.yml
```

To manage lightcam using `init.d`:

```
/etc/init.d/S50lightcam start
/etc/init.d/S50lightcam stop
/etc/init.d/S50lightcam restart
```

### Configuration

Basic configuration of lightcam is done using a YAML file and contain:
- MQTT server configuration including detector temporisation
- serial configuration

```
mqtt: 
    server: "192.168.1.1"
    port: 1883
    username: "username"
    password: "password"
    topic: "cameraip"
    detectortempo: 60.0
serial: 
    port: "/dev/ttyAMA0"
    baudrate: 9600
```

### MQTT topics

in this section, `{basetopic}` is configured in YAML configuration file at `mqtt.topic`.

lightcam will subscribe on the following MQTT topics:

```
{basetopic}/Set/Update
{basetopic}/Set/LightMode={on,detect,config}
{basetopic}/Set/AlarmMode={on,off}
{basetopic}/Set/PirSensibility={1-25}
{basetopic}/Set/LuxSensibility={1-23}
{basetopic}/Set/HighLightLevel={1-19}
{basetopic}/Set/OnTemporisation={1,3,10,15}
{basetopic}/Set/LowLightLevel={1-12}
{basetopic}/Set/LowLightDuration={-1,2,4,6,10}
```

lightcam will publish on the following MQTT topics:

```
// updated each 5 sec
{basetopic}/Watchdog={0-32000}

// set to 1 when GPIO 1_5 activated, reset to 0 at end of temporisation mqtt.detectortempo after last activation of GPIO 1_5	
{basetopic}/Get/Detection={0,1}

// updated each 60 sec or after reception of new message on a subscribed topic.
{basetopic}/Get/LightMode={on,detect,config}
{basetopic}/Get/AlarmMode={on,off}
{basetopic}/Get/PirSensibility={1-25}
{basetopic}/Get/LuxSensibility={1-23}
{basetopic}/Get/HighLightLevel={1-19}
{basetopic}/Get/OnTemporisation={1,3,10,15}
{basetopic}/Get/LowLightLevel={1-12}
{basetopic}/Get/LowLightDuration={-1,2,4,6,10}
```

## Integration

### Home Assitant Integration

Based on MQTT topics, here is an example of integration in Home Assistant.
As prerequiste, you need to add the MQTT integration to Home Assistant.

![HA](/lightcam-application/ha-integration.png)

extract of `configuration.yaml` file. Replace `camera` with your specific name.
```
mqtt:
    sensor:  
      - unique_id: camera_watchdog
        name: "camera_watchdog"
        state_topic: "camera/Watchdog"
        expire_after: 10
    binary_sensor:
      - unique_id: camera_detection
        name: "camera_detection"
        state_topic: "camera/Get/Detection"
        payload_on: "1"
        payload_off: "0"
    light:
      - unique_id: camera_light
        name: "camera_light"
        schema: template
        command_topic: "camera/Set/LightMode"
        command_on_template: "on"
        command_off_template: "detect"
        state_topic: "camera/Get/LightMode"
        state_template: "{% if value=='on' %}on{% else %}off{% endif %}"
    button:
      - unique_id: camera_update
        name: "camera_update"
        command_topic: "camera/Set/Update"
    switch:
      - unique_id: camera_alarmmode
        name: "camera_alarmmode"
        command_topic: "camera/Set/AlarmMode"
        state_topic: "camera/Get/AlarmMode"
        payload_on: "on"
        payload_off: "off"
    select:
      - unique_id: camera_ontemporisation
        name: "camera_ontemporisation"
        command_topic: "camera/Set/OnTemporisation"
        options:
          - "1"
          - "3"
          - "10"
          - "15"
      - unique_id: camera_lowlightduration
        name: "camera_lowlightduration"
        command_topic: "camera/Set/LowLightDuration"
        options:
          - "-1"
          - "2"
          - "4"
          - "6"
          - "10"
    number:
      - unique_id: camera_pirsensibility
        name: "camera_pirsensibility"
        command_topic: "camera/Set/PirSensibility"
        state_topic: "camera/Get/PirSensibility"
        min: 1
        max: 25
        step: 1
      - unique_id: camera_luxsensibility
        name: "camera_luxsensibility"
        command_topic: "camera/Set/LuxSensibility"
        state_topic: "camera/Get/LuxSensibility"
        min: 1
        max: 23
        step: 1
      - unique_id: camera_highlightlevel
        name: "camera_highlightlevel"
        command_topic: "camera/Set/HighLightLevel"
        state_topic: "camera/Get/HighLightLevel"
        min: 1
        max: 19
        step: 1
      - unique_id: camera_lowlightlevel
        name: "camera_lowlightlevel"
        command_topic: "camera/Set/LowLightLevel"
        state_topic: "camera/Get/LowLightLevel"
        min: 1
        max: 12
        step: 1

```

## Resources and Reference

LighCam use the following source :
- Embedded C MQTT Client: [Paho MQTT Client](https://www.eclipse.org/paho/index.php?page=clients/c/embedded/index.php)
- YAML code : [Yaml code](https://github.com/tlsa/libcyaml)



