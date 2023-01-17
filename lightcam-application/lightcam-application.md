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

## Application Structure



## Resources and Reference

LighCam use the following source :
- Embedded C MQTT Client: [Paho MQTT Client](https://www.eclipse.org/paho/index.php?page=clients/c/embedded/index.php)
- YAML code : [Yaml code](https://github.com/tlsa/libcyaml)



