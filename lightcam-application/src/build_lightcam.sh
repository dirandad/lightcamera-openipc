#!/bin/sh
#
# Build lightcam with the OpenIPC toolchain (arm-openipc-linux-musleabi).
#
# Usage: TOOLCHAIN_PATH=<toolchain>/bin ./build_lightcam.sh [camera-ip]
# With a camera IP, the binary is also copied to /usr/sbin/lightcam on that
# camera (SSH key needed) and lightcam is restarted.

TOOLCHAIN_PATH="${TOOLCHAIN_PATH:-$HOME/arm-openipc-linux-musleabi_sdk-buildroot/bin}"
CC="$TOOLCHAIN_PATH/arm-openipc-linux-musleabi-gcc"
CAMIP="$1"

cd "$(dirname "$0")" || exit 1

[ -x "$CC" ] || { echo "compiler not found: $CC (set TOOLCHAIN_PATH)"; exit 1; }

rm -f lightcam

echo "compiling lightcam..."
"$CC" -O2 -Wall -Wno-unused-variable -Wno-unused-but-set-variable -o lightcam \
	lightcam.c \
	serial/serial.c \
	gpiopmux/gpiopmux.c gpiopmux/led.c mqtt/*.c \
	libyaml/api.c libyaml/scanner.c libyaml/writer.c libyaml/parser.c libyaml/reader.c \
	-lpthread || exit 1
echo "done: $(ls -l lightcam)"

if [ -n "$CAMIP" ]; then
	echo "copying lightcam to $CAMIP..."
	ssh root@"$CAMIP" '/etc/init.d/S50lightcam stop; cat > /usr/sbin/lightcam && chmod 755 /usr/sbin/lightcam && /etc/init.d/S50lightcam start' < lightcam
fi
