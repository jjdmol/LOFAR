//#  GlobalProcessControl.cc: Implementation of ACC/PLC ProcessControl class.
//#
//#  Copyright (C) 2002-2007
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

#include <BBSControl/GlobalProcessControl.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/BBSStep.h>
#if 0
# include <BBSKernel/BBSStatus.h>
# include <Transport/DH_BlobStreamable.h>
# include <Transport/TH_Socket.h>
# include <Transport/CSConnection.h>
#endif
#include <BBSControl/CommandQueue.h>
#include <Common/LofarLogger.h>
#include <Common/Exceptions.h>

using namespace LOFAR::ACC::APS;

namespace LOFAR
{
  namespace BBS
  {
    //##--------   P u b l i c   m e t h o d s   --------##//

    GlobalProcessControl::GlobalProcessControl() :
      ProcessControl(),
      itsStrategy(0),
#if 0
      itsDataHolder(0), 
      itsTransportHolder(0), 
      itsConnection(0),
#endif
      itsCommandQueue(0),
      itsStrategySent(false)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }
    

    GlobalProcessControl::~GlobalProcessControl()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      delete itsStrategy;
#if 0
      delete itsDataHolder;
      delete itsTransportHolder;
      delete itsConnection;
#endif
      delete itsCommandQueue;
    }


    tribool GlobalProcessControl::define()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_INFO("GlobalProcessControl::define()");
      
      try {
	// Retrieve the strategy from the parameter set.
	itsStrategy = new BBSStrategy(*globalParameterSet());

	// Retrieve the steps in the strategy in sequential order.
	itsSteps = itsStrategy->getAllSteps();
	LOG_DEBUG_STR("# of steps in strategy: " << itsSteps.size());
    
#if 0
	// Create a new data holder.
	itsDataHolder = new DH_BlobStreamable();

	// Create a new server TH_Socket. Do not open the socket yet.
	itsTransportHolder = 
	  new TH_Socket(globalParameterSet()->getString("Controller.Port"),
			true,         // sync
			Socket::TCP,  // protocol
			false);       // open socket now
#endif
      }
      catch (Exception& e) {
	LOG_ERROR_STR("Caught exception in GlobalProcessControl::define()\n"
		      << e);
	  return false;
      }
      return true;
    }


    tribool GlobalProcessControl::init()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_INFO("GlobalProcessControl::init()");

      try {
#if 0
	// Check pre-conditions
	ASSERT(itsDataHolder);
	ASSERT(itsTransportHolder);
#endif

	// We need to send the strategy first.
	itsStrategySent = false;

	// Set the step iterator at the start of the vector of steps.
	itsStepsIterator = itsSteps.begin();

#if 0
	// DH_BlobStreamable is initialized implicitly by its constructor.
	if (!itsDataHolder->isInitialized()) {
	  LOG_ERROR("Initialization of DataHolder failed");
	  return false;
	}
	// Connect the client socket to the server.
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
#endif
	// Create a new CommandQueue. This will open a connection to the
	// blackboard database.
	itsCommandQueue = 
	  new CommandQueue(globalParameterSet()->getString("BBDB.DBName"));

	// Check if this is a new run. It usually is, but we might be resuming
	// a paused run, or we might be recovering from a crash.
// 	itsIsNewRun = itsCommandQueue->isNewRun();

      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	return false;
      }
      // All went well.
      return true;
    }


    tribool GlobalProcessControl::run()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_INFO("GlobalProcessControl::run()");

      try {
	// Check pre-conditions
#if 0
	ASSERT(itsDataHolder);
	ASSERT(itsConnection);
#endif
	ASSERT(itsStrategy);
	ASSERT(itsCommandQueue);

	// Keep the result of sending data.
	bool result;

	// If we haven't sent the strategy to the command queue yet, we should
	// do it now.
	if (!itsStrategySent) {
	  itsCommandQueue->setStrategy(*itsStrategy);
	  itsStrategySent = true;
	}
	// Else, we should send the next step, unless we're at the end of the
	// vector of steps.
	else {
	  if (itsStepsIterator != itsSteps.end()) {
	    // Send the next step and increment the iterator.
	    itsCommandQueue->addCommand(**itsStepsIterator++);
	  }
	  else {
	    LOG_TRACE_COND("Reached end of vector of steps");
	    result = false;
	  }
	}

#if 0
	// Read back the response from the BBS kernel (probably a BBSStatus).
 	BlobStreamable* bs = recvObject();

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
#endif

	// Wait for a trigger from the database to fetch the result
	


      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	return false;
      }

      // All went well.
      return true;
    }
    

    tribool GlobalProcessControl::quit()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_INFO("GlobalProcessControl::quit()");
      return true;
    }


    tribool GlobalProcessControl::pause(const string& /*condition*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return false;
    }


    tribool GlobalProcessControl::snapshot(const string& /*destination*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return false;
    }


    tribool GlobalProcessControl::recover(const string& /*source*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return false;
    }


    tribool GlobalProcessControl::reinit(const string& /*configID*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return false;
    }

    string GlobalProcessControl::askInfo(const string& /*keylist*/)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      LOG_WARN("Not supported");
      return string();
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//
    
#if 0
    bool GlobalProcessControl::sendObject(const BlobStreamable& bs)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
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


    BlobStreamable* GlobalProcessControl::recvObject()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
      try {
	// Do a blocking receive
	if (itsConnection->read() == CSConnection::Error) {
	  THROW(IOException, "CSConnection::read() failed");
	}

	// Deserialize the object
	BlobStreamable* bs = itsDataHolder->deserialize();
	if (!bs) {
	  LOG_ERROR("Error while receiving object");
	}
	LOG_DEBUG_STR("Received a " << itsDataHolder->classType() <<
		      " object");

	// Return the object
	return bs;
      }
      catch (Exception& e) {
	LOG_ERROR_STR(e);
	return 0;
      }
      // We should never get here.
      ASSERTSTR(false, "We should never get here.");
    }
#endif

  } // namespace BBS

} // namespace LOFAR
