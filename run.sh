#!/bin/sh

 DEVICE=/dev/input/by-id/usb-Dell_Dell_USB_Entry_Keyboard-event-kbd
 ID=12
 
 sudo chown ivan $DEVICE
 xinput disable $ID
 make
  ./out/program -d $DEVICE -c keyboard.conf

 
