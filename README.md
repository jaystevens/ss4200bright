ss4200bright (linux)
============

Intel ss4200 brightness tool

Usage: ss4200bright <number 0-100>
  
  
  
---
`init.d/ss4200led` - init script for setting up leds  

uses leds_ss4200 module  
ss4200bright in (/usr/local/bin)

STARTUP:  
set's brightness to 10% (dim)  
turns off amber leds  
turns on blue leds  
stop's blinking 

SHUTDOWN:  
set's brightness to 100% (high)  
turn on amber leds  
turn off blue leds  
start blinking amber leds

