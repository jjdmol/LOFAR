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

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

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

      public:
	  /**
	   * marshalling methods
	   */
	  /*@{*/
	  unsigned int getSize();
	  unsigned int pack  (void* buffer);
	  unsigned int unpack(void *buffer);
	  /*@}*/

      private:
	  /**
	   * System status fields
	   * Dimension of each array is eaqual to the
	   * number of configured hardware resources.
	   * The dimension of the m_rcu_status array
	   * is dependent on the number of bits set
	   * in the rcumask parameter.
	   */
	  blitz::Array<uint16, 1> m_ap_status;
	  blitz::Array<uint16, 1> m_bp_status;
	  blitz::Array<uint32, 1> m_eth_status;
	  blitz::Array<uint16, 1> m_rcu_status;
      };
};
     
#endif /* SYSTEMSTATUS_H_ */
