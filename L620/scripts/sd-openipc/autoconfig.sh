#!/bin/sh
#
# OpenIPC first-boot configuration for Steinel L620 / L620 SC.
# Executed once by OpenIPC automount when found on the SD card.
#
# - set osmem to 34M (more MMZ memory for majestic)
# - remove the kernel console from ttyAMA0: this UART is used by the light card
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

# Drop every console=... token, normalise spaces.
NEW="$(echo " $OLD " | sed -e 's/ console=[^ ]*/ /g' -e 's/  */ /g' -e 's/^ //' -e 's/ $//')"

if [ "$NEW" = "$OLD" ]; then
	log "bootargs already without console, nothing to do"
else
	fw_setenv bootargs "$NEW"
	log "bootargs after:  $(fw_printenv -n bootargs)"
fi

log "autoconfig done"
