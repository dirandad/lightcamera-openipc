#!/bin/sh
#
# Prepare the migration of a L620 / L620 SC from OpenIPC 2.3 (2023, fixed
# partitions) to the current OpenIPC (split NOR layout), without opening the
# camera. Run ON THE CAMERA, as root, after copying to the SD card:
#   <sd>/openipc-fw.bin   firmware.bin of openipc.hi3518ev300-nor-ultimate-split.tgz
#   <sd>/autoconfig.sh, <sd>/autoconfig/   from scripts/sd-openipc
#
# Usage: sh prepare.sh <sd mount point> <crc32 of openipc-fw.bin, 8 hex digits>
#
# What it does:
# - copies the settings that must survive onto the SD card, for autoconfig.sh
#   at the first boot of the new firmware: Wi-Fi (wifi.conf, from
#   /etc/network/interfaces), lightcam.yml, SSH authorized_keys;
# - sets U-Boot variables so that, at the NEXT boot, U-Boot itself writes the
#   new firmware from the card (no running system, no RAM limit):
#     mig=1, mig_crc, mig_run, and a new bootcmd.
#   U-Boot loads openipc-fw.bin and checks its CRC32 BEFORE erasing anything. If
#   the file is missing or wrong, mig is set to 0 and the old system boots.
#   After writing, it reads the flash back and checks the CRC32 again; if it
#   differs (or the power fails), mig stays 1 and the next boot writes again.
# It does not reboot: reboot the camera to start the migration.
#
# The old 2023 U-Boot is kept. The new kernel finds its partitions by itself and
# ignores the mtdparts of bootargs. bootcmd reads 4 MB of kernel.

set -u
SD=${1:?usage: prepare.sh <sd mount point> <crc32>}
CRC=${2:?usage: prepare.sh <sd mount point> <crc32>}

die() { echo "ERROR: $*"; exit 1; }

[ -f "$SD/openipc-fw.bin" ] || die "$SD/openipc-fw.bin not found"
[ -f "$SD/autoconfig.sh" ] || die "$SD/autoconfig.sh not found"
[ -d "$SD/autoconfig" ] || die "$SD/autoconfig/ not found"
echo "$CRC" | grep -q -E '^[0-9a-fA-F]{8}$' || die "CRC must be 8 hex digits"
SIZE=$(ls -l "$SD/openipc-fw.bin" | awk '{print $5}')
[ "$SIZE" -gt 4000000 ] && [ "$SIZE" -le $((0xfb0000)) ] || die "openipc-fw.bin size $SIZE is not plausible"
[ "$(dd if="$SD/openipc-fw.bin" bs=4 count=1 2>/dev/null | hexdump -v -e '4/1 "%02x"')" = "27051956" ] ||
	die "openipc-fw.bin does not start with a uImage"

# --- settings for autoconfig.sh ---
IF=/etc/network/interfaces
SSID=$(sed -n 's/.*wpa_passphrase "\([^"]*\)" "\([^"]*\)".*/\1/p' $IF | head -1)
PASS=$(sed -n 's/.*wpa_passphrase "\([^"]*\)" "\([^"]*\)".*/\2/p' $IF | head -1)
[ -n "$SSID" ] && [ -n "$PASS" ] || die "Wi-Fi settings not found in $IF"
printf 'wlanssid=%s\nwlanpass=%s\nwlandev=rtl8188fu-generic\n' "$SSID" "$PASS" > "$SD/wifi.conf"
echo "wifi.conf written for network '$SSID'"

if [ -f /etc/lightcam.yml ]; then
	mkdir -p "$SD/autoconfig/etc"
	cp /etc/lightcam.yml "$SD/autoconfig/etc/lightcam.yml" && echo "lightcam.yml copied"
fi
# Light settings saved by lightcam (light mode, PIR, lux, levels, durations).
# Without them lightcam starts with its defaults and sends them to the light
# card, which changes the settings of the lamp.
if [ -f /var/lib/lightcam ]; then
	mkdir -p "$SD/autoconfig/var/lib"
	cp /var/lib/lightcam "$SD/autoconfig/var/lib/lightcam" && echo "light settings (/var/lib/lightcam) copied"
fi

# Host name: lightcam used it as MQTT client ID, and all migrated cameras would
# otherwise get the same default name.
if [ -s /etc/hostname ]; then
	mkdir -p "$SD/autoconfig/etc"
	cp /etc/hostname "$SD/autoconfig/etc/hostname" && echo "host name '$(cat /etc/hostname)' copied"
fi
for k in /root/.ssh/authorized_keys /etc/dropbear/authorized_keys; do
	if [ -f $k ]; then
		mkdir -p "$SD/autoconfig/root/.ssh"
		cp $k "$SD/autoconfig/root/.ssh/authorized_keys" && echo "SSH key copied from $k"
		break
	fi
done
sync

# --- U-Boot variables ---
# crc32 in U-Boot stores the CRC big-endian; itest.l reads it as a little-endian
# word: compare with the byte-swapped value.
C=$(echo "$CRC" | tr 'A-F' 'a-f')
SWAP=0x$(echo "$C" | cut -c7-8)$(echo "$C" | cut -c5-6)$(echo "$C" | cut -c3-4)$(echo "$C" | cut -c1-2)

OLDBC="$(fw_printenv -n bootcmd)"
fw_printenv -n bootcmd_2023 >/dev/null 2>&1 || fw_setenv bootcmd_2023 "$OLDBC"

fw_setenv mig_crc "$SWAP"
fw_setenv mig_run 'mmc rescan; if fatload mmc 0 0x42000000 openipc-fw.bin && crc32 0x42000000 ${filesize} 0x41000000 && itest.l *0x41000000 == ${mig_crc}; then echo "migration: writing"; sf probe 0; sf erase 0x50000 0xfb0000; sf write 0x42000000 0x50000 ${filesize}; mw.b 0x42000000 0xff ${filesize}; sf read 0x42000000 0x50000 ${filesize}; crc32 0x42000000 ${filesize} 0x41000000; if itest.l *0x41000000 == ${mig_crc}; then echo "migration: done"; setenv mig 0; saveenv; else echo "migration: verify failed, again"; reset; fi; else echo "migration: no valid openipc-fw.bin, old system kept"; setenv mig 0; saveenv; fi'
fw_setenv bootcmd 'if itest ${mig} == 1; then run mig_run; fi; setenv setargs setenv bootargs ${bootargs}; run setargs; sf probe 0; sf read 0x42000000 0x50000 0x400000; bootm 0x42000000'
fw_setenv mig 1

echo "U-Boot:"
fw_printenv mig mig_crc bootcmd
echo "Ready: reboot the camera to migrate (crc32 $C, $SIZE bytes)."
