#!/bin/sh

fw_setenv osmem 32M

fw_setenv bootargs mem=\${osmem} panic=20 rootfstype=squashfs root=/dev/mtdblock3 init=/init mtdparts=hi_sfc:256k\(boot\),64k\(env\),3072k\(kernel\),10240k\(rootfs\),-\(rootfs_data\) mmz_allocator=hisi



