//# ReadLine.h: read a line from stdin using readline or cin
//#
//# Copyright (C) 2009
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_COMMON_READLINE_H
#define LOFAR_COMMON_READLINE_H

// \file

// The functions in here read a line from stdin using readline.
// If readline is not found during configure, it reverts to reading from cin.
//
// By default file name completion can be used and command recall and editing.
// If context-sensitive completion is needed, the user should initialize
// readline explicitly.

//# Includes
#include <Common/lofar_string.h>

namespace LOFAR
{
  // Print a prompt, read a line, and add it to the history.
  // False is returned if EOF is given.
  // The resulting line is put in the argument \a line.
  bool readLine (string& line, const string& prompt=string(),
                 bool addToHistory=true);

  // Same as readLine, but remove leading whitespace, skip empty lines,
  // and optionally skip comment lines.
  // The default comment character string is empty meaning that no test
  // for comment lines is done.
  bool readLineSkip (string& line, const string& prompt=string(),
                     const string& commentChars=string());

} // namespace LOFAR

#endif
