#!/bin/bash -eu

echo "Giving /opt to lofarbuild..."
chown lofarbuild.lofarbuild /opt

echo "Giving /localhome/lofar to lofarbuild..."
mkdir /localhome/lofar
chown lofarbuild.lofarbuild /localhome/lofar

echo "Giving capabilities to lofarsys..."
# NOTE: the line added below needs to be inserted BEFORE 'none *'
(echo "cap_net_raw,cap_sys_nice,cap_ipc_lock lofarsys"; grep -v lofarsys /etc/security/capability.conf) > /tmp/new-capability.conf
mv /tmp/new-capability.conf /etc/security/capability.conf

#
# Casacore
#
echo "Removing system-supplied /opt/casacore..."
rm -rf /opt/casacore

#
# QPID
#
echo "Creating qpid user..."
adduser qpid --system --home /localhome/qpid --disabled-password --disabled-login
mkdir -p /localhome/qpid
chown qpid.qpid /localhome/qpid

echo "Installing /etc/init.d/qpidd..."
cp init.d-qpidd.sh /etc/init.d/qpidd
update-rc.d qpidd defaults 95 15
