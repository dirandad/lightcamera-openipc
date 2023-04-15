# lightcamera-openipc
Install OpenIPC on Light Camera (Lutec, Steinel)

## Introduction

This project try to install [OpenIPC](https://openipc.org/) on Light Camera. Any help will be usefull.

![Steinel L620](/ressource/steinel-l-620-cam-sc-small.jpg)

### Light Camera should be :
- Steinel L600
- [Steinel L620 (SC)](/L620/README.md)
- Steinel XLED Cam 1
- Lutec Elara

These cameras are tipically based on :
- Hisilicon SoC : HI3518EV200 or HI3518EV300
- A light extension card base on microship (GX20-2120)
- Wifi based camera (no RJ45 port)
- Sensor
- Originial firmware is cloud base, using smartphone application. Camera cannot be used in local only




### Objectives of the project:
- [ ] Install [OpenIPC](https://openipc.org/) on the camera (Open source IP Camera firmware that give all local access to the camera). OpenIPC installation should be SD Card based as dismounting of the camera can be hard, and sometime definitive. OpenIPC include a lot of IP camera functionalities.
- [ ] Active Wifi Card and networking
- [ ] Active Sensor
- Manage communication with light extension card over webservice
    - [ ] Expose detector status
    - [ ] give light control : ON/OFF/Variable
    - [ ] manage light parameters : ON/OFF management, Light %,...
    - [ ] Manage detector parameters : sensibility, duration,...

Project will be started with a Steinel L620 SC camera.

### Steps for each camera type

- Camera dismounting and hardware identification
- identification of console serial port and connection
- original firmware analysis
- light management extension card analysis
- OpenIPC installation using console
- Wi-Fi card activation
- sensor activation
- module development for light extension card
- module integration in OpenIPC
- script development to automate firmware installation using SD card

## OpenIPC

## Introduction

[OpenIPC](https://openipc.org/) is an Open Source firmware for IPC Camera. It is Linux Based and can enter in SoC with at least 8MB Flash.
Two versions are availlable in standards:
- Lite : for 8MB system, contain the minimum of functions for small systems
- Ultimate : need at least 16MB system. Utlimate version contain more functions and drivers (wifi drivers,...)

It is still possible to build a custom firmware with selected modules and options.

## Comptability

OpenIPC is compatibles with some SoC, but not all. [Compatible Hardware](https://openipc.org/supported-hardware/featured) are presented on OpenIPC website.
Hisilicom SoC are highly compatible.

## Installation

There is several way to install OpenIPC. Here is the list of the more current ways:
- Using Serial port and u-boot. u-boot have various commands to read and write firmware to flash. firmware be load from following sources:
    - SD Card: if the SoC has SD Card slot
    - USB Key: if the SoC has USB port
    - Remote TFTP Server: uf the SoC has ethernet port
    - Serial communication
- Using original firmware update software. This is brand specific. OpenIPC provide a firmware package of this usage. See [Coupler](https://github.com/OpenIPC/coupler). This method don't need to disassemble the camera.
- Using [ipctool](https://github.com/OpenIPC/ipctool) upgrade function and a specific firmware package. This method don't need to disassemble the camera.
- Using a programmer and a CH341a tools directly connected on flash component. This the last chance technic as it can used event when the camera does not start. OpenIPC provide a specific firmware package for programmer.


