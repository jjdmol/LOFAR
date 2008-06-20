//#  ProcCtrlCmdLine.cc: Proxy for the command line controlled process.
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <PLC/ProcCtrlCmdLine.h>
#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace ACC
  {
    namespace PLC
    {
      using APS::ParameterSet;

      //## --------   P u b l i c   m e t h o d s   -------- ##//

      ProcCtrlCmdLine::ProcCtrlCmdLine(ProcessControl* aProcCtrl) :
        ProcCtrlProxy(aProcCtrl)
      {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      }

      int ProcCtrlCmdLine::exec(const ParameterSet& arg)
      {
        LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
        bool   ok       = true;
        string progName = arg.getString("ProgramName");
        uint   noRuns   = arg.getUint32("NoRuns", 0);

        LOG_DEBUG(progName + " starting define");
        if (!(ok = ok && define())) LOG_ERROR("Error during define()");
        else {
          LOG_DEBUG(progName + " initializing");
          if (!(ok = ok && init())) LOG_ERROR("Error during init()");
          else {
            LOG_DEBUG_STR(progName + " running (noRuns=" << noRuns << ")");
            if (noRuns > 0) 
              do ok = ok && run(); while (inRunState() && noRuns-- > 0);
            else
              do ok = ok && run(); while (inRunState());
            if (!ok) LOG_ERROR("Error during run()");
            else {
              LOG_DEBUG(progName + " pausing now");
              if (!(ok = ok && pause(PAUSE_OPTION_NOW))) 
                LOG_ERROR("Error during pause()");
            }
          }
        }
        LOG_DEBUG(progName + " releasing");
        if (!(ok = ok && release())) LOG_ERROR("Error during release()");

        LOG_DEBUG(progName + " quitting");
        if (!(ok = ok && quit())) LOG_ERROR("Error during quit()");

        return (!ok);
      }

    } // namespace PLC

  } // namespace ACC

} // namespace LOFAR
