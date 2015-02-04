#!/bin/sh

 DEVICE=/dev/input/by-id/usb-Dell_Dell_USB_Entry_Keyboard-event-kbd
 REGEXP="Dell"

 ID=`xinput | grep "$REGEXP" |  grep -oP "id=\d+" | grep -oP "\d+"`
 
 if [ -z "$ID" ] ; then
   echo "Can\`t detect device id";
   exit 1;
 fi

 xinput disable "$ID"
 make
  ./out/program -d $DEVICE -c keyboard.conf
  