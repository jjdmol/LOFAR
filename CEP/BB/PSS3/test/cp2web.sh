#!/bin/sh

WEBHOST=lofar0
WEBDIR=/usr/local/httpd/htdocs/BBSTestResults
TESTNAME="`date +%y%m%d`-`hostname`"
USER=`whoami`

while [ 1 = 1 ]
do
  if [ "$1" = "-host" ]; then
    shift
    WEBHOST="$1"
    shift
  elif [ "$1" = "-user" ]; then
    shift
    USER="$1"
    shift
  elif [ "$1" = "-tag" ]; then
    shift
    TESTNAME="${1}-${TESTNAME}"
    shift
  else
    break;
  fi
done

# make directory on webserver
ssh ${USER}@${WEBHOST} mkdir ${WEBDIR}/$TESTNAME
# copy files to the webserver
scp test*.out ${USER}@${WEBHOST}:${WEBDIR}/${TESTNAME}/
# generate html-files on webserver
ssh ${USER}@${WEBHOST} "cd ${WEBDIR}; ./analyze.py ${TESTNAME}"
 
