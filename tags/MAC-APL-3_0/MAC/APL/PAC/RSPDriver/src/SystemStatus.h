//#  -*- mode: c++ -*-
//#
//#  SystemStatus.h: System status information
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

#ifndef SYSTEMSTATUS_H_
#define SYSTEMSTATUS_H_

#include "EPA_Protocol.ph"

#include <iostream>
#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
namespace RSP_Protocol
{
  class SystemStatus
  {
    public:
      /**
       * Constructors for a SystemStatus object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      SystemStatus() { }
	  
      /* Destructor for SystemStatus. */
      virtual ~SystemStatus() {}

      /*@{*/
      /**
       * Member accessor functions.
       */
      blitz::Array<EPA_Protocol::BoardStatus, 1>& board();
      blitz::Array<EPA_Protocol::RCUStatus,   1>& rcu();
      /*@}*/

    public:
      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack  (void* buffer);
      unsigned int unpack(void *buffer);
      /*@}*/

    private:
      /*@{*/
      /**
       * System status fields.
       *
       * Dimensions of the arrays are:
       *  - m_board_status  [N_RSPBOARDS]
       *  - m_rcu_status    [N_BLPS * 2]
       */
      blitz::Array<EPA_Protocol::BoardStatus, 1> m_board_status;
      blitz::Array<EPA_Protocol::RCUStatus,   1> m_rcu_status;
      /*@}*/
  };

  inline blitz::Array<EPA_Protocol::BoardStatus, 1>& SystemStatus::board() 
  {
    return m_board_status;
  }

  inline blitz::Array<EPA_Protocol::RCUStatus,  1>& SystemStatus::rcu()
  {
    return m_rcu_status;
  }
};
}; // namespace LOFAR
#endif /* SYSTEMSTATUS_H_ */
