//#  -*- mode: c++ -*-
//#
//#  BWCommand.h: Beamformer Weights settings command.
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

#ifndef BWCOMMAND_H_
#define BWCOMMAND_H_

#include "Command.h"
#include "RSP_Protocol.ph"

#include <Common/LofarTypes.h>
#include <GCF/GCF_Control.h>

namespace RSP
{
  class BWCommand : public Command
      {
      public:
	  /**
	   * Constructors for a BWCommand object.
	   * Currently the tv_usec part is always set to 0 irrespective
	   * of the value passed in.
	   */
	  BWCommand(GCFEvent& event, GCFPortInterface& port, Operation oper);
	  
	  /* Destructor for BWCommand. */
	  virtual ~BWCommand();

	  /**
	   * Make necessary changes to the cache for the next synchronization.
	   * Any changes will be sent to the RSP boards.
	   */
	  virtual void apply(CacheBuffer& cache);

	  /**
	   * Complete the command by sending the appropriate response on
	   * the m_answerport;
	   */
	  virtual void complete(CacheBuffer& cache);

	  /*@{*/
	  /**
	   * get timestamp of the event
	   */
	  virtual const Timestamp& getTimestamp();
	  virtual void setTimestamp(const Timestamp& timestamp);
	  /*@}*/

      private:
	  BWCommand();
	  RSPSetweightsEvent* m_event;
      };
};
     
#endif /* BWCOMMAND_H_ */
