#!/bin/sh

 ID=10
 xinput disable "$ID"
 make
  ./out/program -d $ID -c keyboard.conf
  