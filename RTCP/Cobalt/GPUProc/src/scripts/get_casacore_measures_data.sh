#!/bin/sh

# get_casacore_measures_tables.sh
# Retrieve new casacore measures tables and install it atomically.

# $Id$

measures_ftp_path=ftp://ftp.atnf.csiro.au/pub/software/measures_data
measures_data_filename=measures_data.tar.bz2
measures_md5sum_filename=$measures_data_filename.md5sum
measures_md5sum_filename2=$measures_md5sum_filename.1  # wget does this if exists
update_id=`date +%FT%T.%N`  # e.g. 2013-09-26T01:58:30.098006623


cd $HOME/root/share
mkdir -p aips++
cd aips++

# Generate date timestamp for a directory to store the table update.
# Mangle in nanosecs to survive concurrent update (don't do it!)
mkdir $update_id
cd $update_id

# Get the data from CSIRO's (slow from NL) FTP server. About 1 MB may take a minute.
# Get the md5sum twice, so we safe ourselves another download of the archive when they update the files in between.
# By default, when wget downloads a file, the timestamp is set to match the timestamp from the remote file.
wget --tries=4 \
  $measures_ftp_path/$measures_md5sum_filename \
  $measures_ftp_path/$measures_data_filename \
  $measures_ftp_path/$measures_md5sum_filename

# Verify that md5 hash is equal to hash in $measures_md5sum_filename
# No need to compare the filename. (And note that the .md5sum from CSIRO contains a CSIRO path.)
md5sum_pre=`cut -f 1 -d ' ' $measures_md5sum_filename`
data_md5=`md5sum $measures_data_filename | cut -f 1 -d ' '`
md5sum_post=`cut -f 1 -d ' ' $measures_md5sum_filename2`
if [md5sum_pre -ne data_md5 && md5sum_post -ne data_md5]; then
  echo "MD5 checksums failed to match. Deleting downloaded files. Retrying retrieval once."
  
  retrieve()
  #check/loop
fi
echo "MD5 checksum matches the archive."

# Unpack. This will create data/ephemeris/ and data/geodetic/
tar jxf measures_data.tar.bz2

# Everything cool. Switch data/ symlink to the new data/ _atomically_ with a rename.
# No race with a reader possible. (As long as we don't delete the old data files too early.)
cd ..   # out of $update_id/
ln -s $update_id/data data_tmp && mv -Tf data_tmp data
sync

