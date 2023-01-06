#! /bin/sh

echo "<<<<<<<<<<<<<<<<<<<<<<<<<<< autoconfig.sh start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

echo "script had been started" > /mnt/mmcblk0p1/autoconfig.txt

sysupgrade -z --kernel=/mnt/mmcblk0p1/uImage.hi3518ev300 --rootfs=/mnt/mmcblk0p1/rootfs.squashfs.hi3518ev300 > /mnt/mmcblk0p1/sysupgrade.txt

fw_setenv osmem 43M

reboot

echo "<<<<<<<<<<<<<<<<<<<<<<<<<<< xm_autoconfig.sh done >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

