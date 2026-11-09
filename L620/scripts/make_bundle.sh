#!/bin/bash
#
# Build the ipctool upgrade bundle (upgrade.hi3518ev300) that installs OpenIPC
# on a Steinel L620 / L620 SC from the original firmware.
#
# Usage: ./make_bundle.sh [output folder]
#
# Needs: bash, curl, tar, python3, git (Linux, WSL or Git Bash with Python).
# It downloads from the OpenIPC GitHub releases:
#   - openipc.hi3518ev300-nor-ultimate-split.tgz (firmware.bin: uImage + squashfs)
#   - u-boot-hi3518ev300-nor.bin
# and uses upgrade_bundle.py from the OpenIPC ipctool repository.
#
# The bundle is "packed": ipctool writes U-Boot at 0x0, keeps the current U-Boot
# environment at 0x40000, writes the kernel at 0x50000 and the rootfs right after
# it, at the next 64 KB boundary. This is the layout of firmware.bin, so the
# OpenIPC kernel finds its partitions by itself ("split NOR layout").
#
# Tested on 2026-11-09 with nightly-20261108 and ipctool 4b0c092.

set -eu

OUT="${1:-output}"
REL=https://github.com/OpenIPC/firmware/releases/download/latest
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

mkdir -p "$OUT"
cd "$WORK"

echo "Downloading the OpenIPC firmware and U-Boot..."
curl -sfL -O "$REL/openipc.hi3518ev300-nor-ultimate-split.tgz"
curl -sfL -O "$REL/u-boot-hi3518ev300-nor.bin"
tar -xzf openipc.hi3518ev300-nor-ultimate-split.tgz
md5sum -c firmware.bin.hi3518ev300.md5sum

FW=firmware.bin.hi3518ev300
# uImage: 64-byte header + data size (big-endian, offset 12)
ksize=$(( 64 + $(od -A n -t u1 -j 12 -N 4 $FW | awk '{print $1*16777216+$2*65536+$3*256+$4}') ))
roff=$(( (ksize + 65535) / 65536 * 65536 ))
# squashfs at the next 64 KB boundary; bytes_used: little-endian, offset 40
magic=$(od -A n -t x1 -j $roff -N 4 $FW | tr -d ' ')
[ "$magic" = "68737173" ] || { echo "no squashfs at offset $roff of $FW"; exit 1; }
rsize=$(od -A n -t u8 -j $((roff + 40)) -N 8 $FW | tr -d ' ')
echo "kernel: $ksize bytes, rootfs: $rsize bytes at offset $roff"
head -c $ksize $FW > uImage.hi3518ev300
tail -c +$((roff + 1)) $FW | head -c $rsize > rootfs.squashfs.hi3518ev300

echo "Getting upgrade_bundle.py from OpenIPC/ipctool..."
git clone -q --depth 1 https://github.com/OpenIPC/ipctool ipctool

python3 ipctool/tools/upgrade_bundle.py \
	-b u-boot-hi3518ev300-nor.bin \
	-k uImage.hi3518ev300 \
	-r rootfs.squashfs.hi3518ev300 \
	-i -p -o upgrade.hi3518ev300

cd - >/dev/null
cp "$WORK/upgrade.hi3518ev300" "$OUT/"
ls -l "$OUT/upgrade.hi3518ev300"
echo "Copy $OUT/upgrade.hi3518ev300 to the root of the SD card."
