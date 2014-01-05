#!/bin/sh

# This script loads the drivers necessary to use the nasAPI.
# Run it after the nasgpio driver is built.

modprobe i2c-dev
rmmod nasgpio
insmod "${CF_PROJECT:+"$CF_PROJECT/"}nasgpio/nasgpio.ko"

# The nasGpio driver uses a dynamic major device number; find out what it is.
while read a b ; do
    [ "Block" = "$a" ] && break
    if [ "nasGpio" = "$b" ] ; then
        major="$a"
    fi
done </proc/devices

if [ -n "$major" ] ; then
    rm -f /dev/nasGpio0
    mknod /dev/nasGpio0 c "$major" 0
else
    printf "%s\n" "WARNING: Couldn't find nasGpio device number; device not created."
fi
