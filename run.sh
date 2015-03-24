#!/bin/bash
#
# You need: 
# sudo apt-get install libx11-dev libxi-dev -y
#
#
#
#

  export SHELL="/bin/bash"
  export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

 make
 
 if [[ "$1" == "-i" ]]; then
    make install
 else
  ./out/singlekey -c keyboard.conf
  #xinput disable 10
 fi;