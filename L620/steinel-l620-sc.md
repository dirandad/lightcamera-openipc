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



## original firmware analysis

### printenv

```
hisilicon # printenv
NID=0x0001
appCloudExAbility=toaXOp2PN2M=
appSystemLanguage=English
appVideoStandard=PAL
baudrate=115200
bootargs=init=linuxrc mem=${osmem} console=ttyAMA0,115200 root=/dev/mtdblock1 rootfstype=squashfs mtdparts=hi_sfc:0x40000(boot),0x540000(romfs),0x740000(user),0x180000(web),0x80000(custom),0x140000(mtd)
bootcmd=setenv setargs setenv bootargs ${bootargs};run setargs;sf probe 0;sf read 43000000 40000 550000;squashfsload;bootm 0x42000000
bootdelay=0
bootfile="uImage"
da=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile u-boot.bin.img;fatload mmc 0;sf probe 0;flwrite
dc=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile custom-x.cramfs.img;fatload mmc 0;sf probe 0;flwrite
dd=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile mtd-x.jffs2.img;fatload mmc 0;sf probe 0;flwrite
de=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile u-boot.env.img;fatload mmc 0;sf probe 0;flwrite
dl=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile logo-x.cramfs.img;fatload mmc 0;sf probe 0;flwrite
dr=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile romfs-x.cramfs.img;fatload mmc 0;sf probe 0;flwrite
du=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile user-x.cramfs.img;fatload mmc 0;sf probe 0;flwrite
dw=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile web-x.cramfs.img;fatload mmc 0;sf probe 0;flwrite
ethact=eth0
ethaddr=00:12:31:88:dc:38
gatewayip=192.168.1.1
gpio0=0x14
gpio1=0x16
gpio2=0x17
gpio3=0x3
gpioval0=0x0
gpioval1=0x1
gpioval2=0x0
gpioval3=0x0
ipaddr=192.168.1.10
muxctl0=0x120c0010
muxctl1=0x120c0018
muxctl2=0x120c001c
muxctl3=0x100c000c
muxval0=0x1e02
muxval1=0x1d02
muxval2=0x1502
muxval3=0x1000
netmask=255.255.0.0
osmem=43M
serverip=192.168.1.107
stderr=serial
stdin=serial
stdout=serial
tk=setenv loadaddr 0x42000000;setenv bootfile uImage;fatload mmc 0;setenv setargs setenv bootargs ${bootargs};run setargs;bootm 0x42000000
ua=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile upall_verify.img;fatload mmc 0;sf probe 0;flwrite
up=mw.b 0x42000000 ff 1000000;setenv loadaddr 0x42000000;setenv bootfile update.img;fatload mmc 0;sf probe 0;flwrite
ver=U-Boot 2016.11 (Oct 29 2018 - 16:06:3
verify=n

Environment size: 2390/65532 bytes
```



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

the communication bettween the light managment card and the camera proc is serial using console serial port.

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
