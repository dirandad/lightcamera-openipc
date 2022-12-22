# lightcamera-openipc
Install OpenIPC on Light Camera (Lutec, Steinel)

## Introduction

This project try to install [OpenIPC](https://openipc.org/) on Light Camera. Any help will be usefull.

### Light Camera should be :
- Steinel L600
- Steinel L620
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
    - [ ] Manage detector parameters




