#!/bin/sh

 ID=10
 xinput enable "$ID"
 make
  ./out/program -d $ID -c keyboard.conf
  