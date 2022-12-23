# Steinel L620 SC Camera

![Steinel L620](../ressource/steinel-l-620-cam-sc-small.jpg)

## Camera dismounting and hardware identification

Dismouting of Steinel Cameras is quite hard, and seems to don't be faisible without breaking a foot of the camera support. This confirm the need of developping a script to install a new firmware without need to open the camera. All connectors are stick (stick can be removed, sometime with breaking a connector...).

### Steinel L620 SC camera is based on this hardware
- SoC : Hisilicon HI3518EV300
- Device ID : 50H20L
- Sensor : ??
- Wifi Card : Realtek RTL8188FU (to be confirmed)
- Light managment card : see above

### Wiring diagram of the light

![Steinel L620 Wiring Diagram](/L620/ressources/steinel-l-620-sc-wirring-diagram.png)

## identification of console serial port and connection
Work in progress

## original firmware analysis

firmware analyzed : General_CamLight_IPC_HI3518EV300_50H20L_8188FU_V4.03.R12.Nat.20200217_ALL.bin

### extracting firmware

extracting firmware using binwalk (could be needed to install additionnal extracting tools. Check binwalk output)

```
binwalk -e General_CamLight_IPC_HI3518EV300_50H20L_8188FU_V4.03.R12.Nat.20200217_ALL.bin
cd _General_CamLight_IPC_HI3518EV300_50H20L_8188FU_V4.03.R12.Nat.20200217_ALL.bin.extracted
binwalk -e romfs-x.cramfs.img
binwalk -e user-x.cramfs.img
binwalk -e custom-x.cramfs.img
binwalk -e web-x.cramfs.img
```



Work in progress

## light management extension card analysis

- Light managment card :
  - 2 PIR Sensors
  - 1 Light Sensor
  - 1 Led
  - Based on microchip GX20-2120 (SSOP-20 format) ref U2

pinout of the U2 microchip :

![Steinel L620 U2 pineout](/L620/ressources/steinel-l-620-sc-U2-pinout.png)

the communication bettween the light managment card and the camera proc is serial (UART ?)

## OpenIPC installation using console
Work in progress

## Wi-Fi card activation
Work in progress

## sensor activation
Work in progress

## module development for light extension card
Work in progress

## module integration in OpenIPC
Work in progress

## script development to automate firmware installation using SD card
