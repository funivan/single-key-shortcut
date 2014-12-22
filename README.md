# One push - one command
===================

Programs for second keyboard. 

```
 
 make 
    
 ./out/program -d /dev/input/by-id/usb-Dell_Dell_USB_Entry_Keyboard-event-kbd -c keyboard.conf
 
 sudo chown userName /dev/input/event5
 
 xinput list-props 12
 xinput set-int-prop 12 "Device Enabled" 8 0 // disable
 xinput set-int-prop 12 "Device Enabled" 8 1 // enable


```