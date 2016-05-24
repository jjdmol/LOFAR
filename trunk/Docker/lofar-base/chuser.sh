#!/usr/bin/env bash

# Configrue user
if [ -z "${USER}" ]; then
  export USER=${UID}
fi

# Create home directory
if [ -z "${HOME}" ]; then
  export HOME=/home/${USER}
  mkdir -p $HOME && cd $HOME
fi

# Add user to system
fgrep -q ":x:${UID}:" /etc/passwd || echo "${USER}:x:${UID}:${UID}::${HOME}:/bin/bash" >> /etc/passwd
fgrep -q ":x:${UID}:" /etc/group  || echo "${USER}:x:${UID}:" >> /etc/group

# Set the environment
[ -e /opt/bashrc ] && source /opt/bashrc

# Run the requested command
if [ -z "$*" ]; then
  exec /bin/bash
else
  exec "$@"
fi
