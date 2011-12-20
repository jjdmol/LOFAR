#!/bin/bash
# version 2.3, date 01-06-2010,  M.J.Norden & A.Schoenmaker

SyntaxError()
{
	Msg=$1

	[ -z "${Msg}" ] || echo "ERROR: ${Msg}"
	echo ""
	echo "Syntax: $(basename $0) -c [yY] -p [pagenr] -a [aphexfile] -b [bphexfile]"
	exit 1
}

if [ ${#argv} == 0 ]; then 
  SyntaxError
fi

answer="n"
page=-1
bphexfile=""
aphexfile=""

eval set argv=`getopt "c:p:a:b:h" $*`
shift

while [ "$1" != "--" ]
do
	case "$1" in
		-c)	answer=$2
			shift 2
			;;
		-p)	page=$2
			shift 2
			;;
		-a)	aphexfile=$2
			shift 2
			;;
		-b)	bphexfile=$2
			shift 2
			;;
		*) SyntaxError
			;;
	esac
done
shift 

station=`hostname -s`

if [ -e /opt/lofar/etc/RemoteStation.conf ]; then
  let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
else
  echo "Could not find /opt/lofar/etc/RemoteStation.conf"
  let rspboards=12
fi

echo "This station is "$station
echo "The number of rspboards is "$rspboards
echo "The selected image page is "$page
echo "The bp hex file is "$bphexfile
echo "The ap hex file is "$aphexfile
if [ ! "$answer" =~ "([yY])" ]; then 
   read -p "Is this ok [y/n]" answer
fi

if [[ "$answer" =~ "([yY])" ]]; then   
   eval "swlevel 1"
   if [ -e $aphexfile -a -e $bphexfile ]; then
      for ((ind=0; ind < $rspboards; ind++)) do
	MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
	sudo rsuctl3 -w -q -p $page -b $bphexfile -a $aphexfile -m $MACadr -F 2>&1
      done
      if [ $page != 0 ] ; then
	for ((ind=0; ind < $rspboards; ind++)) do
  		MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
  		sudo rsuctl3_reset -m $MACadr -p $page -x -q 2>&1
        done
      else 
        echo "When the RSP flash action was sucessful please"
        echo "set page to page=0 and reset the RSP boards"
        echo "with restart_images.sh"
      fi       
   else 
     echo "Could not find one of the ap/bp hex files"
   fi
fi

exit 0
