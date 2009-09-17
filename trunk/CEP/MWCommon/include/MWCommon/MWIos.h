//# MWIos.h: IO stream to a unique file
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  MWIos.h: 
//#
//#  Copyright (C) 2007
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
//#  $Id$

#ifndef LOFAR_MWCOMMON_MWIOSTREAM_H
#define LOFAR_MWCOMMON_MWIOSTREAM_H

// @file
// @brief IO stream to a unique file
// @author Ger van Diepen (diepen AT astron nl)

#include <iostream>
#include <fstream>
#include <string>

#define MWCOUT MWIos::os()

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief  IO stream to a unique file

  // MPI has the problem that the output of cout is unpredictable.
  // Therefore the output of tMWControl is using a separate output
  // file for each rank.
  // This class makes this possible. The alias MWCOUT can be used for it.
  //
  // Note that everything is static, so no destructor is called.
  // The clear function can be called at the end of the program to
  // delete the internal object, otherwise tools like valgrind will
  // complain about a memory leak.

  class MWIos
  {
  public:
    // Define the name of the output file.
    static void setName (const std::string& name)
      { itsName = name; }

    // Get access to its ostream.
    // It creates the ostream if not done yet.
    static std::ofstream& os()
      { if (!itsIos) makeIos(); return *itsIos; }

    // Remove the ostream (otherwise there'll be a memory leak).
    static void clear()
      { delete itsIos; }

  private:
    // Make the ostream if not done yet.
    static void makeIos();

    static std::string    itsName;
    static std::ofstream* itsIos;
  };

}} //# end namespaces

#endif
