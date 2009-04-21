#!/bin/sh

set -e
set -x

# Leave empty for pg_default tablespace
export LOFARSHM_DBLOCATION=/shmdb/shmtest

export LOFARSHM_DBNAME=lofar-shmtest
export LOFARSHM_DBUSER=shm2
export LOFARSHM_DBHOST=localhost
export LOFARSHM_DBPORT=5432

# The following commands assume that the user who executes this script 
# has PostgreSQL superuser privileges (user postgres, usually).
#
# Access to PostgreSQL databases is managed via the 'pg_hba.conf' file. 
# Normally this would be in /var/lib/pgsql/data (SuSe-like systems), or in 
# /etc/postgresql/8.1/main/pg_hba.conf (Debian-like). Make sure you allow 
# access to the database.
#
# Postgres should be told to listen to external TCP ports. Edit postgresql.conf
# In the same directory to have the following stanzas:
#
#     Listen_addresses = '*'
#     Port = 5432
#
# Don't forget to restart Postgres after altering these files, as follows 
#
# Debian:
#     invoke-rc.d postgresql-8.1 restart
#
# SuSe:
#     /etc/init.d/postgresql restart     or
#     /usr/bin/pg_ctl restart
#
# We have allowed the use a separate tablespace (datadir) for the database.
# This allows to install SHM on a separate partition on an existing system
# that also hosts other databases. 
#
# Drop DB & USER. User LOFARSHM_DBUSER owns LOFARSHM_DBNAME, so we have to 
# remove the database first!
# Note the '|| true' at the end. This ensures that the commands succeed, 
# even if the database is not found or other errors occur.

dropdb   --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --echo $LOFARSHM_DBNAME || true
dropuser --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --echo $LOFARSHM_DBUSER || true
if [ $LOFARSHM_DBLOCATION != "" ]; then 
  psql --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT -c "DROP TABLESPACE shmtablespace;" || true
fi

# Create USER, DB, and add the 'plpgsql' language for stored procedures.
if [ $LOFARSHM_DBLOCATION != "" ]; then 
   psql --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT -c "CREATE TABLESPACE shmtablespace LOCATION '${LOFARSHM_DBLOCATION}'"
fi
createuser --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --no-superuser --no-createdb --no-createrole --echo $LOFARSHM_DBUSER || true
if [ $LOFARSHM_DBLOCATION != "" ]; then 
   createdb --tablespace shmtablespace --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --owner $LOFARSHM_DBUSER --echo $LOFARSHM_DBNAME "System Health Management database" || true
else 
   createdb --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --owner $LOFARSHM_DBUSER --echo $LOFARSHM_DBNAME "System Health Management database" || true  
fi
createlang --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --dbname $LOFARSHM_DBNAME --echo "plpgsql" || true

# Create DB tables and functions

SQLSOURCES="create-schema-systems.sql         \
            create-schema-observations.sql    \
            create-schema-lofar.sql           \
            create-schema-job_control.sql     \
            create-functions-observations.sql \
            create-functions-job_control.sql"

for SQLSOURCE in $SQLSOURCES ; do
    psql --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --user $LOFARSHM_DBUSER --dbname $LOFARSHM_DBNAME -f $SQLSOURCE
done

# Initialize Observations DB.

./initdb.py | psql --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --user $LOFARSHM_DBUSER --dbname $LOFARSHM_DBNAME --echo-queries
