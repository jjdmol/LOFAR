#!/bin/bash -eu

function error {
  echo "$@" >&2
  exit 1
}

sudo -n true >&/dev/null || error "Need to be root or have sudo rights active (run 'sudo true' first)."

echo "[Postinstall as root]"
sudo -u root ./postinstall_root.sh

echo "[Postinstall as lofarbuild]"
sudo -u lofarbuild ./postinstall_lofarbuild.sh

echo "[Postinstall as lofarsys]"
sudo -u lofarsys ./postinstall_lofarsys.sh

echo "[Postinstall (final) as root]"
sudo -u root ./postinstall_root_final.sh

echo "[Done]"
