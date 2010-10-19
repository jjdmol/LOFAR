#!/bin/sh

MM="00 01 02 03 04 05 06 07 08 09 0a 0b"

if [ $# -eq 0 ]; then
	echo "usage: `basename $0` arg*"
	echo "       String MM in any of the args will replaced by 00 01 02 etc" 
	echo "       E.g. `basename $0` -m 10:fa:00:00:MM:00 -i eth0 -l"
	exit
fi

#echo args=$*
here=`dirname $0`

for mac in $MM; do
	realargs=`echo -m 10:fa:00:00:MM:00 $* | sed -e "s/MM/$mac/"`
	echo Running rsuctl $realargs
	sudo $here/rsuctl -q $realargs
done
