# lightcamera-openipc
Install OpenIPC on Light Camera (Lutec, Steinel)

## Introduction

This project try to install [OpenIPC](https://openipc.org/) on Light Camera. Any help will be usefull.

![Steinel L620](/ressource/steinel-l-620-cam-sc-small.jpg)

### Light Camera should be :
- Steinel L600
- [Steinel L620](/L620//steinel-l620-sc.md)
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







