#!/bin/sh
#
# lightcam watchdog: starts lightcam again if it is not running.
# Run every minute by cron, with this line in /etc/crontabs/root:
#   * * * * * /etc/lightcamwd.sh
# (autoconfig.sh on the SD card adds it).

if ! pidof -s lightcam >/dev/null 2>&1; then
	logger -t lightcamwd "lightcam is not running, starting it"
	/etc/init.d/S50lightcam start >/dev/null 2>&1
fi
