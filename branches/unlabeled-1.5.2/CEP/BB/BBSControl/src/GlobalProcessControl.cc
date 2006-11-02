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
#include <BBSControl/BBSStep.h>
#include <BBSKernel/BBSStatus.h>
#include <Transport/DH_BlobStreamable.h>
#include <Transport/TH_Socket.h>
#include <Transport/CSConnection.h>
#include <Common/LofarLogger.h>
#include <Common/Exceptions.h>
#include <Common/lofar_smartptr.h>

using namespace LOFAR::ACC::APS;

namespace LOFAR
{
  namespace BBS
  {
    //##--------   P u b l i c   m e t h o d s   --------##//

    BBSProcessControl::BBSProcessControl() :
      ProcessControl(),
      itsStrategy(0),
      itsConnection(0),
      itsStrategySent(false)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }
    

    BBSProcessControl::~BBSProcessControl()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      delete itsStrategy;
      delete itsConnection;
    }


    tribool BBSProcessControl::define()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_INFO("BBSProcessControl::define()");
      
      try {
	// Retrieve the strategy from the parameter set.
	itsStrategy = new BBSStrategy(*globalParameterSet());

	// Retrieve the steps in the strategy in sequential order.
	itsSteps = itsStrategy->getAllSteps();
      }
      catch (Exception& e) {
	LOG_ERROR_STR("Caught exception in BBSProcessControl::define()\n" 
                      << e);
	  return false;
      }
      return true;
    }


    tribool BBSProcessControl::init()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_INFO("BBSProcessControl::init()");

      try {
	// We need to send the strategy first.
	itsStrategySent = false;

	// Set the step iterator at the start of the vector of steps.
	itsStepsIterator = itsSteps.begin();

        // Create a new BBS server connection.
        itsConnection = new BBSConnection
          (globalParameterSet()->getString("Controller.Port"));
      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	return false;
      }
      // All went well.
      return true;
    }


    tribool BBSProcessControl::run()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_INFO("BBSProcessControl::run()");

      try {
	// Check pre-conditions
	ASSERT(itsConnection);

	// Keep the result of sending data.
	bool result;

	// If we have not sent the strategy yet. We should do so now.
	if (!itsStrategySent) {
	  result = itsStrategySent = itsConnection->sendObject(*itsStrategy);
	}
	// Else, we should send the next step, unless we're at the end of the
	// vector of steps.
	else {
	  if (itsStepsIterator != itsSteps.end()) {
	    // Send the next step and increment the iterator.
	   result = itsConnection->sendObject(**itsStepsIterator++);
	  }
	  else {
	    LOG_TRACE_COND("Reached end of vector of steps");
	    result = false;
	  }
	}
	// If something went wrong, return immediately.
	if (!result) return false;

	// Read back the response from the BBS kernel (probably a BBSStatus).
	BlobStreamable* bs = itsConnection->recvObject();

	// Something went (terribly) wrong. Return immediately.
	if (!bs) return false;

        // Assume we've received a BBSStatus
        BBSStatus* sts = dynamic_cast<BBSStatus*>(bs);
        if (!sts) {
          LOG_ERROR("Expected to receive a BBSStatus");
          return false;
        }

        LOG_DEBUG_STR("Received BBSStatus: " << *sts);
        if (!(*sts)) {
          LOG_ERROR_STR("Received BBSStatus: " << *sts);
          return false;
        }

	// Do some smart things.
	// ...

      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	return false;
      }

      // All went well.
      return true;
    }
    

    tribool BBSProcessControl::quit()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_INFO("BBSProcessControl::quit()");
      return true;
    }


    tribool BBSProcessControl::pause(const string& /*condition*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return false;
    }


    tribool BBSProcessControl::snapshot(const string& /*destination*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return false;
    }


    tribool BBSProcessControl::recover(const string& /*source*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return false;
    }


    tribool BBSProcessControl::reinit(const string& /*configID*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return false;
    }

    string BBSProcessControl::askInfo(const string& /*keylist*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return string();
    }

  } // namespace BBS

} // namespace LOFAR
