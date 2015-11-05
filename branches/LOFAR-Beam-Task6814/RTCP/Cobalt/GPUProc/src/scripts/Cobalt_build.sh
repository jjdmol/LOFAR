#!/bin/sh
# Build Online_Cobalt for later production roll-out to cbt nodes and upload archive to NEXUS.
#
# $Id$

# location of the file in the NEXUS
NEXUS_URL=https://support.astron.nl/nexus/content/repositories/releases/nl/astron/lofar/cobalt/${RELEASE_NAME}.ztar
if [ "${RELEASE_NAME}" = "" ]; then
  echo "ERROR: RELEASE_NAME (branch name) is not set or empty. Needed to create archive and upload to NEXUS"
  exit 1
fi

BRANCH=${RELEASE_NAME}

# Update or check out branch.
cd /globalhome/lofarbuild/lofar/lofar_versions || exit 1
if svn info "$BRANCH/LOFAR" 2> /dev/null | grep LOFAR > /dev/null; then
  svn up "$BRANCH/LOFAR" || exit 1
else
  svn co "https://svn.astron.nl/LOFAR/branches/$BRANCH" "$BRANCH/LOFAR" || exit 1
fi

# Configure and build.
cd $BRANCH && rm -rf gnu_opt && mkdir gnu_opt && cd gnu_opt &&
  cmake -DBUILD_PACKAGES=Online_Cobalt "-DCMAKE_INSTALL_PREFIX=/localhome/lofar/lofar_versions/$BRANCH" ../LOFAR &&
  make -j 16 &&
  make install "DESTDIR=/globalhome/lofarbuild/lofar/lofar_versions/$BRANCH" || exit 1

# Tar the localhome directory and upload archive to the NEXUS content server.
tar -czf "${BRANCH}.ztar" localhome &&
  curl --insecure --upload-file "${BRANCH}.ztar" -u upload:upload "${NEXUS_URL}"

