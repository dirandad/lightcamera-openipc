#!/bin/sh

# to enable this script, add the following line to crontab config file : '/etc/crontabs/root'
# <----
# *      *      *     *     *     /etc/lightcamwd.sh
# ---->


#check if lightcam is running
echo "check lightcam status"

if pidof -s lightcam >/dev/null 2>&1
  then
     # abc is running
     echo "running"
  else
     # abc is not running
     echo "not running"

     echo "starting lightcam"
     /etc/init.d/S50lightcam start
     echo "done"

fi
