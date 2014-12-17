# One push - one command
===================

Programs for second keyboard. 

```
gcc -o ./out/program src/program.c && chmod +x ./out/program && ./out/program
 cat /proc/bus/input/devices   
 
 sudo ivan /dev/input/event5
 
 xinput list-props 12
 xinput set-int-prop 12 "Device Enabled" 8 0 // disable
 xinput set-int-prop 12 "Device Enabled" 8 1 // enable


```