#!/bin/bash
#
# Replace the U-Boot of a migrated L620 / L620 SC by the current OpenIPC U-Boot,
# from a PC, through SSH and the SD card of the camera. Optional: the 2023
# U-Boot boots the current firmware (see migrate.sh).
#
# Usage: [SSH_OPTS="-i <key>"] ./update-uboot.sh <camera-ip> <u-boot-hi3518ev300-nor.bin>
#
# Do it on a camera where someone can switch the power off and on, and never
# switch the power off during the reboot that follows: if the ~2 s write of
# U-Boot is interrupted, the camera no longer starts.

set -euo pipefail

IP=${1:?usage: update-uboot.sh <camera-ip> <u-boot.bin>}
UB=${2:?usage: update-uboot.sh <camera-ip> <u-boot.bin>}
HERE=$(cd "$(dirname "$0")" && pwd)
# shellcheck disable=SC2206
SSH=(ssh -o BatchMode=yes -o ConnectTimeout=10 ${SSH_OPTS:-} root@"$IP")

[ -f "$UB" ] || { echo "no file $UB"; exit 1; }
CRC=$(gzip -c "$UB" | tail -c 8 | head -c 4 | od -A n -t x4 | tr -d ' ')
MD5=$(md5sum "$UB" | cut -c1-32)
echo "u-boot: $UB, $(stat -c %s "$UB") bytes, crc32 $CRC"

SD=$("${SSH[@]}" 'S=$(mount | sed -n "s|^/dev/mmcblk0[^ ]* on \([^ ]*\) .*|\1|p" | head -1); [ -n "$S" ] || exit 1; echo $S') ||
	{ echo "no SD card mounted on the camera"; exit 1; }
echo "SD card: $SD"
"${SSH[@]}" "cat > $SD/openipc-uboot.bin" < "$UB"
GOT=$("${SSH[@]}" "md5sum $SD/openipc-uboot.bin | cut -c1-32")
[ "$GOT" = "$MD5" ] || { echo "md5 on the card $GOT differs from $MD5: stop"; exit 1; }
"${SSH[@]}" "cat > /tmp/prepare-uboot.sh" < "$HERE/prepare-uboot.sh"
"${SSH[@]}" "sh /tmp/prepare-uboot.sh $SD $CRC; sync"

cat <<EOF

Ready. Reboot the camera:  ssh root@$IP reboot
Do not switch the power off during this reboot.
To cancel before rebooting:  ssh root@$IP fw_setenv ub 0
EOF
