#!/bin/bash -eu

echo "Giving /opt to lofarbuild..."
chown lofarbuild.lofarbuild /opt

#
# Casacore
#
echo "Removing system-supplied /opt/casacore..."
rm -rf /opt/casacore

#
# QPID
#
echo "Creating /data symlink..."
ln -sfT /localhome /data

echo "Creating qpid datadir..."
mkdir -p /localhome/qpid
chown qpid.qpid /localhome/qpid

echo "Installing /etc/init.d/qpidd..."
cp init.d-qpidd.sh /etc/init.d/qpidd
update-rc.d qpidd defaults 95 15
