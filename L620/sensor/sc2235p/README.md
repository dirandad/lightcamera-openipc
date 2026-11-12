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

### Rebuild

The build is reproducible: rebuilding `src/` as below gives a `libsns_sc2235p.so` with the same MD5 as the binary in this folder (`627acdadb84f8c61e6968add25a5695b`, checked on 2026-11-09).

You need:
- a Linux machine (WSL works);
- the OpenIPC toolchain for hi3516ev200 / hi3518ev300 (`arm-openipc-linux-musleabi`, GCC 8.4.0, musl). The binary was built with the buildroot SDK of that toolchain;
- a checkout of [OpenIPC/openhisilicon](https://github.com/OpenIPC/openhisilicon), for the MPP headers in `include/`. The binary was built with commit `2922830cf7242225aa77df87d59fcd063754fafa` (2023-02-25).

```
git clone https://github.com/OpenIPC/openhisilicon
cd openhisilicon
git checkout 2922830cf7242225aa77df87d59fcd063754fafa
mkdir libraries/sensor/hi3516ev200/smart_sc2235p
cp <this repository>/L620/sensor/sc2235p/src/* libraries/sensor/hi3516ev200/smart_sc2235p/
cd libraries/sensor/hi3516ev200/smart_sc2235p
TC=<toolchain folder>/bin/arm-openipc-linux-musleabi-
make SDK_CODE=0x3516E200 ARCH=arm CC=${TC}gcc AR=${TC}ar
md5sum libsns_sc2235p.so
```

With another toolchain version or another openhisilicon commit, the binary is different but should work, as long as the toolchain uses musl and targets the same MPP version (`Hi3516EV200_MPP_V1.0.1.2`). This was not tested.

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
