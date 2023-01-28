#!/bin/sh

TOOLCHAIN_PATH="/home/adrien/arm-openipc-linux-musleabi_sdk-buildroot/bin/"
CAMIP="192.168.10.41"

if [ -f "lightcam" ]; then

	echo -n "deleting lightcam..."
	rm lightcam
	echo "done"
	
fi

echo -n "compiling lightcam..."


$TOOLCHAIN_PATH/arm-openipc-linux-musleabi-gcc -o lightcam \
 lightcam.c \
 serial/serial.c \
 gpiopmux/gpiopmux.c gpiopmux/led.c mqtt/*.c \
 libyaml/api.c libyaml/scanner.c libyaml/writer.c libyaml/parser.c libyaml/reader.c  \

echo "done"

if [ -f "lightcam" ]; then

	echo -n "copiing lightcam..."
	scp lightcam root@$CAMIP:/root/
	scp lightcam.yml root@$CAMIP:/root/
	scp S51lightcam root@$CAMIP:/root/
	echo "done"

fi
