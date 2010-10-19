//#  CommandQueueTriggers.h: Handle trigger for the BBS command queue
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

#ifndef LOFAR_BBSCONTROL_COMMANDQUEUETRIGGER_H
#define LOFAR_BBSCONTROL_COMMANDQUEUETRIGGER_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

// \file
// Handle trigger for the BBS command queue

//# Includes
#if defined(HAVE_PQXX)
# include <pqxx/trigger>
#else
# error libpqxx, the C++ API to PostgreSQL, is required
#endif

#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward Declarations
    class CommandQueue;

    // \addtogroup BBSControl
    // @{

    // Functor class for handling in incoming trigger.
    class CommandQueueTrigger : public pqxx::trigger
    {
    public:
      // Constructor. The trigger visitor will handle notifications received
      // from the CommandQueue \a queue.
      explicit CommandQueueTrigger(const CommandQueue& queue,
				   const string& name = "");

      // Destructor. Reimplemented only because of no-throw specification.
      virtual ~CommandQueueTrigger() throw() {}

      // Handle the notification.
      virtual void operator()(int be_pid);

    private:
      // Here we store a \e reference to the CommandQueue that will trigger
      // notifications.
      const CommandQueue& itsQueue;

      // Name associated with this trigger visitor.
      const string itsName;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
