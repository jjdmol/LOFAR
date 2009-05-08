#!/bin/bash 
#version 1.0, date 07-04-2009,  M.J.Norden

#
# SyntaxError msg
#
SyntaxError()
{
	Msg=$1

	[ -z "${Msg}" ] || echo "ERROR: ${Msg}"
	echo ""
	echo "Syntax: $(basename $0) -p [pagenr] -m [mphexfile] -t [tphexfile]"
	exit 1
}

page=-1
m_file=""
t_file=""

eval set argv=`getopt "p:m:t:h" $*`
shift

if [ "$1" == "--" ]; then SyntaxError; fi

while [ "$1" != "--" ]
do
	case "$1" in
		-p)	page=$2
			shift 2
			;;
		-m)	m_file=$2
			shift 2
			;;
		-t)	t_file=$2
			shift 2
			;;

		-h) SyntaxError
			;;

	esac
done
shift

station=`hostname -s`
if [ -e /opt/lofar/etc/RemoteStation.conf ]; then
  let tbboards=`sed -n  's/^\s*RS\.N_TBBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
else
  echo "Could not find /opt/lofar/etc/RemoteStation.conf"
  exit 1
fi

echo "This station is "$station
echo "The number of tbboards is "$tbboards
echo "The selected image page is "$page
echo "The mp hex file is "$m_file
echo "The tp hex file is "$t_file

read -p "Is this ok [y/n]" answer

if [[ "$answer" =~ "([yY])" ]]; then   
   if [ -e $m_file -a -e $t_file ]; then
     for ((ind=0; ind < $tbboards; ind++)) do
        tbbctl --writeimage=$ind,$page,3.2,$t_file,$m_file
     done
     tbbctl --config=$page
   else 
     echo "Could not find one of the hex files"
   fi
fi
