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
#include <Common/lofar_map.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward Declarations
    class CommandQueue;

    // \addtogroup BBSControl
    // @{

      // Trigger class handles notification of triggers received from the
      // database backend by raising the flag associated with the trigger.
      class CommandQueueTrigger : public pqxx::trigger
      {
      public:
        // Valid trigger types.
        typedef enum {
          Command = 1L << 0,
          Result  = 1L << 1
        } Type;

        // Construct a trigger handler for trigger of type \a type.
        CommandQueueTrigger(const CommandQueue& queue, Type type);

        // Destructor. Reimplemented only because of no-throw specification in
        // the base class.
        virtual ~CommandQueueTrigger() throw() {}

        // Handle the notification, by raising the flag associated with the
        // received trigger.
        virtual void operator()(int be_pid);

        // Test if any of the \a flags are raised.
        Type testFlags(Type flags) const { return Type(theirFlags & flags); }

        // Raise \a flags.
        void raiseFlags(Type flags) { theirFlags = Type(theirFlags | flags); }

        // Clear \a flags.
        void clearFlags(Type flags) { theirFlags = Type(theirFlags & ~flags); }

        // Test if any of the \a flags are raised and clear them.
        Type testAndClearFlags(Type flags) {
          Type tp = testFlags(flags);
          clearFlags(flags);
          return tp;
        }

      private:
        // Map associating trigger types with their string representation.
        // @{
        typedef map<Type, string> TypeMap;
        static TypeMap theirTypes;
        // @}

        // Keep track of flags that were raised as a result of a handled
        // notification. Note that this is a \e shared variable, because we're
        // not really interested in who responds to a raised flag; it's
        // important that it is handled, but not by whom.
        static Type theirFlags;

        // Initializer struct. The default constructor contains code to
        // initialize the static data members theirTypes and theirFlags.
        struct Init {
          Init();
        };

        // Static instance of Init, triggers the initialization of static data
        // members during its construction.
        static Init theirInit;
      };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
