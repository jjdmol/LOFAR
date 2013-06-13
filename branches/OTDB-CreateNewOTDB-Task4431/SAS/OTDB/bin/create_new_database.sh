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

SyntaxError()
{
        Msg=$1

        [ -z "${Msg}" ] || echo "ERROR: ${Msg}"
        echo ""
        echo "Syntax: $(basename $0) -D <dbname> [-S <dbhost>] [-c -v <version>] "
        echo "        [-l] [-n] [-t] [-d <sourcedbname>] [-s <sourcedbserver>]"
        echo 
        echo "  -D <DBNAME> is the database name to create (no default)"
        echo "  -S <dbhost> is the database server on which to create the database [localhost]"
        echo "  -n indicates that an already existing database is not removed [false]"
        echo "  -c indicates that new components must be loaded into the database [false]"
        echo "  -v <version> is the version number to attach to these components (no default)"
        echo "  -t indicates that default templates must be copied from elsewhere"
        echo "  -d is the database name for the default templates [LOFAR_4]"
        echo "  -s is the database host for the default templates [sasdb]"
        echo "  -u indicates that the copies default templates must be updates to the new"
        echo "       version of the loaded component files"
        echo "  -l indicates that the script is run from a checked out SVN tree [false]"

        exit 0
}

HandleArgs()
{
  # Handle arguments
  if (($# == 0)); then
    echo "Provide arguments"
    SyntaxError
  fi

  while getopts  "D:S:v:clntd:s:uh" flag
    do
      case "$flag" in
      D)
        DBNAME=$OPTARG
        ;;
      S)
        DBHOST=$OPTARG
        ;;
      v)
        otdb_comp_version=$OPTARG
        ;;
      c)
	load_otdb_comps=1
        ;;
      l)
        local_mode=1
        ;;
      n)
        not_create_otdb_if_exists=1
        ;;
      t)
        copy_default_templates=1
        ;;
      d)
        default_templates_dbname=$OPTARG
        ;;
      s)
	default_templates_dbhost=$OPTARG
        ;;

      u)
        update_default_templates=1
        ;;
      h)
        SyntaxError 
        ;;
      *)
        SyntaxError
        ;;
      esac
    done
}

#=== MAIN ===

DBNAME=""
DBHOST=localhost
DBUSER=postgres
DBPORT=5432
load_otdb_comps=0
otdb_comp_version=0
local_mode=0
not_create_otdb_if_exists=0
copy_default_templates=0
default_templates_dbname="LOFAR_4"
default_templates_dbhost="sasdb"
update_default_templates=0

HandleArgs $* 

if [ "$DBNAME" == "" ]; then 
   echo "Provide database name to create"
   SyntaxError
   exit
fi

if [[ $load_otdb_comps -eq 1 && $otdb_comp_version -eq 0 ]]; then 
   echo "Provide a version number for the components to load"
   SyntaxError
   exit
fi

if [[ $update_default_templates -eq 1 && $otdb_comp_version -eq 0 ]]; then 
   echo "Provide a version number for the default templates to update to"
   SyntaxError
   exit
fi


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

if [ $not_create_otdb_if_exists -eq 0 ]; then 
  echo " == Dropping possibly existing database $DBNAME on server $DBHOST =="
  dropdb --host $DBHOST --port $DBPORT --user postgres $DBNAME >& /dev/null

# Create USER, DB, and add the 'plpgsql' language for stored procedures.

  echo " == Creating new database $DBNAME =="
  createdb --host $DBHOST --port $DBPORT --user postgres --owner $DBUSER $DBNAME || exit

  echo " == Creating language pl/pgsql if it doesn't exists =="
  createlang --host $DBHOST --port $DBPORT --user postgres --dbname $DBNAME "plpgsql" 

  echo " == Creating tables and functions in $DBNAME =="

# TO DO: Add option to create from checked out SVN tree.

  if [ -e /opt/lofar/sbin/sql/create_OTDB.sql ]; then 
    cd /opt/lofar/sbin/sql
    # ADD THE ''IF EXISTS'' TO ALL DROP TABLE COMMANDS!
    psql -f create_OTDB.sql --host $DBHOST --port $DBPORT --user $DBUSER $DBNAME 2>&1 | grep ERROR | grep -v "does not exist"
  else
    echo "Cannot find /opt/lofar/sbin/sql/create_OTDB.sql; did you install this?"
    exit
  fi

  # Now load the PIC tree

  if [ -e /opt/lofar/sbin/loadPICtree ]; then 
  # THIS CAN NOW ONLY RUN AS LOFARSYS! PERMISSION ISSUES. FIX THIS
    /opt/lofar/sbin/loadPICtree -D $DBNAME -H $DBHOST
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
fi
# COPY DEFAULT TEMPLATES FROM PRODUCTION (optional)

# Needs production host and production database

if [ $copy_default_templates -eq 1 ]; then
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
fi

# Load new Components (optional)

# Needs version number and switch for local usage
if [ $load_otdb_comps -eq 1 ]; then 
  loadComponents_exists=`which loadComponents 2>&1 1>/dev/null | echo $?`
  if [ $loadComponents_exists -ne 0 ]; then
    echo "Could not find executable loadComponents to load new OTDB components"
    exit
  fi

  if [ $local_mode -eq 1 ]; then
    loadComponents -D $DBNAME -H $DBHOST -v $otdb_comp_version -l > /tmp/loadComponents.log
  else
    loadComponents -D $DBNAME -H $DBHOST -v $otdb_comp_version > /tmp/loadComponents.log
  fi
  loadComponents_exit=$?
  loadComponents_problem=`grep -E 'FATAL|ERROR' /opt/lofar/var/log/loadComponents.log >& /dev/null; echo $?`

  if [ $loadComponents_exit -ne 0 || $loadComponents_problem -eq 0 ]; then
     echo "loadComponents failed or has an error"
     exit
  fi
fi

# Update default templates (optional)

if [ $copy_default_templates -eq 1 ]; then
  
  makeDefaultTemplates_exists=`which makeDefaultTemplates.py 2>&1 1>/dev/null | echo $?`
  if [ $makeDefaultTemplates_exists -ne 0 ]; then
    echo "Could not find executable makeDefaultTemplates.py to convert default templates to newly loaded OTDB components"
    exit
  fi

  if [ $otdb_comp_version -ne 0 ]; then 
    makeDefaultTemplates.py -D $DBNAME -H $DBHOST -v$otdb_comp_version 
  else
    makeDefaultTemplates.py -D $DBNAME -H $DBHOST
  fi
  makeDefaultTemplates_exit=$?
  if [ $makeDefaultTemplates_exit -ne 0 ]; then 
     echo "makeDefaultTemplates failed or has an error"
     exit
  fi
fi

