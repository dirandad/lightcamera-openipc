#!/bin/sh
if [ -f "lightcam" ]; then

	echo -n "deleting lightcam..."
	rm lightcam
	echo "done"
	
fi

echo -n "compiling lightcam..."

# arm-openipc-linux-musleabi-gcc -fPIC adtest.c -o adtest

# -Wl,-rpath=/usr/arm-openipc-linux-musleabi_sdk-buildroot/arm-openipc-linux-musleabi/sysroot/lib \
# -L/usr/arm-openipc-linux-musleabi_sdk-buildroot/arm-openipc-linux-musleabi/sysroot/lib \

../arm-openipc-linux-musleabi_sdk-buildroot/bin/arm-openipc-linux-musleabi-gcc -o lightcam \
 lightcam.c \
 serial/serial.c \
 gpiopmux/gpiopmux.c gpiopmux/led.c mqtt/*.c \
 libyaml/api.c libyaml/scanner.c libyaml/writer.c libyaml/parser.c libyaml/reader.c  \
  # libyaml/dumper.c libyaml/emitter.c  libyaml/loader.c 

echo "done"

if [ -f "lightcam" ]; then

	echo -n "copiing lightcam..."
	scp lightcam root@192.168.10.41:/root/
	scp lightcam.yml root@192.168.10.41:/root/
	echo "done"

fi
