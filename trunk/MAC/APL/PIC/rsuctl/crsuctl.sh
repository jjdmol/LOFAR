#!/bin/sh

#
# Script to run rsuctl on a number of MAC addresses using a MAC address template
# and filling in the missing index (MM).
#

MM="00 01 02 03 04 05 06 07 08 09 0a 0b"
#MM="08 09 0a 0b"

if [ $# -eq 0 ]; then
	echo "usage: `basename $0` arg*"
	echo "       String MM in any of the args will replaced by 00 01 02 etc" 
	echo "       E.g. `basename $0` -m 10:fa:00:00:MM:00 -i eth0 -l"
	exit
fi

#echo args=$*

for mac in $MM; do
	realargs=`echo $* | sed -e "s/MM/$mac/"`
	echo Running rsuctl $realargs
	sudo rsuctl $realargs
#	sudo rsuctl -m $mac_address -i eth1 -w -p $page -f $image $extra_options
done
