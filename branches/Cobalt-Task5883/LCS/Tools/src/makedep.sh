#!/bin/sh

# makedep.sh: generate dependency trees (uses and used-by) for LOFAR packages

# Copyright (C) 2002-4
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$

if [ $# -lt 1 ]; then
  echo "Usage: `basename $0` <LOFAR source root directory>"
  exit 1
fi

a_dir=`dirname $0`
a_root=$1

# Check if pkgdep is present in the same directory as this script.
a_pkgdep=$a_dir/pkgdep
if [ ! -x "$a_pkgdep" ]; then
  echo "Fatal error: could not locate program \`pkgdep'"
  exit 1
fi

# Check if finddep is present in the same directory as this script.
a_finddep=$a_dir/finddep
if [ ! -x "$a_finddep" ]; then
  echo "Fatal error: could not locate program \`finddep'"
  exit 1
fi

# Execute finddep.
echo "Executing $a_finddep $a_root  ..."
$a_finddep $a_root

# Execute pkgdep
echo "Executing $a_pkgdep ..."
$a_pkgdep finddep.pkg top strip xhtml hdrtxt="%pkg% Package Directory Tree" href='<a href="../scripts/makepage.php?name=%pkg%" target="description">' > finddep-pkg.html
$a_pkgdep finddep.used xhtml > finddep-used.html
$a_pkgdep finddep.uses xhtml > finddep-uses.html
$a_pkgdep finddep.used-all xhtml hdrtxt="%pkg% Cross Reference Tree<br>(shows in a recursive way the packages where %pkg% is used)" split=".used.html" 
$a_pkgdep finddep.uses-all xhtml hdrtxt="%pkg% Uses Dependency Tree<br>(shows in a recursive way the packages used by %pkg%)" split=".uses.html" 
$a_pkgdep finddep.uses-all xhtml flat hdrtxt="%pkg% Flat Dependency Tree<br>(shows the packages used by %pkg%)" split=".flat.html" 
