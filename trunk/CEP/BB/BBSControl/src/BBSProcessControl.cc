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
#include <Transport/DH_BlobStreamable.h>
#include <Transport/TH_Socket.h>
#include <Transport/CSConnection.h>
#include <Common/LofarLogger.h>

using namespace LOFAR::ACC::APS;

namespace LOFAR
{
  namespace BBS
  {
    //##--------   P u b l i c   m e t h o d s   --------##//

    BBSProcessControl::BBSProcessControl() :
      ProcessControl(),
      itsStrategy(0),
      itsDataHolder(0), 
      itsTransportHolder(0), 
      itsConnection(0),
      itsStrategySent(false)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }
    

    BBSProcessControl::~BBSProcessControl()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      delete itsStrategy;
      delete itsDataHolder;
      delete itsTransportHolder;
      delete itsConnection;
    }


    tribool BBSProcessControl::define()
    {
      LOG_INFO("BBSProcessControl::define()");
      try {
	// Retrieve the strategy from the parameter set.
	itsStrategy = new BBSStrategy(*globalParameterSet());

	// Retrieve the steps in the strategy in sequential order.
	itsSteps = itsStrategy->getAllSteps();

	// Create a new data holder.
	itsDataHolder = new DH_BlobStreamable();

	// Create a new server TH_Socket. Do not open the socket yet.
	itsTransportHolder = 
	  new TH_Socket(globalParameterSet()->getString("BBSControl.port"),
			true,         // sync
			Socket::TCP,  // protocol
			false);       // open socket now
      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	  return false;
      }
      return true;
    }


    tribool BBSProcessControl::init()
    {
      LOG_INFO("BBSProcessControl::init()");
      try {
	// We need to send the strategy first.
	itsStrategySent = false;

	// Set the step iterator at the start of the vector of steps.
	itsStepsIterator = itsSteps.begin();

	// DH_BlobStreamable is initialized implicitly by its constructor.
	ASSERT(itsDataHolder);
	if (!itsDataHolder->isInitialized()) {
	  LOG_ERROR("Initialization of DataHolder failed");
	  return false;
	}
	// Connect the client socket to the server.
	ASSERT(itsTransportHolder);
	if (!itsTransportHolder->init()) {
	  LOG_ERROR("Initialization of TransportHolder failed");
	  return false;
	}
	// Create a new CSConnection object.
	itsConnection = new CSConnection("RWConn", 
					 itsDataHolder, 
					 itsDataHolder, 
					 itsTransportHolder);
	if (!itsConnection || !itsConnection->isConnected()) {
	  LOG_ERROR("Creation of Connection failed");
	  return false;
	}
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
      LOG_INFO("BBSProcessControl::run()");

      try {
	// Check pre-conditions
	ASSERT(itsDataHolder);
	ASSERT(itsConnection);
	ASSERT(itsStrategy);

	// If we have not sent the strategy yet. We should do so now.
	if (!itsStrategySent) {
	  return sendObject(*itsStrategy);
	}
	// Else, we should send the next step, unless we're at the end of the
	// vector of steps.
	else {
	  if (itsStepsIterator == itsSteps.end()) {
	    LOG_TRACE_COND("Reached end of vector of steps");
	    return false;
	  }
	  // Send the next step and increment the iterator.
	  return sendObject(**itsStepsIterator++);
	}
      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	return false;
      }
    }
    

    bool BBSProcessControl::sendObject(const BlobStreamable& bs)
    {
      try {
	// Serialize the object
	itsDataHolder->serialize(bs);
	LOG_DEBUG_STR("Sending a " << itsDataHolder->classType() << " object");

	// Do a blocking send
	if (itsConnection->write() == CSConnection::Error) {
	  LOG_ERROR("Connection::write() failed");
	  return false;
	}
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
      LOG_INFO("BBSProcessControl::quit()");
      return true;
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//
    
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
      return string();
    }


  } // namespace BBS

} // namespace LOFAR
