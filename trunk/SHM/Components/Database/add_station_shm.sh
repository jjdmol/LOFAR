#!/bin/bash
#
# Script to add a LOFAR station to the Job Control database, so it will
# be used by the SHM system
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
    result=`psql -U${dbuser} -d${dbdatab} -h${dbhost} -p${dbport} -c "Select si_name from lofar.macinformationservers where si_name = '$station'" >& /dev/null ; echo $?`
    if [ $result != 0 ]; then 
	echo "Station "$station" already present in table lofar.macinformationservers!"
	exit 1
    fi
}

check_si_id_known()
{
    _si_id=$1
    result=`psql -U${dbuser} -d${dbdatab} -h${dbhost} -p${dbport} -c "Select si_id from systems.systeminstances where si_id = $_si_id" >& /dev/null ; echo $?`
    if [ $result != 0 ]; then 
	echo "Appointed system ID $_si_id already present in table systems.systeminstances!"
	exit 1
    fi
}


insert_station_row()
{
    station=$1
    result=`host -i ${station}C >& /dev/null; echo $?`
    if [ $result != 0 ]; then
	echo "Host ${station}C unknown"
	exit 1
    else
	host_ip=`host -i ${station}C | awk '{print $4}'`
    fi
    station_nr=`echo $station| cut -c 3-`

    let si_id=station_nr+1000
    check_si_id_known $si_id

    query="Insert into systems.systeminstances (si_id, si_name, si_description, sc_id) VALUES ($si_id, '"$station"','"$station"',3);"
    insert_row "$query"

    query="Insert into lofar.macinformationservers (si_id, mis_address, mis_port, si_name) VALUES ("$si_id",'"$host_ip"',23990,'"$station"');"
    insert_row "$query"
}
        
get_max_id()
{
    max_id=`psql -U${dbuser} -d${dbdatab} -h${dbhost} -p${dbport} -t -c "Select max(id) from job_control.queue"`
    
}

insert_row()
{
  result=`psql -U${dbuser} -d${dbdatab} -h${dbhost} -p${dbport} -c "$1" >& /dev/null ; echo $?`
  if [ $result != 0 ]; then 
      echo "Could not insert new row. Command given is:"
      echo $1
  fi
}

get_user_info()
{
    read -p "Which station to add? "
    if [ "$REPLY" != "" ]; then
	new_station=`echo $REPLY | tr '[a-z]' '[A-Z]'`
    else
	new_station='unknown'
    fi

}

# Main program
test_connection
get_user_info
check_station_known $new_station

get_max_id

curdate=`date +%Y-%m-%d\ %H:%M:%S`
let next_id=max_id+1
rsp_name="rsp_"$new_station

insert_rsp="INSERT INTO job_control.queue (id, name, scheduled_time, ticket_no, state, client, time_of_bind, watchdog, wd_timeout, period, command) VALUES (nextval('job_control.queue_id_seq'),'"$rsp_name"', '"${curdate}"',1, 0, NULL, NULL, NULL, '00:07:00', '00:06:00', 'fetch-rspstatus-daemon.v2.py -c -j "${new_station}"');"
insert_row "$insert_rsp"

#
sb_name="sb_"$new_station
let next_id+=1
insert_sb="INSERT INTO job_control.queue (id, name, scheduled_time, ticket_no, state, client, time_of_bind, watchdog, wd_timeout, period, command) VALUES (nextval('job_control.queue_id_seq'),'"$sb_name"', '"${curdate}"',1, 0, NULL, NULL, NULL, '00:07:00', '00:06:00', 'fetch-sbstats-daemon.v2.py -c -j "${new_station}"');"
insert_row "$insert_sb"

acm_name="acm_"$new_station
let next_id+=1
insert_acm="INSERT INTO job_control.queue (id, name, scheduled_time, ticket_no, state, client, time_of_bind, watchdog, wd_timeout, period, command) VALUES (nextval('job_control.queue_id_seq'),'"$acm_name"', '"${curdate}"', 1, 0, NULL, NULL, NULL, '00:04:00', '00:03:01', 'fetch-acm-daemon.v2.py -c -j -s 281 "${new_station}"');"
insert_row "$insert_acm"

diag_name="dia_"$new_station
let next_id+=1
insert_diag="INSERT INTO job_control.queue (id, name, scheduled_time, ticket_no, state, client, time_of_bind, watchdog, wd_timeout, period, command) VALUES (nextval('job_control.queue_id_seq'),'"$diag_name"', '"${curdate}"', 1, 0, NULL, NULL, NULL, '00:11:00', '00:10:01', 'do_diagnosis.v2.py "${new_station}"');"
insert_row "$insert_diag"

insert_station_row $new_station

echo "Done!"



