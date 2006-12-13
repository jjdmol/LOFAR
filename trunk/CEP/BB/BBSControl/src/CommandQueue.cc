//#  CommandQueue.cc: Command queue of the blackboard system.
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <BBSControl/CommandQueue.h>
#include <BBSControl/CommandQueueTransactors.h>
#include <BBSControl/BBSSingleStep.h>
#include <BBSControl/BBSStrategy.h>
#include <BBSControl/Exceptions.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace BBS 
  {

    CommandQueue::CommandQueue(const string& dbname, const string& user,
			       const string& host, const string& port)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      string opts("dbname=" + dbname  + " user=" + user + 
		  " host=" + host + " port=" + port);
      try {
	LOG_DEBUG_STR("Connecting to database using options: " << opts);
	itsConnection.reset(new pqxx::connection(opts));
      }
      catch (pqxx::broken_connection& e) {
	THROW (BBSControlException, "pqxx::broken_connection:\n\t"
	       << opts << "\n\t" << e.what());
      }
    }

    void CommandQueue::addStep(const BBSSingleStep& step)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");

      // Create a transactor object and perform the transaction
      itsConnection->perform(AddStep(step));
    }

    const BBSSingleStep& CommandQueue::getNextStep()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }

    void CommandQueue::setStrategy(const BBSStrategy& strategy)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }

    const BBSStrategy& CommandQueue::getStrategy()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


  } // namespace BBS
  
} // namespace LOFAR
