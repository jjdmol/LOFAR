#!/bin/bash
#
# To record raw data:
#
# 1. Define an observation in OTDB.
# 2. Disable Cobalt, by either:
#      a. removing OnlineControl from the VIC tree
#      b. moving/disabling cbm001:/opt/lofar/bin/startBGL.sh
# 3. Adjust the following parameters in this script:
#      a. NAME, used as an ID in the file names that will be created
#      b. FILTERPARAMS (# subbands/board, start time, stop time)
#      c. STATIONS, FIELD, and BOARDS
#      d. OUTPUT (to route to Cobalt disk (< ~2min obs) or to locus nodes)
#      e. LOCUS_NODES, if routing to locus
# 4. Run this script, which produces:
#       record-cobalt.sh:  the script to run to record station data as ROOT on Cobalt
#       record-locus.sh:  the script to run to record station data on locus
#       replay.sh:     the script to run to replay the recorded station data
#       replay.parset: parset keys needed to replay the recorded station data
# 5. Run record.sh before the obs starts

# Sane default
if [ -z "$LOFARROOT" ]; then
  LOFARROOT=/opt/lofar
fi

# Raw parameters to filterRSP
# -q: Quiet, don't spam progress logs
# -s: Number of subbands to record [SET TO NUMBER OF SUBBANDS]
# -f: Record from this timestamp [SET TO OBS START TIME - 1SEC]
# -t: Record until this timestamp [SET TO OBS END TIME]
FILTERPARAMS="-s 122 -q -f '2015-01-12 13:44:59' -t '2015-01-12 13:50:01'"

# Identifier for the output files [SET TO SOMETHING UNIQUE]
NAME="b1919-2015-01-12-run2"

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

# Antennaset to record [SET TO ANTENNA SET]
FIELD=HBA_DUAL

# Which board(s) to record
BOARDS="0"

# Stations to record [SET TO STATION LIST]
STATIONS="
  CS001
  CS002 CS003 CS004 CS005 CS006 CS007
  CS011 CS017
  CS021 CS024 CS026 CS028
  CS030 CS031 CS032
  CS101 CS103
  CS201
  CS301 CS302
  CS401
"

# Output to "locus" or to "file"
OUTPUT=locus

# List of locus nodes, and first port number to use (if OUTPUT=locus)
#
# To sort locus nodes on disk space available on /data, use:
#
# [lhn001] cexec -p df /data | grep sda10 | sort -n -k 7 | cut -c 12-14 | tr '\n' ' '
LOCUS_NODES=(094 092 100 093 080 058 013 069 079 077 063 066 071 076 082 090 061 062 064 068 085 060 067 072 086 087 089 047 048 050 053 055 088 049 056 078 040 043 045 070 075 051 054 074 026 027 073 081 091 098 016 029 039 041 008 012 014 025 034 035 038 042 052 006 010 018 031 032 037 057 005 007 011 017 020 021 028 030 046 015 019 022 023 059 084 004 003 009 036 044 001 065 083 096 099 095 024 002 033 097)
LOCUS_FIRST_PORT=12345

# Temporary location for the parset we create
PARSET=record_raw_data.parset

# We'll need a timestamp that won't change during this script
NOW=`date +%FT%T`

# Construct the list of all fields (f.e. CS001HBA0 CS001HBA1 RS106HBA)
FIELDS=""
for s in $STATIONS
do
  case $s in
    CS*)
      case $FIELD in
        LBA*)
          FIELDS="$FIELDS ${s}LBA"
          ;;
        HBA_ZERO*)
          FIELDS="$FIELDS ${s}HBA0"
          ;;
        HBA_ONE*)
          FIELDS="$FIELDS ${s}HBA0"
          ;;
        HBA_DUAL*)
          FIELDS="$FIELDS ${s}HBA0 ${s}HBA1"
          ;;
        HBA_JOINED*)
          FIELDS="$FIELDS ${s}HBA"
          ;;
      esac
      ;;
    *)
      case $FIELD in
        LBA*)
          FIELDS="$FIELDS ${s}LBA"
          ;;
        HBA*)
          FIELDS="$FIELDS ${s}HBA"
          ;;
      esac
      ;;
  esac
done

(
# Create a basic configuration
echo "Observation.Beam[0].subbandList = [0]"
echo "Observation.VirtualInstrument.stationList = [`echo $FIELDS | tr ' ' ','`]"
echo "Observation.rspBoardList = [0]"
echo "Observation.rspSlotList  = [0]"

# Add all input specifications
cat $LOFARROOT/etc/parset-additions.d/default/*.parset
)> $PARSET

LOCUSIDX=0

# Reset replay scripts
rm -f record-cobalt.sh record-locus.sh replay.sh replay.parset

# Start recording all fields
for s in $FIELDS
do
  # Determine the host for this field
  HOST=`$LOFARROOT/bin/station_stream -S $s -h $PARSET 2>/dev/null`;

  # List of input streams for the observation that is going to process this data
  OBS_INSTREAMS=""

  for b in $BOARDS
  do
    # The input stream
    INSTREAM=`$LOFARROOT/bin/station_stream -S $s -B $b -s $PARSET 2>/dev/null`;

    # The interface on which the stream is received
    IFACE=`echo $INSTREAM | cut -d: -f 2`

    # The socket to bind to on the node
    CPU=`$LOFARROOT/bin/station_stream -S $s -B $b -c $PARSET 2>/dev/null`;

    # The output stream
    case "$OUTPUT" in
      file)
        OUTSTREAM="file:/localhome/mol/raw-$NAME-$NOW-$s-$b.udp"
        OBS_INSTREAM="$OUTSTREAM"
        ;;
      locus)
        NR=${#LOCUS_NODES[@]}
        DESTNODENR=`echo "$LOCUSIDX % $NR" | bc`
        DESTNODE="locus${LOCUS_NODES[$DESTNODENR]}"
        DESTPORT=$((LOCUS_FIRST_PORT + LOCUSIDX))
        LOCUSIDX=$((LOCUSIDX + 1))

        # The interface Cobalt uses to connect to the given locus node:
        # locus001..025 -> 10GB01
        # locus026..050 -> 10GB02
        # locus051..075 -> 10GB03
        # locus076..100 -> 10GB04
        LOCUS_COBALT_IFACE_NR=`echo "(${LOCUS_NODES[$DESTNODENR]}-1) / 25 + 1" | bc`
        COBALT_NODE=`echo $IFACE | cut -d- -f 1`
        LOCUS_COBALT_IFACE="${COBALT_NODE}-10GB0${LOCUS_COBALT_IFACE_NR}"

        OUTSTREAM="tcp:$DESTNODE:$DESTPORT"
        FILESTREAM="file:/data/raw-$NAME-$NOW-$s-$b.udp"

        echo "# stream $s board $b [$OUTSTREAM -> $FILESTREAM]" >> record-locus.sh
        echo ssh $DESTNODE \"/globalhome/romein/bin.x86_64/udp-copy tcp:0:$DESTPORT $FILESTREAM\" "&" >> record-locus.sh

        OBS_INSTREAM="tcp:$LOCUS_COBALT_IFACE:$DESTPORT"
        echo "ssh $DESTNODE \"/globalhome/romein/bin.x86_64/udp-copy $FILESTREAM $OBS_INSTREAM \"" "&" >> replay.sh
        ;;
    esac

    if [ -z "$OBS_INSTREAMS" ]; then
      OBS_INSTREAMS="$OBS_INSTREAM"
    else
      OBS_INSTREAMS="$OBS_INSTREAMS,$OBS_INSTREAM"
    fi

    # The command to execute to record this field
    echo "# stream $s board $b [$INSTREAM -> $OUTSTREAM]" >> record-cobalt.sh
    echo ssh $HOST \"nice -n -20 numactl --cpunodebind=$CPU --membind=$CPU $LOFARROOT/bin/filterRSP -i $INSTREAM -o $OUTSTREAM "$FILTERPARAMS" \" "&" >> record-cobalt.sh
  done

  echo "PIC.Core.$s.RSP.ports = [$OBS_INSTREAMS]" >> replay.parset
done

# At the end of all ssh commands, wait for all of them to finish
echo wait >> record-cobalt.sh
echo wait >> record-locus.sh
echo wait >> replay.sh
