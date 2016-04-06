#!/usr/bin/env bash

# Fetch the user name used in this container
export USER=${SUDO_USER}

if [ -n "${LUSER}" ]; then
  if [ -z "${LGROUP}" ]; then
    LGROUP=${LUSER}
  fi

  OLDID=`id -u ${USER}`

  # Replace USER by LUSER:LGROUP
  sed -i -e "s/${USER}:x:[0-9]\+:[0-9]\+/${USER}:x:${LUSER}:${LGROUP}/g" /etc/passwd
  sed -i -e "s/${USER}:x:[0-9]\+:/${USER}:x:${LGROUP}:/g" /etc/group

  # Set ownership of home dir to new user
  chown --from=${OLDID} -R ${LUSER}:${LGROUP} ${HOME}

  # Set ownership of installed software to new user
  chown --from=${OLDID} -R ${LUSER}:${LGROUP} /opt
fi

# Update environment for updated user
export HOME=/home/${USER}
touch -a $HOME/.bashrc
source $HOME/.bashrc

# Use exec to make sure we propagate signals
exec sudo -u ${USER} -E "$@"
