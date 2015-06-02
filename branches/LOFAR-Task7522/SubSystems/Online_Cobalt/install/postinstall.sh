#!/bin/bash -eu

sudo -n true >&/dev/null || error "Need to be root or have sudo rights active (run 'sudo true' first)."

echo "[Postinstall as root]"
sudo -u root ./postinstall_root

echo "[Postinstall as lofarbuild]"
sudo -u lofarbuild ./postinstall_lofarbuild

echo "[Postinstall as lofarsys]"
sudo -u lofarsys ./postinstall_lofarsys

echo "[Postinstall (final) as root]"
sudo -u root ./postinstall_root_final

echo "[Done]"
