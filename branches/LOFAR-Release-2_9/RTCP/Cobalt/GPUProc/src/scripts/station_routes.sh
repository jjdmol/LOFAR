#!/bin/bash

# Just a note containing all stations
ALLSTATIONS="
  CS001
  CS002 CS003 CS004 CS005 CS006 CS007
  CS011 CS013 CS017
  CS021 CS024 CS026 CS028
  CS030 CS031 CS032
  CS101 CS103
  CS201
  CS301 CS302
  CS401
  CS501
  RS106
  RS205 RS208 RS210
  RS305 RS306 RS307 RS310
  RS406 RS407 RS409
  RS503 RS508 RS509

  DE601 DE602 DE603 DE604 DE605
  FR606 SE607 UK608
"

function error {
  echo "$@" >&2
  exit 1
}

# Return the control IP of a certain station (CS001)
function station_ip {
  STATION="$1"

  host "$STATION"c | cut -d\  -f 4
}

# Extract the station name (CS001) from a full field name (CS001HBA1)
function fullfield_to_station {
  FULLFIELD="$1"
  echo "$FULLFIELD" | cut -c1-5
}

# Extract the field name (HBA1) from a full field name (CS001HBA1)
function fullfield_to_field {
  FULLFIELD="$1"
  echo "$FULLFIELD" | cut -c6-
}

# Return the list of IP suffices for a certain field (HBA1)
function field_ip_suffices {
  FIELD="$1"

  case "$FIELD" in
    "HBA1") echo "7 8 9 10";
            ;;
    *)      echo "1 2 3 4";
  esac
}

# Return all IPs for a certain full field name (CS001HBA1)
function fullfield_to_ips {
  FULLFIELD="$1"
  STATION=`fullfield_to_station "$FULLFIELD"`
  STATIONIP=`station_ip "$STATION"`
  FIELD=`fullfield_to_field "$FULLFIELD"`
  FIELDSUFFICES=`field_ip_suffices "$FIELD"`

  for SUFFIX in $FIELDSUFFICES
  do
    echo "$STATIONIP" | sed "s/151/175/; s/[.][0-9]*$/.$SUFFIX/"
  done
}

function usage {
  echo "$0: Configure the routing table on a Cobalt node to receive specific antenna fields."
  echo ""
  echo "Usage: $0 -a -i IFACE FIELD [FIELD...]"
  echo "  Adds a FIELD (CS002HBA1, etc) to arrive on interface IFACE (eth2, or cbt009-10GB01, etc). [needs root]"
  echo ""
  echo "Usage: $0 -d FIELD [FIELD...]"
  echo "  Delete a FIELD (CS002HBA1, etc) to arrive on this node. [needs root]"
  echo ""
  echo "Usage: $0 -l"
  echo "  List the fields that arrive on this node."
  echo ""
  echo "Note: Only routes for HBA fields have to be considered for correct operation. The LBA configuration"
  echo "      overlaps with the HBA(0) fields."
  exit 1
}

IFACE=""
ADD=0
DELETE=0
LIST=0

while getopts "dahli:" opt; do
  case $opt in
    h)  usage
        ;;
    i)  IFACE="$OPTARG"
        ;;
    a)  ADD=1
        ;;
    d)  DELETE=1
        ;;
    l)  LIST=1
        ;;
  esac
done
shift $((OPTIND-1))

if [ "$LIST" -eq 1 ]; then
  ERROR=0

  # List stations we're receiving
  for IFACE in eth2 eth3 eth4 eth5
  do
    IPS=`ip route show dev $IFACE | cut -d\  -f 1`
    for IP in $IPS
    do
      # convert to control interface
      CONTROLIP=`echo $IP |sed 's/175/151/; s/[.][0-9]*$/.1/'`

      host "$CONTROLIP" >/dev/null || continue

      STATION=`host "$CONTROLIP" | cut -d\  -f 5 | cut -d. -f 1 | cut -c1-5 | tr a-z A-Z`

      SUFFIX=`echo $IP | cut -d. -f 4`

      if [ "${SUFFIX}" -le 4 ]; then
        if [ "`echo "$STATION" | cut -c1,2`" == "CS" ]; then
          FIELD=HBA0
        else
          FIELD=HBA
        fi
      else
        FIELD=HBA1
      fi

      FULLFIELDS="$FULLFIELDS $STATION$FIELD"
    done
  done

  # Check whether we receive all boards from all stations we receive something from
  for FULLFIELD in `echo $FULLFIELDS | tr ' ' '\n' | sort | uniq`
  do
    echo "Checking routes for field $FULLFIELD..."
    LASTIFACE=""
    for IP in `fullfield_to_ips "$FULLFIELD"`
    do
      ROUTE=`ip route show $IP`
      if [ -z "$ROUTE" ]; then
        echo "ERROR: Missing route for $IP"
        ERROR=1
      else
        IFACE=`echo "$ROUTE" | awk '{ print $3; }'`

        if [ "$LASTIFACE" == "" ]; then
          echo "  Arrives on $IFACE"
        elif [ "$IFACE" != "$LASTIFACE" ]; then
          echo "ERROR: Route to $IP arrives on $IFACE, but previous board(s) arrive on $LASTIFACE"
          ERROR=1
        fi

        LASTIFACE="$IFACE"
      fi
    done
  done

  exit $ERROR
fi

# Add stations to receive

if [ "$ADD" -eq 1 ]; then
  [ -z "$IFACE" ] && usage

  [ "`whoami`" == "root" ] || error "Need to be root."

  HOST=localhost

  # support cbt00X-10GB0Y notation (go to the relevant node)
  if `echo "$IFACE" | fgrep -q -- '-'`
  then
    HOST=`echo "$IFACE" | cut -d- -f 1`
    IFACE=`echo "$IFACE" | cut -d- -f 2 | sed 's/10GB01/eth2/i; s/10GB02/eth3/i; s/10GB03/eth4/i; s/10GB04/eth5/i;'`
  fi

  for FULLFIELD in "$@"
  do
    echo "Adding routes for field $FULLFIELD..."
    for IP in `fullfield_to_ips "$FULLFIELD"`
    do
      ssh $HOST ip route add $IP dev $IFACE
    done
  done

  exit 0
fi

if [ "$DELETE" -eq 1 ]; then
  [ "`whoami`" == "root" ] || error "Need to be root."

  for FULLFIELD in "$@"
  do
    echo "Deleting routes for field $FULLFIELD..."
    for IP in `fullfield_to_ips "$FULLFIELD"`
    do
      ip route del $IP
    done
  done

  exit 0
fi

usage
