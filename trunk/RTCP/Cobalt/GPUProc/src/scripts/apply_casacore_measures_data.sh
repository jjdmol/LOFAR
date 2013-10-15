#!/bin/sh
# apply_casacore_measures_tables.sh
# Install downloaded casacore measures tables atomically and verify which tables are in use.
# Written for jenkins@fs5 (DAS-4).
#
# $Id$

# Keep these vars in sync with get_casacore_measures_tables.sh
working_dir=$HOME/root/share/aips++  
dir_prefix=measures_data-


# find the latest
if ! cd "$working_dir"; then exit 1; fi
update_id=`ls -d $dir_prefix* 2> /dev/null | tail -n 1`
if [ -z "$update_id" ]; then
  echo "No casacore measures directory recognized. Running findmeastable to see if it has it elsewhere."
  findmeastable
  exit
fi

# If not already in use, switch data/ symlink to the latest data/ _atomically_ with a rename. Avoids race with a reader.
if [ "`readlink data`" != "$update_id/data" ]; then
  echo "Applying $update_id"
  ln -s "$update_id/data" "data_${update_id}_tmp" && mv -Tf "data_${update_id}_tmp" data
else
  echo "No new table to apply."
fi

# See if casacore uses the latest tables by extracting the last token from findmeastable.
# If ok, it prints: "Measures table Observatories found as /home/jenkins/root/share/aips++/data/geodetic/Observatories"
if ! findmeastable > /dev/null; then exit 1; fi
used_dir=`findmeastable | awk -F\  '{print $NF}'`
if [ $? -ne 0 ]; then exit 1; fi
used_path=`readlink -f "$used_dir/../../../data"`
if [ $? -ne 0 ]; then exit 1; fi
update_id_path="$working_dir/$update_id/data"
if [ "$update_id_path" != "$used_path" ]; then
  echo "It appears that the most recently retrieved measures data is not in use. Most recent is: '$update_id/data'."
  # potential improvement: revert if applied (and if it used to work) (e.g. empty $update_id/)
  exit 1
fi
echo "All cool. The most recently retrieved measures data is (now) in use."

