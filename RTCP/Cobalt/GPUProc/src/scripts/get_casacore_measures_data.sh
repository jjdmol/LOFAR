#!/bin/sh
# get_casacore_measures_tables.sh
# Retrieve new casacore measures tables under $working_dir and extract. Written for jenkins@fs5 (DAS-4).
# If it works out, remove very old download dirs starting with $dir_prefix.
#
# $Id$

# Keep these in sync with apply_casacore_measures_tables.sh
working_dir=$HOME/root/share/aips++
dir_prefix=measures_data-


update_id=$dir_prefix`date +%FT%T.%N`  # e.g. measures_data-2013-09-26T01:58:30.098006623
measures_ftp_path=ftp://ftp.atnf.csiro.au/pub/software/measures_data
measures_data_filename=measures_data.tar.bz2
measures_md5sum_filename=$measures_data_filename.md5sum


# Get the data from CSIRO's (slow from NL) FTP server. About 1 MB may take a minute.
# By default, when wget downloads a file, the timestamp is set to match the timestamp from the remote file.
download()
{
  wget -N --tries=4 \
    $measures_ftp_path/$measures_md5sum_filename \
    $measures_ftp_path/$measures_data_filename
}

# Verify that md5 hash is equal to hash in $measures_md5sum_filename
# No need to compare the filename. (And note that the .md5sum from CSIRO contains a CSIRO path.)
check_md5()
{
  local md5sum
  local data_md5

  if ! md5sum=`cut -f 1 -d ' ' $measures_md5sum_filename` && \
       data_md5=`md5sum $measures_data_filename | cut -f 1 -d ' '`
    echo "Failed to extract or compute md5 sum."
    return 1
  fi

  if [ $md5sum != $data_md5 ]; then
    echo "Computed and downloaded md5 sums do not match."
    return 1
  fi

  return 0
}

cleanup_abort()
{
  cd $working_dir && rm -rf tmp_$update_id
  exit 1
}


# Generate date timestamp for a directory to store the table update.
# Exit on failed mkdir and mangle in nanosecs to survive concurrent runs.
if ! cd $working_dir && mkdir tmp_$update_id && cd tmp_$update_id; then
  echo "Failed to enter $working_dir, and create tmp_$update_id."
  exit 1
fi

if ! download && check_md5; then
  echo "Download or MD5 checksums check failed. Retrying once."
  rm -f tmp_$update_id/*
  sleep 2
  if ! download && check_md5; then
    echo "Download or MD5 checksum check failed again."
    cleanup_abort
  fi
fi

if ! tar jxf $measures_data_filename; then
  echo "Failed to extract downloaded archive."
  cleanup_abort
fi

# Remove earlier downloaded entries beyond the 3 latest. ('ls' also sorts.)
if [ -d $dir_prefix* ]; then
  cd $working_dir && rm -rf `ls -d $dir_prefix* | head -n -3`
  if [ $? -ne 0 && ]; then
    echo "Failed to remove old download dir(s)."
  fi
fi

# Make available to the apply script to install/move into the right place.
mv tmp_$update_id $update_id

