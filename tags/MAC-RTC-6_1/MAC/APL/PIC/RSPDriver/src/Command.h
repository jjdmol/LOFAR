//#  -*- mode: c++ -*-
//#
//#  Command.h: RSP Driver command class
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

#ifndef COMMAND_H_
#define COMMAND_H_

#include <APL/RTCCommon/Timestamp.h>
#include "Cache.h"
#include "RefCount.h"

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace RSP {

    class Command : public RefCount
    {
    public:

      /** types */
      enum Operation
	{
	  READ = 1,
	  WRITE,
	};

      /**
       * Constructors for a Command object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      Command();
	  
      /* Destructor for Command. */
      virtual ~Command();

      /*@{*/
      /**
       * Accessor methods for the period field.
       * The period (in seconds) with which this command should
       * be executed. The command will be executed every 'period' seconds.
       */
      void  setPeriod(uint16 period);
      int16 getPeriod();
      /*@}*/

      /*@{*/
      /**
       * Set/get the type of operation READ/WRITE.
       */
      void setOperation(Operation oper);
      Operation getOperation() const;
      /*@}*/

      /*@{*/
      /**
       * Accessor methods for the port member.
       */
      void setPort(GCFPortInterface& port);
      GCFPortInterface* getPort();
      /*@}*/

      /**
       * Acknowledge the command by sending the appropriate
       * response on m_port.
       */
      virtual void ack(CacheBuffer& cache) = 0;

      /**
       * Make necessary changes to the cache for the next synchronization.
       * Any changes will be sent to the RSP boards.
       */
      virtual void apply(CacheBuffer& cache, bool setModFlag = true) = 0;

      /**
       * Complete the command by sending the appropriate response on
       * the m_port;
       */
      virtual void complete(CacheBuffer& cache) = 0 ;

      /**
       * Get or set the timestamp of the underlying event
       * for a command.
       */
      virtual const RTC::Timestamp& getTimestamp() const = 0;
      virtual void setTimestamp(const RTC::Timestamp& timestamp) = 0;

      /**
       * Validate the parameters of the event.
       * @return true if they are ok.
       */
      virtual bool validate() const = 0;

      /**
       * Compare operator to order commands in the queue
       */
      bool operator<(const Command& other);

    private:
      uint16             m_period;
      GCFEvent*          m_event;
      GCFPortInterface*  m_port;
      Operation          m_operation;
    };

    /**
     * Comparison function to order a priority_queue of Ptr<Command>* pointers
     * as it is used in the Scheduler class.
     */
    struct Command_greater { 
      bool operator() (Ptr<Command>& x, Ptr<Command>& y) const 
      { return x->getTimestamp() > y->getTimestamp(); }
    };
  };
};
     
#endif /* COMMAND_H_ */
