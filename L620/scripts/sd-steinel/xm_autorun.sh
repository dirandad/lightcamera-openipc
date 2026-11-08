#!/bin/sh
#
# Diagnostic + backup script for a Steinel L620 / L620 SC on the original
# firmware. Executed by /etc/init.d/rcS at boot, or by /etc/mdev/automount.sh
# when the card is inserted after boot. Writes only to the SD card (and the
# u-boot env variable telnetctrl, written AFTER the backup so the dump is pristine).
#
# Results on the SD card:
#   diag/ ...                 system information
#   firmwaredump-<mac>.bin    full 16 MB flash dump (ipctool backup)
#   firmwaredump-<mac>.md5
#   DONE                      present when the script reached the end

SD=$(cd "$(dirname "$0")" && pwd)
LOCK=/var/tmp/xm_diag.lock
[ -e $LOCK ] && exit 0
touch $LOCK

OUT=$SD/diag
mkdir -p $OUT
LOG=$OUT/run.log
log() { echo "$(date '+%Y-%m-%d %H:%M:%S') $*" >> $LOG; }
log "start, sd=$SD, called as $0"

# Same as fun_start_app in /etc/init.d/rcS, which rcS skips when this script
# exists. Only when the Wi-Fi daemon is not already running (hotplug case).
if ! pidof wlandaemon >/dev/null 2>&1; then
	log "starting network daemons"
	[ ! -z `command -v XmSearchIp` ] && XmSearchIp &
	if [ -f /mnt/mtd/Log/WifiStartupType ]; then
		STARTUP_TYPE=`sed -n "s/StartupType:\(.*\).*/\1\n/p" /mnt/mtd/Log/WifiStartupType`
		if [ "$STARTUP_TYPE" = 2 ]; then
			[ ! -z `command -v wlandaemon` ] && wlandaemon 0 1 2 &
		elif [ "$STARTUP_TYPE" = 3 ]; then
			[ ! -z `command -v wlandaemon` ] && wlandaemon 1 1 2 &
		else
			[ ! -z `command -v wlandaemon` ] && wlandaemon $STARTUP_TYPE &
		fi
	else
		[ ! -z `command -v wlandaemon` ] && wlandaemon 0 1 0 &
	fi
	[ ! -z `command -v wifiscan` ] && wifiscan &
	[ ! -z `command -v dogtest` ] && dogtest &
	[ -x /mnt/custom/extapp.sh ] && /mnt/custom/extapp.sh &
fi

# --- system information (no Wi-Fi keys copied) ---
log "collecting information"
{
	echo "== date";        date
	echo "== uname";       uname -a
	echo "== cmdline";     cat /proc/cmdline
	echo "== mtd";         cat /proc/mtd
	echo "== mount";       mount
	echo "== meminfo";     head -5 /proc/meminfo
	echo "== cpuinfo";     cat /proc/cpuinfo
	echo "== FirmwareInfo"; cat /mnt/custom/FirmwareInfo 2>&1
	echo "== ProductTrail"; cat /mnt/mtd/Log/ProductTrail 2>&1
	echo "== SystemInfo";  ls -la /mnt/custom /mnt/mtd/Config /mnt/mtd/Log 2>&1
	echo "== uboot env (XmEnv)"; XmEnv -a 2>&1 | head -80
	echo "== ps";          ps
	echo "== ifconfig";    ifconfig -a
	echo "== sd mount";    mount | grep -i mmc; ls -l /dev/mmcblk* 2>&1
} > $OUT/system.txt 2>&1

cp /etc/init.d/rcS $OUT/rcS.txt 2>/dev/null
cp /etc/mdev/automount.sh $OUT/automount.sh.txt 2>/dev/null
cp /etc/mdev.conf $OUT/mdev.conf.txt 2>/dev/null

if [ -x $SD/ipctool ] || chmod +x $SD/ipctool 2>/dev/null; then
	$SD/ipctool > $OUT/ipctool.txt 2>&1
	$SD/ipctool reginfo > $OUT/ipctool-reginfo.txt 2>&1
	log "ipctool info done"
fi

# --- full flash backup, once ---
MAC=$(cat /sys/class/net/wlan0/address 2>/dev/null | tr -d ':')
[ -z "$MAC" ] && MAC=$(cat /sys/class/net/eth0/address 2>/dev/null | tr -d ':')
[ -z "$MAC" ] && MAC=$(sed -n 's/.*mac: "\(.*\)"/\1/p' $OUT/ipctool.txt | tr -d ':')
[ -z "$MAC" ] && MAC=unknown
DUMP=$SD/firmwaredump-$MAC.bin
if [ -x $SD/ipctool ] && [ ! -s $DUMP ]; then
	log "backup to $DUMP"
	$SD/ipctool backup $DUMP > $OUT/ipctool-backup.txt 2>&1
	ls -l $DUMP >> $LOG 2>&1
	log "backup done"
else
	log "backup skipped (exists or no ipctool)"
fi

# --- telnet for interactive diagnostics (login root, see README) ---
sleep 10
XmEnv -s telnetctrl 1 >/dev/null 2>&1 || XmEnv telnetctrl 1 >/dev/null 2>&1
telnetd
sleep 1
touch /var/Telnetd_WanCheckFlag
log "telnetd started"

sync
date > $SD/DONE
sync
log "end"
