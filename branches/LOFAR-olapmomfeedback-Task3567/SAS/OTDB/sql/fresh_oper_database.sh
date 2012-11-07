#!/bin/sh
#
# DROPS the current database and make a whole new test database
#
DATABASENAME=CDR
if [ $# -eq 1 ]; then
	DATABASENAME=$1
fi

#
echo "DELETING AND REBUILDING DATABASE " $DATABASENAME
sleep 5
#
dropdb -h sas001 -U lofarsystem $DATABASENAME
createdb -h sas001 -U lofarsystem $DATABASENAME

echo "creating new tables"
psql -f create_OTDB.sql -h sas001 -U lofarsystem $DATABASENAME 2>&1 | grep ERROR | grep -v "does not exist"

