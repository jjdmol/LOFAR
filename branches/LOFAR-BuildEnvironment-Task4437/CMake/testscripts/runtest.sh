#!/bin/sh

# runtest.sh: script to assay a test program
#
#  Copyright (C) 2002
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#  $Id$

# Remove all aliases, we want a clean shell.
unalias -a

# Set default options.
NEEDOUTFIL=0

# Sanity check! Can be removed once things work OK.
if [ -z "$srcdir" ]; then
  echo "FATAL ERROR: srcdir is not set"
  exit 1
fi

# Handle possible options. 
while [ $# != 0 ]
do
  if [ "$1" = "-stdout" ]; then
    NEEDOUTFIL=1
    shift
  elif [ "$1" = "-nostdout" ]; then
    NEEDOUTFIL=0
    shift
  else
    break
  fi
done

if [ $# -lt 1 ] || [ $# -gt 3 ]; then
  echo "usage: runtest.sh [-stdout] <testname> [<max run-time>] [<precision>]"
  exit 1
fi

# List of files to copy (and later clean up)
FILELIST="$1.in* $1.stdout $1.run $1.py $1.parset* $1.log_prop $1.debug"

# Maximum run-time in seconds, defaults to 300
MAXTIME=${2:-300}

# Numeric precision, defaults to 1e-5
PREC=${3:-1e-5}

# Get directory of this script.
lfr_script_dir=`dirname "$0"`
lfr_script_dir=`cd "$lfr_script_dir" && pwd`

# Export lfr_script_dir, so that it can be used by assay
export lfr_script_dir

# Add the current directory to the path. We don't care if it's already in.
PATH=.:$PATH
export PATH

# Copy required files to current directory
cp "$lfr_script_dir/default.log_prop" "$1.log_prop"
cp "$lfr_script_dir/default.debug" "$1.debug"
for f in $FILELIST
do
  eval cp -a "$srcdir/$f" . 2>/dev/null
done

# Run assay
"$lfr_script_dir/assay" $1 $MAXTIME $PREC $NEEDOUTFIL
STATUS=$?

# Clean up
trap 'for f in $FILELIST; do rm -rf "$f"; done; \
      trap - 0;
      exit $STATUS' 0 1 2 3 15
