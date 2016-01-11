#!/bin/bash
#
# V4.0, A.P. Schoenmakers, 07-01-2016

# usage: poweruphba.sh RCUMODE (5, 6 or 7) or
#        poweruphba.sh [-a] -m RCUMODE (5, 6, or 7)
#
# Power up of the HBA Tiles. 
# for rcumode 6 you need to switch the clock seperately (rspctl --clock=160)
# from MAC version V2_12 and higher you can directly switch between rcumodes
# The setting of hbadelays with 253 is needed for the new HBA-FE that are default off
#
# - automatic clock switch when needed
# - optionally automatic disble broken tiles from PVSS database

SyntaxError()
{
        Msg=$1

        [ -z "${Msg}" ] || echo "ERROR: ${Msg}"
        echo ""
        echo "Syntax: $(basename $0) [-a] [-m RCU mode] or $(basename $0) [RCU mode]"
        echo "-a: Power up all RCUs, do not check broken tile info"
        echo "-m: RCU mode to use (5,6 or 7)"
        echo ""
        echo "If only one argument provided, that is interpreted as the requested RCU mode."
	echo "No check for broken tile info will occur then."
        echo ""
        exit 0
}

HandleArgs()
{
  # Handle arguments
  if [ "$#" -eq 0 ]; then
    SyntaxError
  fi

  while getopts  "am:" flag
    do
      case "$flag" in
      a)
        checkbroken=0
        ;;
      m)
        hbamode=$OPTARG	   
        ;;
      h)
        SyntaxError
        ;;
      *)
        SyntaxError
        ;;
      esac
    done
}

# --- MAIN ---

# first check if swlevel < 6, otherwise this script will not funstion properly
level=`swlevel -S`

if [ $level -lt 2 -o $level -gt 5 ]; then 
  echo "First set swlevel between 2 and 5 (current is $level)"
  exit 0
fi

#Initialize parameters
checkbroken=1
hbamode=0

if [ "$#" -eq 1 ]; then
   hbamode=$1
   checkbroken=0
else 
   HandleArgs $* 
fi     


if [ $hbamode -lt 5 -o $hbamode -gt 7 ]; then 
   echo "RCUMode must be 5,6, or 7" 
   exit 0
fi

# Find current clock setting
clock=`rspctl --clock 2>&1|grep "Sample frequency"|sed -e 's/.*clock=\(...\)MHz/\1/'`

# Set clock to proper value
if [ $hbamode -eq 6 ]; then 
   if [ "$clock" == "200" ]; then 
      echo "wait 30 seconds for 160MHz clock switch"
      rspctl --clock=160
      sleep 30
   fi
else 
   if [ "$clock" == "160" ]; then
      echo "wait 30 seconds for 200MHz clock switch"
      rspctl --clock=200  
      sleep 30
   fi
fi 

# Set selected mode and switch on HBAs
rspctl --rcumode=$hbamode
sleep 1
rspctl --rcuenable=1
sleep 1

# Determine broken tiles. On International stations in local mode we need to read a file
# called /localhome/stationtest/DISABLED/disabled-mode5.txt
# For NL stations and international stations in ILT mode we can query PVSS

DISABLED_RCU_LIST=""
modeline=`stationswitch -s`
if [[ $modeline =~ ilt ]]; then 
   if [ $checkbroken -eq 1 ]; then 
      if [ -e /opt/lofar/sbin/disabledRCUlist ]; then  
         DISABLED_RCU_LIST=`/opt/lofar/sbin/disabledRCUlist $hbamode 2</dev/null`
      else
         echo "Cannot determine broken RCUs; missing /opt/lofar/sbin/disabledRCUlist"
      fi
   fi 
else
  if [ -e /localhome/stationtest/DISABLED/disabled-mode5.txt ]; then 
     DISABLED_RCU_LIST=`cat /localhome/stationtest/DISABLED/disabled-mode5.txt`
  else
     echo "Cannot determine broken RCUs; missing file /localhome/stationtest/DISABLED/disabled-mode5.txt"
  fi
fi

# Switch off broken tiles (if any)
if [ "$DISABLED_RCU_LIST" == "" ]; then 
   echo "No disabled HBA tiles found" 
else 
   echo "List of disabled RCUs: "$DISABLED_RCU_LIST
   rspctl --rcumode=0 --select=$DISABLED_RCU_LIST 
   sleep 1
   rspctl --rcuenable=0 --select=$DISABLED_RCU_LIST 
   sleep 1
fi

# Set delay values
rspctl --hbadelays=253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253
sleep 1

# Set spectral Inversion properly
if [ $hbamode -eq 5 ]; then 
   rspctl --specinv=1
else
   rspctl --specinv=0
fi
sleep 1

echo "Done"
