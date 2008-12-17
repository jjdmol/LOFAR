
#!/bin/sh

set -e
set -x

export LOFARSHM_DBNAME=lofar-shm
export LOFARSHM_DBUSER=shm2
export LOFARSHM_DBHOST=localhost
export LOFARSHM_DBPORT=5432

# The following commands assume that the user who executes this script has superuser privileges.
#
# Access to PostgreSQL databases is managed via the 'pg_hba.conf' file. In current Debian systems,
# this file can be found at /etc/postgresql/8.1/main/pg_hba.conf. Make sure you allow access
# to the database.
#
# Postgres should be told to listen to external TCP ports. Edit /etc/postgresql/8.1/main/postgresql.conf
# to have the following stanzas:
#
#     Listen_addresses = '*'
#     Port = 5432
#
# Don't forget to restart Postgres after altering these files, as follows (Debian):
#
#     invoke-rc.d postgresql-8.1 restart

# Drop DB & USER. User LOFARSHM_DBUSER owns LOFARSHM_DBNAME, so we have to remove the database first!
# Note the '|| true' at the end. This ensures that the commands succeed, even if the database is not found.

dropdb   --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --echo $LOFARSHM_DBNAME || true
dropuser --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --echo $LOFARSHM_DBUSER || true

# Create USER, DB, and add the 'plpgsql' language for stored procedures.

createuser --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --no-superuser --no-createdb --no-createrole --echo $LOFARSHM_DBUSER
createdb   --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --owner $LOFARSHM_DBUSER                     --echo $LOFARSHM_DBNAME "System Health Management database"
createlang --host $LOFARSHM_DBHOST --port $LOFARSHM_DBPORT --dbname $LOFARSHM_DBNAME                    --echo "plpgsql"

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
