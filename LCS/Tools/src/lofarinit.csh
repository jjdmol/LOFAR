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

# Define root here.
# This is a placeholder, so do NOT change the line.
# The actual value is filled in by make install (see Makefile.am).
set a_root = . #filled in by install

# First strip the current LOFARROOT from PATH and LD_LIBRARY_PATH
# Take care that a possible . is preceeded by a backslash.
if ($?LOFARROOT) then
    set a_path = `echo $LOFARROOT | sed -e 's/\./\\\./g'`
    set a_bin = "$a_path/bin"
    setenv PATH `echo $PATH | sed -e "s%:${a_bin}:%:%g" -e "s%^${a_bin}:%%"  -e "s%:${a_bin}"'$%%' -e "s%^${a_bin}"'$%%'`
    set a_lib = "$a_path/lib"
    setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | sed -e "s%:${a_lib}:%:%g" -e "s%^${a_lib}:%%"  -e "s%:${a_lib}"'$%%' -e "s%^${a_lib}"'$%%'`
endif

# Now define the new LOFARROOT
setenv LOFARROOT `cd $a_root; pwd`      # make path absolute

# Also strip this path from the current paths (in case it is contained in it).
set a_path = `echo $LOFARROOT | sed -e 's/\./\\\./g'`
set a_bin = "$a_path/bin"
setenv PATH `echo $PATH | sed -e "s%:${a_bin}:%:%g" -e "s%^${a_bin}:%%"  -e "s%:${a_bin}"'$%%' -e "s%^${a_bin}"'$%%'`
set a_lib = "$a_path/lib"
setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | sed -e "s%:${a_lib}:%:%g" -e "s%^${a_lib}:%%"  -e "s%:${a_lib}"'$%%' -e "s%^${a_lib}"'$%%'`

# Add the path to the standard paths.
if (! $?PATH) then
    setenv PATH $LOFARROOT/bin
else
    setenv PATH $LOFARROOT/bin:$PATH
endif
if (! $?LD_LIBRARY_PATH) then
    setenv LD_LIBRARY_PATH $LOFARROOT/lib
else
    setenv LD_LIBRARY_PATH $LOFARROOT/lib:$LD_LIBRARY_PATH
endif
