#!/bin/sh
#
# DROPS the current database and make a whole new test database
#

set -e
set -x

export DBNAME=LOFAR_1
export DBUSER=postgres
export DBHOST=sas001
export DBPORT=5432

# The following commands assume that the user who executes this script has super user 
# privileges.
#
# Access to PostgreSQL databases is managed via the 'pg_hba.conf' file. In current Debian 
# systems, this file can be found at /etc/postgresql/8.1/main/pg_hba.conf. Make sure youallow 
# access to the database.
#
# Postgres should be told to listen to external TCP ports. 
# Edit /sasdb/data/postgresq.conf
# to have the following stanzas:
#
#     Listen_addresses = '*'
#     Port = 5432
#
# Don't forget to restart Postgres after altering these files, 
# as follows (Debian):
#
#  pg_ctl start -D /sasdb/data -l /sasdb/data/pg_log/logfile
#
# Drop DB & USER. User DBUSER owns DBNAME, so we have to remove the database first!
# Note the '|| true' at the end. This ensures that the commands succeed, 
# even if the database is not found.

dropdb   --host $DBHOST --port $DBPORT --user postgres --echo $DBNAME || true
# Create USER, DB, and add the 'plpgsql' language for stored procedures.

createdb   --host $DBHOST --port $DBPORT --user postgres --owner $DBUSER --echo $DBNAME
createlang --host $DBHOST --port $DBPORT --user postgres --dbname $DBNAME  --echo "plpgsql"

echo "creating new tables"
psql -f create_OTDB.sql --host $DBHOST --port $DBPORT --user $DBUSER $DBNAME 2>&1 | grep ERROR | grep -v "does not exist"
