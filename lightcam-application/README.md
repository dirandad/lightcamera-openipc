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
## Usage

### Configuration



### MQTT topics

	// Watchdog : 
	//			{basetopic}/Watchdog={0-32000}
	
	// Get Topics : 
	//			{basetopic}/Get/Detection={0,1}
	
	// Set Topics :
	//			{basetopic}/Set/Update
	//			{basetopic}/Set/LightMode={on,detect,config}
	//			{basetopic}/Set/AlarmMode={on,off}
	//			{basetopic}/Set/PirSensibility={1-25}
	//			{basetopic}/Set/LuxSensibility={1-23}
	//			{basetopic}/Set/HighLightLevel={1-19}
	//			{basetopic}/Set/OnTemporisation={1,3,10,15}
	//			{basetopic}/Set/LowLightLevel={1-12}
	//			{basetopic}/Set/LowLightDuration={-1,2,4,6,10}


## Resources and Reference

LighCam use the following source :
- Embedded C MQTT Client: [Paho MQTT Client](https://www.eclipse.org/paho/index.php?page=clients/c/embedded/index.php)
- YAML code : [Yaml code](https://github.com/tlsa/libcyaml)



