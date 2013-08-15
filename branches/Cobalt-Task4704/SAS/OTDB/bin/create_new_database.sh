#!/bin/bash
#
# $Id:$
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
#

SyntaxError()
{
        Msg=$1

        [ -z "${Msg}" ] || echo "ERROR: ${Msg}"
        echo ""
        echo "Syntax: $(basename $0) -D <dbname> [-S <dbhost>] [-c -v <version>] "
        echo "        [-l] [-u] [-n] [-t] [-p] [-d <sourcedbname>] [-s <sourcedbserver>]"
        echo 
        echo "  === Use from checked-out SVN tree or from installation ==="
        echo "  -l indicates that the script is run from a checked out SVN tree [false]"
        echo
        echo "  ===  Database Creation parameters ==="
        echo "  -D <DBNAME> is the database name to create (no default)"
        echo "  -S <dbhost> is the database server on which to create the database [localhost]"
        echo "  -n indicates that an already existing database is not removed [false]"
        echo "  -p load and activate the PIC tree (takes a long time) [false]"
        echo
        echo "  === OTDB Component parameters ==="
        echo "  -c indicates that new components must be loaded into the database [false]"
        echo "  -v <version> is the version number to attach to these components (no default)"
        echo
        echo "  === Default Template parameters ==="
        echo "  -t indicates that default templates must be copied from elsewhere"
        echo "  -d is the database name for the default templates [LOFAR_4]"
        echo "  -s is the database host for the default templates [sasdb]"
        echo "  -u indicates that the copied default templates must be updated to the new"
        echo "       version of the loaded component files"
        echo
        exit 0
}

HandleArgs()
{
  # Handle arguments
  if (($# == 0)); then
    echo "Provide arguments"
    SyntaxError
  fi

  while getopts  "D:S:v:clntd:s:uhp" flag
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
      p)
        load_PICtree=1
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
load_PICtree=0

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

if [ $update_default_templates -eq 1 ]; then 
   if [ $otdb_comp_version -eq 0 ]; then 
     echo "Provide a version number for the default templates to update to"
     SyntaxError
     exit
   elif [ $load_otdb_comps -eq 0 ]; then 
     echo "To update default templates also load new component files"
     SyntaxError
     exit
   elif [ $copy_default_templates -eq 0 ]; then 
     echo "Default templates must be copied or they cannot be updated"
     SyntaxError
     exit
   fi
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
  if [ $local_mode -eq 1 ]; then 
    cd ../sql
    if [ -e create_OTDB.sql ]; then 
      # ADD THE ''IF EXISTS'' TO ALL DROP TABLE COMMANDS!
      psql -f create_OTDB.sql --host $DBHOST --port $DBPORT --user $DBUSER $DBNAME 2>&1 | grep ERROR | grep -v "does not exist"
      otdb_create_result=$?
    else
      echo "Cannot find create_OTDB.sql; did you check it out?"
      exit
    fi
  else
    if [ -e /opt/lofar/sbin/sql/create_OTDB.sql ]; then 
      cd /opt/lofar/sbin/sql
      # ADD THE ''IF EXISTS'' TO ALL DROP TABLE COMMANDS!
      psql -f create_OTDB.sql --host $DBHOST --port $DBPORT --user $DBUSER $DBNAME 2>&1 | grep ERROR | grep -v "does not exist"
      otdb_create_result=$?
    else
      echo "Cannot find /opt/lofar/sbin/sql/create_OTDB.sql; did you install this?"
      exit
    fi
  fi

  if [ $otdb_create_result -ne 1 ]; then 
    echo "Problem creating new OTDB"
    exit
  fi
fi
if [ $load_PICtree -eq 1 ]; then  
    # Now load the PIC tree
    if [ $local_mode -eq 1 ]; then 
      cd ../../../MAC/Deployment/data/OTDB/
      if [ -e loadPICtree ]; then 
      # THIS CAN NOW ONLY RUN AS LOFARSYS! PERMISSION ISSUES. FIX THIS
        loadPICtree_command="./loadPICtree -l"
        loadPICtree_log="./loadPICtree.log"
      else
        echo "Cannot find loadPICtree in $PWD"
        exit
      fi
    else
      loadPICtree_exists=`which loadPICtree 2>&1 1>/dev/null | echo $?`
      if [ $loadPICtree_exists -ne 0 ]; then
        echo "Could not find executable loadPICtree"
        exit
      fi
      loadPICtree_command=`which loadPICtree`
      loadPICtree_log="/tmp/loadPICtree.log"
    fi  
    # THIS CAN NOW ONLY RUN AS LOFARSYS! PERMISSION ISSUES. FIX THIS
    echo "== Loading and activating PIC tree; this takes a long time... =="
    rm -f $loadPICtree_log || exit
    $loadPICtree_command -D $DBNAME -H $DBHOST 2>&1 1>$loadPICtree_log

    loadPICtree_result=$?
    if [ $loadPICtree_result -ne 0 ]; then 
      echo "Problem or error in loadPICtree; check logfile $loadPICtree_log"
      exit
    fi

    # Activate PIC tree

    pic_treeid=`psql -q -t -h $DBHOST -U $DBUSER -d $DBNAME -c "select treeid from gettreelist('10','0',0,'','','');"| awk '{print $1}'`
    if [ "$pic_treeid" == "" ]; then
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
else
    if [ $local_mode -eq 1 ]; then 
      cd ../../../MAC/Deployment/data/OTDB/
    fi
fi

# COPY DEFAULT TEMPLATES FROM PRODUCTION (optional)

# Needs production host and production database

if [ $copy_default_templates -eq 1 ]; then
  echo "== Copying default templates from database $default_templates_dbname on $default_templates_dbhost =="

  def_templ_prod_list=( `psql -q -t -h $default_templates_dbhost -U postgres -d $default_templates_dbname -c "select treeid from getdefaulttemplates() where name not like '#%';" | awk '{print $1}'` )
  if [ ${#def_templ_prod_list[@]} -eq 0 ]; then
    echo "Could not find valid default templates in database $prod_db on host $prod_host"
    exit
  fi
  if [ $local_mode -eq 1 ]; then 
    cd ../../../../SAS/OTDB/bin/
    if [ -e copyTree.py ]; then 
      copytree_command="./copyTree.py"
      copytree_log="./copyTree.log"
    else
      echo "Cannot find copyTree.py in $PWD"
      exit
    fi
  else 
    copytree_exists=`which copyTree.py 2>&1 1>/dev/null | echo $?`
    if [ $copytree_exists -ne 0 ]; then
      echo "Could not find executable copytree needed to copy default templates"
      exit
    fi
    copytree_command=`which copyTree.py`
    copytree_log="/tmp/copyTree.log"
  fi
  rm -f $copytree_log || exit
  for def_templ_prod_id in "${def_templ_prod_list[@]}"
  do 
    echo "Copying default template with treeID $def_templ_prod_id; this may take a while ..."
    $copytree_command -D $default_templates_dbname -S $default_templates_dbhost -d $DBNAME -s $DBHOST -t $def_templ_prod_id 2>&1 1>>$copytree_log
    copyTree_result=$?
    if [ $copyTree_result -ne 0 ]; then
      echo "Problem copying treeID $def_templ_prod_id; see file $copytree_log for details"
      exit 
    fi
  done
fi

# Load new Components (optional)

# Needs version number and switch for local usage
if [ $load_otdb_comps -eq 1 ]; then 
  if [ $local_mode -eq 1 ]; then 
    cd ../../../MAC/Deployment/data/OTDB/
    if [ -e loadComponents ]; then
      loadComponents_command="./loadComponents -l "
      loadComponents_runlog="loadComponents_run.log"
      loadComponents_outlog="loadComponents.log"
    else
      echo "Cannot find loadComponents in $PWD"
    fi
  else
    loadComponents_exists=`which loadComponents 2>&1 1>/dev/null | echo $?`
    if [ $loadComponents_exists -ne 0 ]; then
      echo "Could not find executable loadComponents to load new OTDB components"
      exit
    fi
    loadComponents_command=`which loadComponents`
    loadComponents_runlog="/tmp/loadComponents_run.log"
    loadComponents_outlog="/opt/lofar/var/log/loadComponents.log"
  fi
  echo "== Loading new OTDB components using version $otdb_comp_version =="
  rm -f $loadComponents_log || exit
  $loadComponents_command -D $DBNAME -H $DBHOST -v $otdb_comp_version 2>&1 1>$loadComponents_runlog

  loadComponents_exit=$?
  loadComponents_problem=`grep -E 'FATAL|ERROR' $loadComponents_outlog 2>&1 1>/dev/null; echo $?`

  if [[ $loadComponents_exit -ne 0 || $loadComponents_problem -eq 0 ]]; then
     echo "loadComponents failed or has an error; see $loadComponents_outlog and $loadComponents_runlog for more information"
     exit
  fi
fi

# Update default templates (optional)

if [ $copy_default_templates -eq 1 ]; then

  if [ $local_mode -eq 1 ]; then 
    cd ../../../../SAS/OTDB/bin/

    if [ -e makeDefaultTemplates.py ]; then 
      makeDefaultTemplates_command="./makeDefaultTemplates.py"
      makeDefaultTemplates_log="makeDefaultTemplates.log"
    else
      echo "Cannot find makeDefaultTemplates.py in $PWD"
      exit
    fi
  else 
    makeDefaultTemplates_exists=`which makeDefaultTemplates.py 2>&1 1>/dev/null | echo $?`
    if [ $makeDefaultTemplates_exists -ne 0 ]; then
      echo "Could not find executable makeDefaultTemplates.py to convert default templates to newly loaded OTDB components"
      exit
    fi
    makeDefaultTemplates_command=`which makeDefaultTemplates.py`
    makeDefaultTemplates_log="/tmp/makeDefaultTemplates.log"
  fi
  
  echo "== Updating default templates to new component version =="
  rm -f $makeDefaultTemplates_log || exit
  if [ $otdb_comp_version -ne 0 ]; then
    $makeDefaultTemplates_command -D $DBNAME -H $DBHOST -v$otdb_comp_version 2>&1 1>$makeDefaultTemplates_log
  else
    $makeDefaultTemplates_command -D $DBNAME -H $DBHOST 2>&1 1>$makeDefaultTemplates_log
  fi
  makeDefaultTemplates_exit=$?
  if [ $makeDefaultTemplates_exit -ne 0 ]; then 
     echo "makeDefaultTemplates failed or has an error; see $makeDefaultTemplates_log for log info"
     exit
  fi
fi

echo "== Done. New OTDB created =="
