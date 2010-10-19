#!/bin/echo Usage: .
#-----------------------------------------------------------------------------
# lofarinit.sh: Define the LOFAR environment for Bourne-like shells
#-----------------------------------------------------------------------------
#
# Copyright (C) 2005
# ASTRON (Netherlands Foundation for Research in Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# $Id$

# Define root here.
# This is a placeholder, so do NOT change the line.
# The actual value is filled in by make install (see Makefile.am).
lfr_root=. #filled in by install

# First strip the current LOFARROOT from PATH and LD_LIBRARY_PATH
# Take care that a possible . is preceeded by a backslash (for the later sed).
if [ "$LOFARROOT" != "" ]; then
    lfr_path=`echo $LOFARROOT | sed -e 's/\./\\\./g'`
    lfr_bin="$lfr_path/bin"
    PATH=`echo $PATH | sed -e "s%:$lfr_bin:%:%g" -e "s%^$lfr_bin:%%"  -e "s%:$lfr_bin$%%" -e "s%^$lfr_bin$%%"`
    export PATH
    lfr_lib="$lfr_path/lib"
    LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | sed -e "s%:$lfr_lib:%:%g" -e "s%^$lfr_lib:%%"  -e "s%:$lfr_lib$%%" -e "s%^$lfr_lib$%%"`
    export LD_LIBRARY_PATH
fi

# Now define the new LOFARROOT (if possible)
# Do it only if the bin directory exists.
lfr_nroot=`cd $lfr_root > /dev/null; pwd`      # make path absolute
if [ "$lfr_nroot" = ""  -o  ! -d $lfr_nroot/bin ]; then
    echo "LOFAR root directory $lfr_nroot/bin does not exist; keeping old LOFARROOT $LOFARROOT"
else
    LOFARROOT=$lfr_nroot
    export LOFARROOT

    # Also strip root from the current paths (in case it is contained).
    lfr_path=`echo $LOFARROOT | sed -e 's/\./\\\./g'`
    lfr_bin="$lfr_path/bin"
    PATH=`echo $PATH | sed -e "s%:$lfr_bin:%:%g" -e "s%^$lfr_bin:%%"  -e "s%:$lfr_bin$%%" -e "s%^$lfr_bin$%%"`
    export PATH
    lfr_lib="$lfr_path/lib"
    LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | sed -e "s%:$lfr_lib:%:%g" -e "s%^$lfr_lib:%%"  -e "s%:$lfr_lib$%%" -e "s%^$lfr_lib$%%"`
    export LD_LIBRARY_PATH
fi

# Add to the paths if the bin directory exsists.
if [ "$LOFARROOT" = ""  -o  ! -d $LOFARROOT/bin ]; then
    echo "No LOFARROOT defined"
else
    # Add the path to the standard paths.
    if [ "$PATH" = "" ]; then
        PATH=$LOFARROOT/bin
    else
        PATH=$LOFARROOT/bin:$PATH
    fi
    export PATH
    if [ "$LD_LIBRARY_PATH" = "" ]; then
        LD_LIBRARY_PATH=$LOFARROOT/lib
    else
        LD_LIBRARY_PATH=$LOFARROOT/lib:$LD_LIBRARY_PATH
    fi
    export LD_LIBRARY_PATH
fi

# Now define the new LOFARDATAROOT (if possible).
# First try as data directory of the LOFAR install directory.
data_path=`echo $LOFARROOT | sed -e 's%/installed.*%%'`
if [ "$data_path" != ""  -a  -d $data_path/data ]; then
    LOFARDATAROOT=$data_path/data
    export LOFARDATAROOT
else
    # Try it as the LOFARDATA directory (part of the source tree).
    data_path=`echo $LOFARROOT | sed -e 's%/LOFAR/.*%/LOFAR%'`
    if [ "$data_path" != ""  -a  -d ${data_path}DATA ]; then
        LOFARDATAROOT=${data_path}DATA
        export LOFARDATAROOT
    fi
fi

# Clean up
unset lfr_root lfr_nroot lfr_bin lfr_lib lfr_path
