#!/bin/sh
#
# OpenIPC first-boot configuration for Steinel L620 / L620 SC.
# Executed once by OpenIPC automount when found on the SD card.
#
# - set osmem to 34M (only used by older OpenIPC versions)
# - CMA memory allocator, so that upgrades from the web interface have the RAM
#   they need
# - free ttyAMA0, the UART of the light card: kernel console off (console=null)
#   and no login prompt (getty) on it
# - read 4 MB of kernel at boot, so that later upgrades with a bigger kernel boot
# - set the Wi-Fi network from wifi.conf, if present (then delete wifi.conf)
# - install the files of autoconfig/ (SC2235P sensor driver, lightcam, Wi-Fi
#   power saving off, SSH key) with normal permissions, then rename the folder
#   to autoconfig.done/ so that they are not copied again at each boot
# - start lightcam with a cron watchdog, when /etc/lightcam.yml was provided
#
# IMPORTANT: bootargs is edited in place. The partition layout (mtdparts,
# root=...) written by the upgrade bundle is kept as is. A hardcoded mtdparts
# line made cameras flashed with the packed bundle (2 MB kernel) loop on
# kernel panic at the second boot (issues #7 and #19).

LOG="$(dirname "$0")/autoconfig.log"
touch "$LOG" 2>/dev/null || LOG=/tmp/autoconfig.log

log() {
	echo "$(date '+%Y-%m-%d %H:%M:%S') $*" >> "$LOG"
}

log "autoconfig start"

fw_setenv osmem 34M && log "osmem=34M"

OLD="$(fw_printenv -n bootargs 2>/dev/null)"
if [ -z "$OLD" ]; then
	log "ERROR: cannot read bootargs, left unchanged"
	exit 1
fi
log "bootargs before: $OLD"

# Drop every console=... token, normalise spaces, then add console=null.
# Without any console=, the kernel of recent OpenIPC versions falls back to
# ttyAMA0 (the light card UART): its messages and the login prompt would go to
# the MCU. console=null keeps the UART for lightcam only. U-Boot keeps its own
# serial console, for recovery.
NEW="$(echo " $OLD " | sed -e 's/ console=[^ ]*/ /g' -e 's/  */ /g' -e 's/^ //' -e 's/ $//')"
NEW="console=null $NEW"

if [ "$NEW" = "$OLD" ]; then
	log "bootargs already with console=null, nothing to do"
else
	fw_setenv bootargs "$NEW"
	log "bootargs after:  $(fw_printenv -n bootargs)"
fi

# Memory: CMA allocator. With the default "hisi" allocator, this kernel gives
# Linux 32 MB and the video memory (MMZ) the other 32 MB, whatever osmem says:
# Linux has about 8 MB free, and an upgrade from the web interface (sysupgrade)
# is killed by the OOM killer while it downloads the 10 MB firmware (tested on
# 2026-11-09). With CMA, Linux sees the 64 MB and lends the MMZ area to the
# upgrade once majestic is stopped. set_allocator keeps the other bootargs.
if command -v set_allocator >/dev/null 2>&1; then
	if ! grep -q 'mmz_allocator=cma' /proc/cmdline && ! fw_printenv -n bootargs 2>/dev/null | grep -q 'mmz_allocator=cma'; then
		set_allocator cma 32M >/dev/null 2>&1 && log "memory: CMA allocator set, at the next boot: $(fw_printenv -n bootargs)"
	fi
fi

# Kernel read size. ipctool writes a bootcmd that reads exactly the size of the
# kernel it installed (for example "sf read 0x42000000 0x50000 0x1e0000").
# sysupgrade later rewrites the whole firmware but not bootcmd: a bigger kernel
# would then be read truncated, and the camera would stay in U-Boot (bad CRC).
# Read 4 MB instead: bootm only uses the size given in the uImage header.
BC="$(fw_printenv -n bootcmd 2>/dev/null)"
case "$BC" in
	*"sf read 0x42000000 0x50000 0x"*)
		NEWBC="$(echo "$BC" | sed 's/sf read 0x42000000 0x50000 0x[0-9a-fA-F]*/sf read 0x42000000 0x50000 0x400000/')"
		if [ "$NEWBC" != "$BC" ]; then
			fw_setenv bootcmd "$NEWBC"
			log "bootcmd: kernel read size set to 4 MB: $(fw_printenv -n bootcmd)"
		fi
		;;
	*)
		log "bootcmd left unchanged: $BC"
		;;
esac

# Wi-Fi: wifi.conf on the card, lines "key=value" with the keys wlanssid,
# wlanpass and optionally wlandev (default rtl8188fu-generic, the Wi-Fi chip of
# the L620) and wlanmac. The values go to the U-Boot environment, read by the
# OpenIPC network scripts. The file is deleted afterwards: the password does not
# stay on the card.
WIFI="$(dirname "$0")/wifi.conf"
if [ -f "$WIFI" ]; then
	# "|| [ -n ... ]": also read a last line without a newline (Notepad).
	tr -d '\r' < "$WIFI" | while IFS= read -r line || [ -n "$line" ]; do
		key="${line%%=*}"
		val="${line#*=}"
		case "$key" in
			wlandev|wlanssid|wlanpass|wlanmac)
				[ -n "$val" ] && fw_setenv "$key" "$val" && log "$key set"
				;;
		esac
	done
	[ -n "$(fw_printenv -n wlanssid 2>/dev/null)" ] && [ -z "$(fw_printenv -n wlandev 2>/dev/null)" ] &&
		fw_setenv wlandev rtl8188fu-generic && log "wlandev=rtl8188fu-generic"
	log "Wi-Fi network: $(fw_printenv -n wlanssid 2>/dev/null)"
	rm -f "$WIFI" && log "wifi.conf deleted from the card"
fi

# OpenIPC copies autoconfig/ to / with "cp -a" before running this script. On a
# FAT card every folder is rwxrwxrwx, and cp -a gives these permissions to the
# existing system folders (/etc, /root, ...). Restore normal permissions on every
# folder that came from the card. Without this, /etc and /root are writable by
# everyone, and dropbear refuses SSH keys ("/root must be owned by user or root,
# and not writable by others").
AC="$(dirname "$0")/autoconfig"
if [ -d "$AC" ]; then
	(cd "$AC" && find . -type d) | while IFS= read -r d; do
		d="${d#.}"
		[ -n "$d" ] && [ -d "$d" ] && chmod 755 "$d"
	done
	(cd "$AC" && find . -type f) | while IFS= read -r f; do
		f="${f#.}"
		case "$f" in
			*/init.d/*|/usr/sbin/*|/usr/bin/*|*.sh) chmod 755 "$f" ;;
			*) chmod 644 "$f" ;;
		esac
	done
	log "permissions of the copied folders restored"
	# lightcam.yml holds the MQTT password: root only, and not left on the card
	if [ -f /etc/lightcam.yml ]; then
		chmod 600 /etc/lightcam.yml
		rm -f "$AC/etc/lightcam.yml" && log "lightcam.yml installed, deleted from the card"
	fi
	# OpenIPC copies autoconfig/ at EVERY boot, but runs this script only once
	# (it deletes it). Rename the folder so that the next boots do not copy it
	# again with the FAT permissions, nor overwrite later changes.
	rm -rf "$AC.done"
	mv "$AC" "$AC.done" && log "autoconfig/ renamed to autoconfig.done/ on the card"
fi

# Host name from autoconfig/etc/hostname (kept by the migration from OpenIPC
# 2.3): apply it now, init only reads /etc/hostname at boot.
if [ -s /etc/hostname ] && [ "$(hostname)" != "$(cat /etc/hostname)" ]; then
	hostname -F /etc/hostname && log "host name: $(hostname)"
fi

# SSH key copied from autoconfig/root/.ssh/authorized_keys (/root/.ssh is a
# link to /etc/dropbear on OpenIPC).
if [ -f /root/.ssh/authorized_keys ]; then
	chmod 700 "$(readlink -f /root/.ssh)"
	chmod 600 /root/.ssh/authorized_keys
	log "SSH key installed"
fi

# IR-cut filter off in majestic: the L620 lights the scene with its own lamp at
# night, and no GPIO moves the filter (the Steinel firmware never moved it).
# Without this, the web interface warns that majestic cannot move the filter.
if command -v cli >/dev/null 2>&1; then
	cli -s .nightMode.irCut off && log "majestic: IR-cut filter set to off"
fi

# No login prompt on ttyAMA0: comment the getty line of /etc/inittab. It reads
# the UART too and would take the answers of the MCU.
if grep -q '^console::respawn:/sbin/getty' /etc/inittab 2>/dev/null; then
	sed -i 's|^console::respawn:/sbin/getty|#lightcam: ttyAMA0 is used by the light card# &|' /etc/inittab
	kill -HUP 1 2>/dev/null
	killall getty 2>/dev/null
	log "getty on the console disabled in /etc/inittab"
fi

# lightcam: start it now (rcS chose the scripts to run before this script
# copied S50lightcam), and add the cron watchdog that restarts it.
if [ -x /usr/sbin/lightcam ]; then
	if [ -f /etc/lightcam.yml ]; then
		CRON=/etc/crontabs/root
		mkdir -p /etc/crontabs
		if ! grep -q lightcamwd "$CRON" 2>/dev/null; then
			echo "* * * * * /etc/lightcamwd.sh" >> "$CRON"
			log "lightcam watchdog added to $CRON"
		fi
		/etc/init.d/S50lightcam start >/dev/null 2>&1 && log "lightcam started"
	else
		log "lightcam installed but not started: no autoconfig/etc/lightcam.yml on the card"
	fi
fi

log "autoconfig done"
