# !/bin/bash
# version 2.2, date 05-11-2008,  M.J.Norden
SyntaxError()
{
        Msg=$1

        [ -z "${Msg}" ] || echo "ERROR: ${Msg}"
        echo ""
        echo "Syntax: $(basename $0) -p [pagenr]"
        echo "        Default pagenr is 1"

        exit 1
}

page=-1
if [ "$1" == "" ]; then 
   page=1
else
  eval set argv=`getopt "p:h" $*`
  shift
  if [ "$1" == "--" ]; then SyntaxError; fi

  while [ "$1" != "--" ]
  do
        case "$1" in
                -p)     page=$2
                        shift 2
                        ;;

                -h) SyntaxError
                        ;;

                *)  SyntaxError
                        ;;
        esac
  done
  shift
fi

if [ -e /opt/lofar/etc/RemoteStation.conf ]; then
  let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
else
  echo "Could not find /opt/lofar/etc/RemoteStation.conf"
  let rspboards=12
fi

echo "You have selected page "$page
echo "Nr. of RSPBoards is: "$rspboards
read -p "Is this OK (y/n) [n]:" ok
[ -z "$ok" ] && ok="n"
case "$ok" in
   [yY] )
      ;;
   [nN] ) exit 0
      ;;
   *    ) echo "Assuming this means no..."
	  exit 0
      ;;
esac

eval "swlevel 1"

for ((ind=0; ind < $rspboards; ind++)) do
  MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
  sudo rsuctl3_reset -q -x -p $page -m $MACadr;
done

eval "swlevel 2"

