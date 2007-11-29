//#  -*- mode: c++ -*-
//#
//#  GetRCUCmd.h: Beamformer Weights settings command.
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

#ifndef GETRCUCMD_H_
#define GETRCUCMD_H_

#include "Command.h"
#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include <Common/LofarTypes.h>
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
  namespace RSP {

    class GetRCUCmd : public Command
    {
    public:
      /**
       * Constructors for a GetRCUCmd object.
       */
      GetRCUCmd(GCFEvent& event, GCFPortInterface& port, Operation oper);
	  
      /* Destructor for GetRCUCmd. */
      virtual ~GetRCUCmd();

      /**
       * Acknowledge the command by sending the appropriate
       * response on m_port.
       */
      virtual void ack(CacheBuffer& cache);

      /**
       * Make necessary changes to the cache for the next synchronization.
       * Any changes will be sent to the RSP boards.
       */
      virtual void apply(CacheBuffer& cache, bool setModFlag = true);

      /**
       * Complete the command by sending the appropriate response on
       * the m_answerport;
       */
      virtual void complete(CacheBuffer& cache);

      /*@{*/
      /**
       * get timestamp of the event
       */
      virtual const RTC::Timestamp& getTimestamp() const;
      virtual void setTimestamp(const RTC::Timestamp& timestamp);
      /*@}*/

      /**
       * Validate the event that underlies the command.
       */
      virtual bool validate() const;

      /**
       * Return true if value should be read from cache.
       */
      bool readFromCache() const;

    private:
      GetRCUCmd();

      RSPGetrcuEvent* m_event;
    };
  };
};
     
#endif /* GETRCUCMD_H_ */
