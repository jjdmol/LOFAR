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

#include "Timestamp.h"
#include "Cache.h"

#include <Common/LofarTypes.h>
#include <GCF/GCF_Control.h>

namespace RSP
{
  class Command
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
	  void  setPeriod(int16 period);
	  int16 getPeriod();
	  /*@}*/

	  /**
	   * Set the type of operation READ/WRITE.
	   */
	  void setOperation(Operation oper);

	  /*@{*/
	  /**
	   * Accessor methods for the port member.
	   */
	  void setPort(GCFPortInterface& port);
	  GCFPortInterface* getPort();
	  /*@}*/

	  /**
	   * Make necessary changes to the cache for the next synchronization.
	   * Any changes will be sent to the RSP boards.
	   */
	  virtual void apply(CacheBuffer& cache) = 0;

	  /**
	   * Complete the command by sending the appropriate response on
	   * the m_port;
	   */
	  virtual void complete(CacheBuffer& cache) = 0 ;

	  /**
	   * Get or set the timestamp of the underlying event
	   * for a command.
	   */
	  virtual const RSP_Protocol::Timestamp& getTimestamp() = 0;
	  virtual void setTimestamp(const RSP_Protocol::Timestamp& timestamp) = 0;

      private:
	  int16              m_period;
	  GCFEvent*          m_event;
	  GCFPortInterface*  m_port;
	  Operation          m_operation;
      };
};
     
#endif /* COMMAND_H_ */
