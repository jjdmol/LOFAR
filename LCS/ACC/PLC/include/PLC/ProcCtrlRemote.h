//# ProcCtrlRemote.h: Proxy for the ACC controlled process.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_PLC_PROCCTRLREMOTE_H
#define LOFAR_PLC_PROCCTRLREMOTE_H

// \file
// Proxy for the command line process control.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <PLC/ProcCtrlProxy.h>
#include <PLC/ProcControlServer.h>

namespace LOFAR
{
  namespace ACC 
  {
    namespace PLC 
    {
      // \addtogroup PLC
      // @{

      // Proxy for the command line process control.
      class ProcCtrlRemote : public ProcCtrlProxy
      {
      public:
        // Constructor. The argument \a aProcCtrl is a pointer to the "real"
        // Process Control object.
        ProcCtrlRemote(ProcessControl* aProcCtrl);

        // Start the process controller. Let it run under control of a
        // ProcControlServer.
        virtual int operator()(const ParameterSet& arg);
      };

      // @}

    } // namespace PLC

  } // namespace ACC

} // namespace LOFAR

#endif
