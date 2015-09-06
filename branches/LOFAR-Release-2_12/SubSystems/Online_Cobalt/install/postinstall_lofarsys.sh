#!/bin/bash -eu

# NOTE: '~' may belong to a different user, due to intricacies of sudo.
# So use '~lofarsys' instead.

# ********************************************
#  Populate ~/.ssh directory
#
#  We draw a copy from /globalhome/lofarsystem
#  to stay in sync with the rest of LOFAR.
# ********************************************
echo "Configuring ssh..."
cp -a /globalhome/lofarsystem/.ssh ~lofarsys

echo "  Testing..."
ssh localhost true >/dev/null

# ********************************************
#  Install bash initialisation scripts
# ********************************************
echo "Configuring bash..."
cp lofarsys/bashrc ~lofarsys/.bashrc
cp lofarsys/bash_profile ~lofarsys/.bash_profile

echo "  Validating login..."
ssh localhost true

# ********************************************
#  Create directories for LOFAR's
#  temporary files
# ********************************************
echo "Configuring /opt/lofar/var..."
mkdir -p ~lofarsys/lofar/var/{log,run}
