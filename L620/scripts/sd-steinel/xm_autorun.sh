#! /bin/sh

# this function is in /etc/init.d/rcS
# it manage wificonnection, but is not run by rcS if there is a xm_autorun.sh file

fun_start_app()
{
	[ ! -z `command -v XmSearchIp` ] && XmSearchIp &
if [ -f /mnt/mtd/Log/WifiStartupType ];then
	STARTUP_TYPE=`sed -n "s/StartupType:\(.*\).*/\1\n/p" /mnt/mtd/Log/WifiStartupType`
	if [ $STARTUP_TYPE = 2 ];then
		[ ! -z `command -v wlandaemon` ] && wlandaemon 0 1 2 &
	elif [ $STARTUP_TYPE = 3 ];then
		[ ! -z `command -v wlandaemon` ] && wlandaemon 1 1 2 &
	else
		[ ! -z `command -v wlandaemon` ] && wlandaemon $STARTUP_TYPE &
	fi	
else
	[ ! -z `command -v wlandaemon` ] && wlandaemon 0 1 0 &
fi
	[ ! -z `command -v wifiscan` ] && wifiscan &
	[ ! -z `command -v dogtest` ] && dogtest &
	#[ ! -z `command -v route_switch` ] && route_switch &
	if [ -x /usr/bin/pppd ];then
	    if [ -f /mnt/mtd/Config/ppp/pppoe-start ];then
	        PPPOE_START=/mnt/mtd/Config/ppp/pppoe-start
	    else
	        PPPOE_START=/etc/ppp/pppoe-start
	    fi
	    /usr/bin/pppd pty $PPPOE_START file /etc/ppp/pppoe-options &
	fi
	[ -x /mnt/custom/extapp.sh ] && /mnt/custom/extapp.sh &
	#[ ! -z `command -v xm_reboot` ] && xm_reboot 60 &

	#dvrHelper /lib/modules /usr/sbin/SofiaRun.sh 127.0.0.1 9578 1 &
}


echo "<<<<<<<<<<<<<<<<<<<<<<<<<<< xm_autorun.sh start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"

mkdir -p /var/tmp/sd
mount /dev/mmcblk0 /var/tmp/sd

echo "script had been started" > /var/tmp/sd/test.txt

fun_start_app

sleep 10

# set environnement variable to enable telnet (could need a reboot)
XmEnv telnetctrl 1

# start telnetd
telnetd 
sleep 1

# need to exist after start of telnetd to allow local telnet connexion
touch /var/Telnetd_WanCheckFlag


# if ipctool exists, than create a backup of firmware

if [ -f /var/tmp/sd/ipctool ];then
	/var/tmp/sd/ipctool backup /var/tmp/sd/firmwaredump.bin > /var/tmp/sd/ipctoolbackup.txt
fi


echo "<<<<<<<<<<<<<<<<<<<<<<<<<<< xm_autorun.sh done >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
