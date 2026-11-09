# SC2235P sensor driver for the Steinel L620 (non-SC)

The Steinel **L620** (not the L620 SC) uses a SmartSens **SC2235P** sensor. OpenIPC detects it (`fw_printenv sensor` gives `sensor=sc2235p`) but does not ship a driver for it: without these files, majestic stops with

```
No matches for 'sc2235p' in /etc/sensors directory
Cannot start SDK
```

and there is no image (issues #17 and #19). The `libsns_sc2235.so` driver shipped with OpenIPC is for another sensor and does not work.

The L620 SC uses a SC2315E sensor, which is supported by OpenIPC: it does not need these files.

## Files

| File | Install to | MD5 |
|---|---|---|
| `libsns_sc2235p.so` | `/usr/lib/sensors/libsns_sc2235p.so` | `627acdadb84f8c61e6968add25a5695b` |
| `sc2235p_i2c_dc_ad.ini` | `/etc/sensors/sc2235p_i2c_dc_ad.ini` | `82e1c43e6326f9e50713b033e5bc213a` |

- `libsns_sc2235p.so` was built in April 2023 with the OpenIPC toolchain (`arm-openipc-linux-musleabi`, GCC 8.4.0), for the HiSilicon MPP `Hi3516EV200_MPP_V1.0.1.2` API (`HI_MPI_ISP/AE/AWB_SensorRegCallBack`). It exports `stSnsSc2235pObj`. The register initialisation was written from I2C dumps of the sensor running under the Steinel firmware.
- `sc2235p_i2c_dc_ad.ini`: MIPI, 10-bit RAW, 27 MHz, `lane_id = 0|2`, BGGR, 1920x1080 at 25 fps. Majestic selects the ini whose name contains the sensor name, so keep `sc2235p` in the file name.

## Sources

The `src/` folder contains the sources of `libsns_sc2235p.so`:

| File | Content |
|---|---|
| `sc2235p_sensor_ctl.c` | **The actual change**: the sensor register initialisation, rewritten from I2C dumps of the SC2235P under the Steinel firmware. The software reset (`0x0103`) is disabled. |
| `sc2235p_cmos.c`, `sc2235p_cmos_ex.h` | Identical to the `smart_sc2235` driver of OpenIPC/openhisilicon (AE, AWB, ISP callbacks), with `sc2235` renamed to `sc2235p` so that the driver exports `stSnsSc2235pObj` |
| `Makefile` | Same as `smart_sc2235`, builds `libsns_sc2235p.so` and `libsns_sc2235p.a` |

They are derived from [OpenIPC/openhisilicon](https://github.com/OpenIPC/openhisilicon) (`libraries/sensor/hi3516ev200/smart_sc2235`), distributed under the GPL-3.0 license of that repository. The files keep their original headers.

The binary in this folder was built on 2023-04-23 from these sources, in a copy of openhisilicon (`libraries/sensor/hi3516ev200/smart_sc2235p/`), with the OpenIPC toolchain (`arm-openipc-linux-musleabi`, GCC 8.4.0). The binary found next to these sources in the 2023 build environment has the same MD5 as `libsns_sc2235p.so` above.

To rebuild it, copy `src/` to `libraries/sensor/hi3516ev200/smart_sc2235p/` in an openhisilicon checkout and build it the same way as the other hi3516ev200 sensor drivers, with the OpenIPC toolchain for hi3518ev300.

## Tested on

| Camera | OpenIPC | Result |
|---|---|---|
| L620, SC2235P | 2.3.04.14 (April 2023) | image OK, in use since April 2023 |
| L620, SC2235P | 2.6.11.06 (November 2026, split NOR layout) | `HiSilicon SDK started`, H.265 1920x1080 at 25 fps, no VENC timeout, snapshot OK |

## Installation

From a PC on the same network (replace `<camera-ip>`), in the folder that contains the two files:

```
ssh root@<camera-ip> 'cat > /usr/lib/sensors/libsns_sc2235p.so' < libsns_sc2235p.so
ssh root@<camera-ip> 'cat > /etc/sensors/sc2235p_i2c_dc_ad.ini' < sc2235p_i2c_dc_ad.ini
ssh root@<camera-ip> 'chmod 644 /usr/lib/sensors/libsns_sc2235p.so /etc/sensors/sc2235p_i2c_dc_ad.ini && /etc/init.d/S95majestic restart'
```

(`scp` also works, but older dropbear versions on the camera lack the SFTP server used by recent `scp` clients.)

The files are written in the overlay (`rootfs_data`): they survive reboots and `sysupgrade`, but not `sysupgrade -n` or a factory reset.

## Check

On the camera:

```
fw_printenv sensor                         # sensor=sc2235p
logread | grep -i -E "sc2235p|SDK|venc"     # expect "HiSilicon SDK started"
```

Then open the web interface or the RTSP stream (`rtsp://<camera-ip>/stream=0`).

If `sensor` is not `sc2235p`, set it with `fw_setenv sensor sc2235p` and reboot.
