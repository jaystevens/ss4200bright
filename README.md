ss4200bright (linux)
============

Intel ss4200 brightness tool

`Usage: ss4200bright <number 0-255>`  
  
Does not work if the `lm85` kernel hwmon module is loaded and using the i2c device.  
If you are rolling your own kernel, make sure `lm85` is a Module.  
Requires i2c device module(i2c-dev).  
  
Make sure to double check the i2c device of the ss4200 LED device before compiling ss4200bright.  
On Fedora 18-22 with kernels 3.9-4.2 i2c device is `/dev/i2c-6`  
On Intel/EMC firmware i2c device was `/dev/i2c-0`  
  
---
`init.d/ss4200led` - init script for setting up leds  

ss4200bright in (/usr/local/bin)

STARTUP:  
set's brightness to 5 (very dim)  

SHUTDOWN:  
set's brightness to 255 (high)  

NOTES:  
kernel module `leds_ss4200` crashes when loading sometime after 3.9/3.10 up to 4.2  
so there is no way now to control individual leds: blue, amber, on, off, blink


---
`init.d/ss4200hdparm` - init script for setting drive parameters

disables drive APM `hdparm -B 255`  
disables drive spin down `hdparm -S 0`  

---
**no longer tested/used**
`init.d/varlogdirs` - init script to buiding /var/log folder structure

this script builds skeleton files and folders for `/var/log` on Fedora 18/19

on shutdown this script rsync's `/var/log` to `/var/.log`  
i.e. assumes that `/var/log` is tmpfs.  
yes I know that this overwrites log files from the last clean shutdown.
