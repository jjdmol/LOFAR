//#  ProcCtrlCmdLine.h: Proxy for the command line controlled process.
//#
//#  Copyright (C) 2008
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_PLC_PROCCTRLCMDLINE_H
#define LOFAR_PLC_PROCCTRLCMDLINE_H

// \file
// Proxy for the command line controlled process.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <PLC/ProcCtrlProxy.h>

namespace LOFAR
{
  namespace ACC
  {
    namespace PLC 
    {
      // \addtogroup PLC
      // @{

      // Proxy for the command line controlled process.
      class ProcCtrlCmdLine : public ProcCtrlProxy
      {
      public:
        // Constructor. The argument \a aProcCtrl is a pointer to the "real"
        // Process Control object.
        ProcCtrlCmdLine(ProcessControl* aProcCtrl);

        // Start the process controller. Make a predefined sequence of calls
        // to ProcessController's member functions; the number of times that
        // the run() method will be called is determined by the value of the
        // value of the key \c noRuns in \a arg.
        virtual int operator()(const ParameterSet& arg);
      };

      // @}

    } // namespace PLC

  } // namespace ACC

} // namespace LOFAR

#endif
