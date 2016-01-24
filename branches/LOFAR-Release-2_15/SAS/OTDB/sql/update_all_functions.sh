#!/bin/bash
#
# Script to update all sql functions in the OTDB
#
LOFAR_DB=LOFAR_4

# Gather all sql functions
all_funcs=( `ls -1 *_func.sql` )
if [ ${#all_funcs[@]} -ge 1 ]; then
  if [ -e /tmp/otdb_func_update_$$.sql ]; then 
     result=`rm -f /tmp/otdb_func_update_$$.sql >& /dev/null; echo $?`
     if [ $result -ne 0 ]; then 
        echo "Could not remove file  /tmp/otdb_func_update_$$.sql" 
	exit
     fi     
  fi
  for func in "${all_funcs[@]}" 
  do 
     echo "\i $func" >> /tmp/otdb_func_update_$$.sql 
  done
else
  echo "Could not find sql function files; wrong directory?"
  exit
fi

# Update database
if [ -e /tmp/otdb_func_update_$$.sql ]; then
  echo "Updating database $LOFAR_DB ; see file otdb_func_update_$$.log for log messages"
  psql -f /tmp/otdb_func_update_$$.sql -U postgres -h localhost $LOFAR_DB >& otdb_func_update_$$.log 
else
  echo "Could not find file /tmp/otdb_func_update_$$.sql "
  exit
fi
