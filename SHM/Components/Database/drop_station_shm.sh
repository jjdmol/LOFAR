#!/bin/bash
#
# $Id:$
#
# Script to drop a LOFAR station from the Job Control database, so it will
# not be used anymore by the SHM system
#
# Station is dropped from :
# - lofar.macinformationservers -> used for web-interface etc.
# - jobcontrol.queue -> executes data fetching
#
# The station will remain in 
# - systems.systeminstances -> List of all stations with unique ID
#

# This exits the script if an error occurs
set -o errexit

dbhost='sas001.lofar'
dbport='5432'
dbuser='shm2'
dbdatab='lofar-shm'

# test connection:
test_connection() 
{
    echo -n "Testing connection ... "
    result=`psql -U${dbuser} -d${dbdatab} -h${dbhost} -p${dbport} -c "Select max(id) from job_control.queue" >& /dev/null ; echo $?`


    if [ $result != 0 ]; then 
	echo
	echo "Could not connect to SHM jobcontrol database using these parameters:"
	echo "  Host: $dbhost"
	echo "  Port: $dbport"
	echo "  User: $dbuser"
	echo "  DB:   $dbdatab"
    else 
	echo "Done!"
    fi
}

check_station_known()
{
    station=$1
    result=`psql -U${dbuser} -d${dbdatab} -h${dbhost} -p${dbport} -c "Select si_name from lofar.macinformationservers where si_name = '$station'" | grep "1 row" >& /dev/null ; echo $?`
    if [ $result != 0 ]; then 
        echo "Station $station unknown in table lofar.macinformationservers!"
        exit 1
    fi
}

drop_row()
{
  result=`psql -U${dbuser} -d${dbdatab} -h${dbhost} -p${dbport} -c "$1" >& /dev/null ; echo $?`
  if [ $result != 0 ]; then 
      echo "Could not drop row. Command given is:"
      echo $1
  fi
}

get_user_info()
{
    read -p "Which station to drop? "
    if [ "$REPLY" != "" ]; then
	the_station=`echo $REPLY | tr '[a-z]' '[A-Z]'`
    else
	the_station='unknown'
    fi

}

# Main program
test_connection
get_user_info
check_station_known $the_station

drop_MIS="DELETE from lofar.macinformationservers where si_name = '"$the_station"';"
drop_row "$drop_MIS"
 
rsp_name="rsp_"$the_station
drop_rsp="DELETE from job_control.queue where name = '"$rsp_name"';"
drop_row "$drop_rsp"

#
sb_name="sb_"$the_station
drop_sb="DELETE from job_control.queue where name = '"$sb_name"';"
drop_row "$drop_sb"

acm_name="acm_"$the_station
drop_acm="DELETE from job_control.queue where name = '"$acm_name"';"
drop_row "$drop_acm"

diag_name="dia_"$the_station
drop_diag="DELETE from job_control.queue where name = '"$diag_name"';"
drop_row "$drop_diag"

echo "Done!"



