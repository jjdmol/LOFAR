#!/bin/sh
#
# DROPS the current database and makes a whole new test database
#
# $Id$
#
DATABASENAME=coordtest
if [ $# -eq 1 ]; then
	DATABASENAME=$1
fi

#
echo "DELETING AND REBUILDING DATABASE " $DATABASENAME
sleep 5
#
dropdb -h dop50 -U postgres $DATABASENAME
createdb -h dop50 -U postgres $DATABASENAME

echo "creating new tables"
psql -f create_CDB_base_tables.sql -h dop50 -U postgres $DATABASENAME 2>&1 | grep ERROR | grep -v 'does not exist'
psql -f create_CDB_tables.sql -h dop50 -U postgres $DATABASENAME 2>&1 | grep ERROR

