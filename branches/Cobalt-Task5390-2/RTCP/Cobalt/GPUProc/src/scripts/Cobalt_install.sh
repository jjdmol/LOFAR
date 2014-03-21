#!/bin/sh
# Install {RELEASE_NAME}.ztar from the NEXUS onto cbt nodes.
#
# Note: the 'current' symlink still has to be repointed on all cbt nodes to use the new release by default!
#
# $Id$

username=lofarbuild
effective_u=$(whoami)
if [ "$effective_u" != "$username" ]; then
  echo "ERROR: script must be run as $username"
  exit 1
fi
unset username

# location of the file in the NEXUS
NEXUS_URL=https://support.astron.nl/nexus/content/repositories/releases/nl/astron/lofar/cobalt/${RELEASE_NAME}.ztar
if [ "${RELEASE_NAME}" = "" ]; then
  echo "ERROR: RELEASE_NAME is not set or empty. Needed to download archive to install"
  exit 1
fi

nhosts=9
for ((h = 1; h <= $nhosts; h++)); do
  host=$(printf cbm%03u $h)
  echo "ssh-ing to node $host"

  # Escape double quotes below the following line!
  ssh $host "
  pwd

  # Create temp location for incoming tar file
  dl_dir=/localhome/lofarbuild/incoming
  mkdir -p $dl_dir && cd $dl_dir || exit 1

  # Download archive from NEXUS. -N: clobber existing files
  wget -N --tries=3 --no-check-certificate \"${NEXUS_URL}\" || exit 1

  # The full pathnames are in the tar file, so unpack from root dir.
  # -m: don't warn on timestamping /localhome
  cd / && tar -zxvmf \"/localhome/lofarbuild/incoming/${RELEASE_NAME}.ztar\" || exit 1

  # Sym link installed var/ to common location.
  cd \"/localhome/lofar/lofar_versions/${RELEASE_NAME}\" && \
    ln -sfT /localhome/lofarsystem/lofar/var var

  # Set capabilities so our soft real-time programs can elevate prios.
  COBALT_CAPABILITIES='cap_sys_admin,cap_sys_nice,cap_ipc_lock'
#disabled until we've updated /etc/sudoers to allow lofarbuild to do this
#also, we don't need cap_sys_admin and should drop it, idem on CEP2
  #sudo /sbin/setcap \"${COBALT_CAPABILITIES}\"=ep bin/rtcp bin/outputProc
  " || exit 1
done
