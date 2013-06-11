#!/bin/bash
#
# Script to create a OTDB database from scratch, using the sql statements
#
# Usage: create_new_database DBNAME <DBHOST>
#
# Requires all sql-commands to be present in /opt/lofar/sbin/sql
# If existing, DROPS the current database
#

# TO DO: Add optionparser for options -d en -H (and -h for help)
#        Add option -l for local (using a checked out SVN tree)
#        Add option -v for version of new components 
#        Add switches for:
#         - NOT Recreating database if exists; otherwise assume database has PIC tree, all functions, default templates, components
#         - Copying default templates (requires production host and database)
#         - Loading new version of components (needs version number)
#         - Update existing default templates to version of loaded components


if [ -z $1 ]; then
  echo "Usage: fresh_database <DBNAME> [<DBHOST>]"
  echo "  DBHOST defaults to localhost"
  exit 0
fi

if [ "$1" == "-h" ]; then 
  echo "Usage: fresh_database <DBNAME>"
  exit 0
fi

DBNAME=$1

if [ -z $2 ]; then 
  DBHOST=localhost
else
  DBHOST=$2
fi

DBUSER=postgres
DBPORT=5432

# The following commands assume that the user has super user privileges.
#
# Access to PostgreSQL databases is managed via the 'pg_hba.conf' file. 
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
# Drop DB & USER. User DBUSER owns DBNAME, so we have to remove the database 
# first!
# Note the '|| true' at the end. This ensures that the commands succeed, 
# even if the database is not found.


echo " == Dropping possibly existing database $DBNAME on server $DBHOST =="
#dropdb   --host $DBHOST --port $DBPORT --user postgres --echo $DBNAME || exit

# Create USER, DB, and add the 'plpgsql' language for stored procedures.

echo " == Creating new database $DBNAME =="
#createdb   --host $DBHOST --port $DBPORT --user postgres --owner $DBUSER --echo $DBNAME || exit

echo " == Creating language pl/pgsql if it doesn't exists =="
#createlang --host $DBHOST --port $DBPORT --user postgres --dbname $DBNAME  --echo "plpgsql" 

echo " == Creating tables and functions in $DBNAME =="

# TO DO: Add option to create from checked out SVN tree.

if [ -e /opt/lofar/sbin/sql/create_OTDB.sql ]; then 
  cd /opt/lofar/sbin/sql
  # ADD THE ''IF EXISTS'' TO ALL DROP TABLE COMMANDS!
#  psql -f create_OTDB.sql --host $DBHOST --port $DBPORT --user $DBUSER $DBNAME 2>&1 
#| grep ERROR | grep -v "does not exist"
else
  echo "Cannot find /opt/lofar/sbin/sql/create_OTDB.sql; did you install this?"
  exit
fi

# Now load the PIC tree

if [ -e /opt/lofar/sbin/loadPICtree ]; then 
# THIS CAN NOW ONLY RUN AS LOFARSYS! PERMISSION ISSUES. FIX THIS
#    /opt/lofar/sbin/loadPICtree -D $DBNAME -H $DBHOST
  uptime
else
  echo "Cannot find /opt/lofar/sbin/loadPICtree; did you install this?"
  exit
fi

# Activate PIC tree

pic_treeid=`psql -q -t -h $DBHOST -U $DBUSER -d $DBNAME -c "select treeid from gettreelist('10','0',0,'','','');"| awk '{print $1}'`
if [ $pic_treeid == "" ]; then
  echo "Could not find PIC tree in database $DBNAME"
  exit
fi

pic_set_active=`psql -q -t -h $DBHOST -U $DBUSER -d $DBNAME -c "select * from settreestate('1',$pic_treeid,'600');"`
if [ $? -ne 0 ]; then
  echo "Could not set status of PIC tree $pic_treeid to active"
  exit
fi

pic_set_operational=`psql -q -t -h $DBHOST -U $DBUSER -d $DBNAME -c "select * from classify('1',$pic_treeid,'3');"`
if [ $? -ne 0 ]; then
  echo "Could not set classification of PIC tree $pic_treeid to operational"
  exit
fi

# COPY DEFAULT TEMPLATES FROM PRODUCTION (optional)

# Needs production host and production database

prod_host=10.149.32.3
prod_db=LOFAR_4

def_templ_prod_list=( `psql -q -t -h $prod_host -U postgres -d $prod_db -c "select treeid from getdefaulttemplates() where name not like '#%';" | awk '{print $1}'` )
if [ ${#def_templ_prod_list[@]} -eq 0 ]; then
  echo "Could not find valid default templates in database $prod_db on host $prod_host"
  exit
fi

copytree_exists=`which copyTree.py 2>&1 1>/dev/null | echo $?`
if [ $copytree_exists -ne 0 ]; then
  echo "Could not find executable copytree needed to copy default templates"
  exit
fi

for def_templ_prod_id in "${def_templ_prod_list[@]}"
do 
   echo "Copying default template with treeID $def_templ_prod_id; this may take a while ..."
   copyTree.py -D $prod_db -S $prod_host -d $DBNAME -s $DBHOST -t $def_templ_prod_id 2>&1 1>/tmp/create_db_copytemplates.log
   result=$?
   if [ $result -ne 0 ]; then
     echo "Problem copying treeID $def_templ_prod_id; see file /tmp/create_db_copytemplates.log for details"
     exit 
   fi
done
echo "Ready copying default templates"

# Load new Components (optional)

# Needs version number and switch for local usage

loadComponents_exists=`which loadComponents 2>&1 1>/dev/null | echo $?`
if [ $loadComponents_exists -ne 0 ]; then
  echo "Could not find executable loadComponents to load new OTDB components"
  exit
fi

if [ $isLocal ]; then
  loadComponents -D $DBNAME -H $DBHOST -v 99999 -l
else 
  loadComponents -D $DBNAME -H $DBHOST -v 99999
fi
loadComponents_result=$?


if [ $? -ne 0 ]; then

# Update defulet templates (optional)

# Needs version number
makeDefaultTemplates_exists=`which makeDefaultTemplates.py 2>&1 1>/dev/null | echo $?`
if [ $makeDefaultTemplates_exists -ne 0 ]; then
  echo "Could not find executable makeDefaultTemplates.py to convert default templates to newly loaded OTDB components"
  exit
fi

makeDefaultTemplates.py -D $DBNAME -H $DBHOST -v 99999
