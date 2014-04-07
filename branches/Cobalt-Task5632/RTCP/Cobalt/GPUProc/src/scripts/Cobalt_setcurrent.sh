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
nhosts=9
for ((h = 1; h <= $nhosts; h++)); do
  ssh $(printf cbm%03u $h) "ln -sfT \"/localhome/lofar/lofar_versions/${RELEASE_NAME}\" /localhome/lofarsystem/lofar/current" || exit 1
done

# Sanity check. Assume $RELEASE_NAME is something like Cobalt-RELTYPE-x_y_z. Check for "x.y".
release_version_str=$(echo "${RELEASE_NAME}" | cut -f 3 -d -)
release_version_maj=$(echo "$release_version_str" | cut -f 1 -d _)
release_version_min=$(echo "$release_version_str" | cut -f 2 -d _)
release_version="$release_version_maj.$release_version_min"
source /opt/lofar/lofarinit.sh || exit 1
rtcp -h | grep "$release_version" > /dev/null && outputProc -h | grep "$release_version" > /dev/null
if [ $? -ne 0 ]; then
  echo "WARNING: version printed by rtcp -h or outputProc -h does not contain expected version number $release_version"
  exit 1
fi
