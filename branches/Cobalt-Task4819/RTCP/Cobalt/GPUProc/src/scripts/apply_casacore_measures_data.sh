#!/bin/sh
# apply_casacore_measures_tables.sh
# Install downloaded casacore measures tables atomically. Written for jenkins@fs5 (DAS-4).
#
# $Id$

# Keep these in sync with get_casacore_measures_tables.sh
working_dir=$HOME/root/share/aips++  
dir_prefix=measures_data-


# find the latest
cd $working_dir
if [ ! -d $dir_prefix* ]; then
  echo "No casacore measures directory recognized"
  findmeastable
  exit
fi
update_id=`ls -d $dir_prefix* | tail -n 1`

# Switch data/ symlink to the latest data/ _atomically_ with a rename. Avoids race with a reader.
ln -s $update_id/data data_${update_id}_tmp && mv -Tf data_${update_id}_tmp data && sync

# See if casacore uses the latest tables.
# rv gets the path in: Measures table Observatories found as /home/jenkins/root/share/aips++/data/geodetic/Observatories
used_dir=`findmeastable | awk -F\  '{print $NF}'`
status=$?
echo "findmeastable reports using: $used_dir"
if [ $status -ne 0 || "$used_dir" != "$working_dir/$update_id/data/geodetic/Observatories" ]; then
  exit 1
fi

