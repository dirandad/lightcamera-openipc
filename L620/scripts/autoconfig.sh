#! /bin/sh

echo "<<<<<<<<<<<<<<<<<<<<<<<<<<< autoconfig.sh start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

echo "script had been started" > /mnt/mmcblk0p1/autoconfig.txt

sysupgrade -z --kernel=/mnt/mmcblk0p1/uImage.hi3518ev300 --rootfs=/mnt/mmcblk0p1/rootfs.squashfs.hi3518ev300 > /mnt/mmcblk0p1/sysupgrade.txt

fw_setenv osmem 43M

fw_setenv bootargs mem=\${osmem} panic=20 rootfstype=squashfs root=/dev/mtdblock3 init=/init mtdparts=hi_sfc:256k\(boot\),64k\(wtf\),2048k\(kernel\),5120k\(rootfs\),-\(rootfs_data\) mmz_allocator=hisi

reboot

echo "<<<<<<<<<<<<<<<<<<<<<<<<<<< xm_autoconfig.sh done >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

