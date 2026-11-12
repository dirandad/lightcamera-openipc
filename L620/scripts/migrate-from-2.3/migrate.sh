#!/bin/bash
#
# Migrate a Steinel L620 / L620 SC running OpenIPC 2.3 (2023, fixed partitions)
# to the current OpenIPC, from a PC, through SSH and the SD card of the camera.
# Tested on 2026-11-11 on a copy of a camera installed in April 2023 (OpenIPC
# 2.3.04.14, 2 MB kernel, U-Boot of March 2023), migrated to nightly-20261109.
#
# Usage: [SSH_OPTS="-i <key>"] ./migrate.sh <camera-ip> <firmware.bin.hi3518ev300>
#   firmware.bin.hi3518ev300: from openipc.hi3518ev300-nor-ultimate-split.tgz
#
# Needs on the PC: bash, ssh with a key accepted by the camera, gzip, md5sum.
# Needs on the camera: an SD card (FAT32), with at least 20 MB free.
#
# The script prepares everything and does NOT reboot. At the next reboot,
# U-Boot writes the firmware from the card (see prepare.sh), then the new
# firmware configures itself from the card (autoconfig.sh): Wi-Fi, host name,
# SSH key, sensor driver, lightcam and its configuration.
#
# Before running it, back up the flash of the camera:
#   ssh root@<ip> 'cat /dev/mtd0 /dev/mtd1 /dev/mtd2 /dev/mtd3 /dev/mtd4' > flash-<name>.bin

set -euo pipefail

IP=${1:?usage: migrate.sh <camera-ip> <firmware.bin>}
FW=${2:?usage: migrate.sh <camera-ip> <firmware.bin>}
HERE=$(cd "$(dirname "$0")" && pwd)
SDOPENIPC="$HERE/../sd-openipc"
# Extra ssh options, for example SSH_OPTS="-i ~/.ssh/mykey"
# shellcheck disable=SC2206
SSH=(ssh -o BatchMode=yes -o ConnectTimeout=10 ${SSH_OPTS:-} root@"$IP")

[ -f "$FW" ] || { echo "no file $FW"; exit 1; }
[ "$(head -c 4 "$FW" | od -A n -t x1 | tr -d ' ')" = "27051956" ] || { echo "$FW is not a firmware.bin (no uImage)"; exit 1; }

# CRC32 of the firmware: the last 8 bytes of a gzip stream are the CRC32
# (little-endian) and the size of the input.
CRC=$(gzip -c "$FW" | tail -c 8 | head -c 4 | od -A n -t x4 | tr -d ' ')
MD5=$(md5sum "$FW" | cut -c1-32)
echo "firmware: $FW, $(stat -c %s "$FW") bytes, crc32 $CRC, md5 $MD5"

echo "== camera"
"${SSH[@]}" 'cat /etc/hostname; grep OPENIPC_VERSION /etc/os-release; cat /proc/mtd | tail -n +2 | tr "\n" " "; echo'

echo "== SD card"
SD=$("${SSH[@]}" 'S=$(mount | sed -n "s|^/dev/mmcblk0[^ ]* on \([^ ]*\) .*|\1|p" | head -1)
if [ -z "$S" ]; then
	S=/mnt/sd; mkdir -p $S
	d=/dev/mmcblk0p1; [ -b $d ] || d=/dev/mmcblk0
	mount -t vfat -o rw,noatime $d $S || exit 1
fi
echo $S')
echo "SD card mounted on $SD"
"${SSH[@]}" "df -h $SD | tail -1; rm -rf $SD/autoconfig $SD/autoconfig.done $SD/autoconfig.sh $SD/wifi.conf"

echo "== copying the firmware and autoconfig/ to the card"
"${SSH[@]}" "cat > $SD/openipc-fw.bin" < "$FW"
tar -C "$SDOPENIPC" -cf - autoconfig.sh autoconfig | "${SSH[@]}" "tar -C $SD -xf -"
"${SSH[@]}" "cat > /tmp/prepare.sh" < "$HERE/prepare.sh"
GOT=$("${SSH[@]}" "md5sum $SD/openipc-fw.bin | cut -c1-32")
[ "$GOT" = "$MD5" ] || { echo "md5 on the card $GOT differs from $MD5: stop"; exit 1; }
echo "firmware on the card: md5 OK"

echo "== preparing (settings to the card, U-Boot variables)"
"${SSH[@]}" "sh /tmp/prepare.sh $SD $CRC; sync"

cat <<EOF

Ready. To migrate, reboot the camera:  ssh root@$IP reboot
Then, after 2 to 3 minutes:
  - open http://$IP/ and set the root password (and paste your SSH public key);
  - reboot the camera once more (CMA memory allocator and console=null apply
    at the next boot);
  - check: cat /mnt/mmcblk0*/autoconfig.log
If something went wrong before the flash was erased, the old system boots
again (mig=0). To cancel before rebooting:  ssh root@$IP fw_setenv mig 0
EOF
