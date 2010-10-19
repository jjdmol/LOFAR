#!/bin/sh
#
# DROPS the current database and makes a whole new test database
#
# $Id$
#

if [ -e database.py ]; then
  DATABASENAME=`grep ^dbName database.py | awk -F= '{print $2}' | sed s/\"//g`
  DATABASEHOST=`grep ^dbHost database.py | awk -F= '{print $2}' | sed s/\"//g`
else
  echo "Cannot find file database.py"
  exit 1
fi

echo -e "Using database $DATABASENAME on host $DATABASEHOST; is this ok [y/n]? \c"
read answer
if [[ ! "$answer" =~ [yY] ]]; then
  exit
fi
#
echo "DELETING AND REBUILDING DATABASE " $DATABASENAME
sleep 5
#
dropdb -h $DATABASEHOST -U postgres $DATABASENAME
createdb -h $DATABASEHOST -U postgres $DATABASENAME

echo "creating new tables"
psql -f create_CDB_base_tables.sql   -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR | grep -v 'does not exist'
psql -f create_CDB_tables.sql        -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR
psql -f create_CDB_types.sql         -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR

echo "adding functions to the database"
psql -f get_objects_func.sql         -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR
psql -f add_object_func.sql          -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR

psql -f add_reference_coord_func.sql -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR
psql -f get_ref_objects_func.sql     -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR

psql -f add_gen_coord_func.sql       -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR
psql -f get_gen_coord_func.sql       -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR

psql -f add_hba_deltas_func.sql      -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR
psql -f get_hba_deltas_func.sql      -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR

psql -f add_normal_vector_func.sql   -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR
psql -f get_normal_vector_func.sql   -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR

psql -f add_rotation_matrix_func.sql -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR
psql -f get_rotation_matrix_func.sql -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR

psql -f add_field_rotation_func.sql -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR
psql -f get_field_rotation_func.sql -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR

psql -f get_transformation_info_func.sql -h $DATABASEHOST -U postgres $DATABASENAME 2>&1 | grep ERROR