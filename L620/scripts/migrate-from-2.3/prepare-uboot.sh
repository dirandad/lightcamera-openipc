#!/bin/sh
#
# Prepare the replacement of U-Boot by the current OpenIPC U-Boot, written by
# the running U-Boot itself at the next boot. Run ON THE CAMERA, as root, after
# copying u-boot-hi3518ev300-nor.bin to <sd>/openipc-uboot.bin.
#
# Usage: sh prepare-uboot.sh <sd mount point> <crc32 of openipc-uboot.bin>
#
# At the next boot, the running U-Boot loads openipc-uboot.bin, checks its CRC32
# BEFORE erasing anything, then erases 0x0-0x40000, writes the file, reads it
# back and checks the CRC32, up to 3 times in the same boot. Then it boots as
# usual. The U-Boot environment (0x40000) is kept: the new U-Boot uses it.
#
# This is the riskiest write of the flash: if the power fails during the ~2 s of
# the write, the camera no longer starts and needs the serial console or a
# CH341A programmer. Tested on 2026-11-11 on the test camera (U-Boot 2016.11 of
# March 2023 replaced by U-Boot 2022.10 of 2026-11-09, environment kept).

set -u
SD=${1:?usage: prepare-uboot.sh <sd mount point> <crc32>}
CRC=${2:?usage: prepare-uboot.sh <sd mount point> <crc32>}

die() { echo "ERROR: $*"; exit 1; }

F="$SD/openipc-uboot.bin"
[ -f "$F" ] || die "$F not found"
echo "$CRC" | grep -q -E '^[0-9a-fA-F]{8}$' || die "CRC must be 8 hex digits"
SIZE=$(ls -l "$F" | awk '{print $5}')
[ "$SIZE" -gt 100000 ] && [ "$SIZE" -le 262144 ] || die "$F size $SIZE is not plausible (max 256 KB)"
# HiSilicon U-Boot images start with ARM vectors (bytes 3 and 7 = 0xea) and
# 0xdeadbeef at 0x20 (the image is compressed: no readable version string).
HEAD=$(dd if="$F" bs=36 count=1 2>/dev/null | hexdump -v -e '36/1 "%02x"')
[ "$(echo "$HEAD" | cut -c7-8)$(echo "$HEAD" | cut -c15-16)$(echo "$HEAD" | cut -c65-72)" = "eaeaefbeadde" ] ||
	die "$F does not look like a HiSilicon U-Boot image"
[ "$(fw_printenv -n mig 2>/dev/null)" = 1 ] && die "a firmware migration is pending (mig=1): reboot first"

C=$(echo "$CRC" | tr 'A-F' 'a-f')
SWAP=0x$(echo "$C" | cut -c7-8)$(echo "$C" | cut -c5-6)$(echo "$C" | cut -c3-4)$(echo "$C" | cut -c1-2)

fw_setenv ub_crc "$SWAP"
fw_setenv ub_run 'mmc rescan; if fatload mmc 0 0x42000000 openipc-uboot.bin && crc32 0x42000000 ${filesize} 0x41000000 && itest.l *0x41000000 == ${ub_crc}; then echo "u-boot: writing ${filesize} bytes"; setenv ub_ok 0; for t in 1 2 3; do if itest ${ub_ok} == 0; then sf probe 0; sf erase 0x0 0x40000; sf write 0x42000000 0x0 ${filesize}; mw.b 0x43000000 0x00 ${filesize}; sf read 0x43000000 0x0 ${filesize}; crc32 0x43000000 ${filesize} 0x41000000; if itest.l *0x41000000 == ${ub_crc}; then setenv ub_ok 1; echo "u-boot: written and verified, try ${t}"; else echo "u-boot: verify failed, try ${t}"; fi; fi; done; else echo "u-boot: no valid openipc-uboot.bin, nothing written"; fi; setenv ub 0; saveenv'

BC="$(fw_printenv -n bootcmd)"
case "$BC" in
	*"run ub_run"*) ;;
	*) fw_setenv bootcmd "if itest \${ub} == 1; then run ub_run; fi; $BC" ;;
esac
fw_setenv ub 1

echo "U-Boot:"
fw_printenv ub ub_crc bootcmd
echo "Ready: reboot the camera to replace U-Boot (crc32 $C, $SIZE bytes)."
