//#  BBSStep.cc: 
//#
//#  Copyright (C) 2002-2004
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

#include <lofar_config.h>

#include <BBSControl/BBSProcessControl.h>
#include <BBSControl/BBSStrategy.h>
#include <Common/LofarLogger.h>
#include <boost/logic/tribool.hpp>

using namespace boost::logic;
using namespace LOFAR::ACC::APS;

namespace LOFAR
{
  namespace BBS
  {

    //##--------   P u b l i c   m e t h o d s   --------##//

    BBSProcessControl::~BBSProcessControl()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    tribool BBSProcessControl::define()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      try {
	itsParamSet = *globalParameterSet();
	itsStrategy = new BBSStrategy(itsParamSet);
	itsSteps = itsStrategy->getAllSteps();
      } catch(Exception& e) {
	LOG_WARN_STR(e);
	return false;
      }
      return true;
    }


    tribool BBSProcessControl::init()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      return indeterminate;
    }


    tribool BBSProcessControl::run()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      return indeterminate;
    }
    
    
    tribool BBSProcessControl::quit()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      return indeterminate;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//
    
    tribool BBSProcessControl::pause(const string& condition)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      return indeterminate;
    }


    tribool BBSProcessControl::snapshot(const string& destination)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      return indeterminate;
    }


    tribool BBSProcessControl::recover(const string& source)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      return indeterminate;
    }


    tribool BBSProcessControl::reinit(const string& configID)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      return indeterminate;
    }

    string BBSProcessControl::askInfo(const string& keylist)
    {
      return string();
    }

    //##--------   G l o b a l   m e t h o d s   --------##//


  } // namespace BBS

} // namespace LOFAR
