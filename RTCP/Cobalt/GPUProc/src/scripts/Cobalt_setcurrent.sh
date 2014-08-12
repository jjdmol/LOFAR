#!/bin/sh
# Set the 'current' symlink to the ${RELEASE_NAME} release on all cbt nodes, so it is used by default.
#
# $Id$

username=lofarsys
if [ "$USER" != "$username" ]; then
  echo "ERROR: script must be run as $username"
  exit 1
fi
unset username

if [ "${RELEASE_NAME}" = "" ]; then
  echo "ERROR: RELEASE_NAME is not set or empty."
  exit 1
fi

# Make indicated release 'current'.
for HOST in ${HOSTS:-cbm001 cbm002 cbm003 cbm004 cbm005 cbm006 cbm007 cbm008}; do
  ssh $HOST "ln -sfT \"/localhome/lofar/lofar_versions/${RELEASE_NAME}\" /localhome/lofarsystem/lofar/current" || exit 1
done
