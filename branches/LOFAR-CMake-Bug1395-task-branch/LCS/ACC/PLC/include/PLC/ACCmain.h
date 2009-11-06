//# ACCmain.h: main loop that can be used by any ACC enabled program
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_PLC_ACCMAIN_H
#define LOFAR_PLC_ACCMAIN_H

// \file
// main loop that can be used by any ACC enabled program

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <PLC/ProcessControl.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {

// \addtogroup PLC
// @{

// This function is the main loop that can (or should) be used by ACC controlled
// programs. To use it do the following:
// Subclass ProcessControl and implement all functions.
// Implement a real main:
//
//   #include <libgen.h>
//   #include <PLC/ACCmain.h>
//   #include <DerivedClassOfProcessControl.h>
//
//   int main(int argc, char* argv[]) {
//     INIT_LOGGER(basename(argv[0]));
//     DerivedClassOfProcessControl myProcess;
//     return LOFAR::ACC::PLC::ACCmain(argc, argv, &myProcess);
//   }
int ACCmain (int argc, char* argv[], ProcessControl* theProcess);

// @}

    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

#endif
