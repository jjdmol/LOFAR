//#  CommandQueueTrigger.cc: Handle trigger for the BBS command queue
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
#include <BBSControl/CommandQueueTrigger.h>
#include <BBSControl/CommandQueue.h>
#include <BBSControl/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_typeinfo.h>

namespace LOFAR
{
  namespace BBS 
  {

    CommandQueueTrigger::TypeMap CommandQueueTrigger::theirTypes;
    CommandQueueTrigger::Type    CommandQueueTrigger::theirFlags;
    CommandQueueTrigger::Init    CommandQueueTrigger::theirInit;


    CommandQueueTrigger::Init::Init()
    {
      theirTypes[Command] = "insert_command";
      theirTypes[Result]  = "insert_result";
    }


    CommandQueueTrigger::CommandQueueTrigger(const CommandQueue& queue, 
                                             Type type) :
      pqxx::trigger(*queue.itsConnection,
                    theirTypes.find(type) == theirTypes.end() 
                    ? "" : theirTypes[type])
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      ASSERT(theirTypes.find(type) != theirTypes.end());
    }


    void CommandQueueTrigger::operator()(int be_pid)
    {
      LOG_DEBUG_STR("Received notification: " << name() << 
		    "; pid = " << be_pid);
      TypeMap::const_iterator it;
      for (it = theirTypes.begin(); it != theirTypes.end(); ++it) {
        if (it->second == name()) {
          LOG_TRACE_COND_STR("Raising flag [" << 
                             it->first << "," << it->second << "]");
          raiseFlags(it->first);
          break;
        }
      }
    }

  } // namespace BBS
  
} // namespace LOFAR
