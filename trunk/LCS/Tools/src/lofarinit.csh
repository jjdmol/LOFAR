#!/bin/echo Usage: source
#-----------------------------------------------------------------------------
# lofarinit.csh: Define the LOFAR environment for C-like shells
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

# Define correct root here.
set a_root = . #filled in by install

# First strip the current LOFARPATH from PATH and LD_LIBRARY_PATH
# Take care that a possible . is preceeded by a backslash.
if (! $?LOFARPATH) then
    set a_path = `echo $LOFARPATH | sed -e 's/\./\\\./g'`
    set a_bin = "$a_path/bin"
    setenv PATH `echo $PATH | sed -e "s%:$a_bin:%:%g" -e "s%^$a_bin:%%"  -e "s%:$a_bin$%%" -e "s%^$a_bin$%%"`
    set a_lib = "$a_path/lib"
    setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | sed -e "s%:$a_lib:%:%g" -e "s%^$a_lib:%%"  -e "s%:$a_lib$%%" -e "s%^$a_lib$%%"`
endif

# Now define the new LOFARPATH
setenv LOFARPATH `dirname $0`
setenv LOFARPATH `cd $LOFARPATH; pwd`      # make path absolute

# Add the path to the standard paths.
if (! $?PATH) then
    setenv PATH $LOFARPATH/bin
else
    setenv PATH $LOFARPATH/bin:$PATH
endif
if (! $?LD_LIBRARY_PATH) then
    setenv LD_LIBRARY_PATH $LOFARPATH/lib
else
    setenv LD_LIBRARY_PATH $LOFARPATH/lib:$LD_LIBRARY_PATH
endif
