//#  filename.cc: Proxy for the ProcessControl class hierarchy.
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
#include <PLC/ProcCtrlProxy.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace ACC
  {
    namespace PLC
    {
      //## --------   P u b l i c   m e t h o d s   -------- ##//

      ProcCtrlProxy::~ProcCtrlProxy()
      {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      }

      tribool	ProcCtrlProxy::define 	 ()
      {
        return itsProcCtrl->define();
      }

      tribool	ProcCtrlProxy::init 	 ()
      {
        return itsProcCtrl->init();
      }

      tribool	ProcCtrlProxy::run 	 ()
      {
        tribool result;
        setRunState();
        result = itsProcCtrl->run();
        if (!result) clearRunState();
        return result;
      }

      tribool	ProcCtrlProxy::pause  	 (const	string&	condition)
      {
        if (condition == PAUSE_OPTION_NOW) clearRunState();
        return itsProcCtrl->pause(condition);
      }

      tribool	ProcCtrlProxy::release	 ()
      {
        return itsProcCtrl->release();
      }

      tribool	ProcCtrlProxy::quit  	 ()
      {
        return itsProcCtrl->quit();
      }

      tribool	ProcCtrlProxy::snapshot (const string&	destination)
      {
        return itsProcCtrl->snapshot(destination);
      }

      tribool	ProcCtrlProxy::recover  (const string&	source)
      {
        return itsProcCtrl->recover(source);
      }

      tribool	ProcCtrlProxy::reinit	 (const string&	configID)
      {
        return itsProcCtrl->reinit(configID);
      }

      string	ProcCtrlProxy::askInfo  (const string& keylist)
      {
        return itsProcCtrl->askInfo(keylist);
      }


      //## -------- P r o t e c t e d   m e t h o d s   -------- ##//

      ProcCtrlProxy::ProcCtrlProxy(ProcessControl* aProcCtrl) :
        itsProcCtrl(aProcCtrl)
      {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      }

    } // namespace PLC

  } // namespace ACC

} // namespace LOFAR
