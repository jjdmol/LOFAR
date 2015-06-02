#!/bin/bash -eu

# ********************************************
#  Populate ~/.ssh directory
#
#  We draw a copy from /globalhome/lofarsystem
#  to stay in sync with the rest of LOFAR.
# ********************************************
echo "Configuring ssh..."
cp -a /globalhome/lofarsystem/.ssh ~

echo "  Testing..."
ssh localhost true >/dev/null

# ********************************************
#  Install bash initialisation scripts
# ********************************************
echo "Configuring bash..."
cp lofarsys/bashrc ~/.bashrc
cp lofarsys/bash_profile ~/.bash_profile

echo "  Validating login..."
ssh localhost true

# ********************************************
#  Create directories for LOFAR's
#  temporary files
# ********************************************
echo "Configuring /opt/lofar/var..."
mkdir -p ~/lofar/var/{log,run}
