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
dropdb -h rs005 -U postgres $DATABASENAME
createdb -h rs005 -U postgres $DATABASENAME

echo "creating new tables"
psql -f create_OTDB.sql -h rs005 -U postgres $DATABASENAME 2>&1 | grep ERROR | grep -v "does not exist"

