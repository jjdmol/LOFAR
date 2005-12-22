#
# DROPS the current database and make a whole new test database
#
DATABASENAME=otdbtest
#
echo "DELETING AND REBUILDING DATABASE " $DATABASENAME
sleep 5
#
dropdb -h dop50 -U postgres $DATABASENAME
createdb -h dop50 -U postgres $DATABASENAME

echo "creating new tables"
psql -f create_OTDB.sql -h dop50 -U postgres $DATABASENAME 2>&1 | grep ERROR | grep -v "does not exist"

