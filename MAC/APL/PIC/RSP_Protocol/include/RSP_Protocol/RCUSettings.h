//#  RCUSettings.h: RCU control information
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

#ifndef RCUSETTINGS_H_
#define RCUSETTINGS_H_

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace RSP_Protocol
{
  class RCUSettings
      {
      public:
	  /**
	   * Constructors for a RCUSettings object.
	   * Currently the tv_usec part is always set to 0 irrespective
	   * of the value passed in.
	   */
	  RCUSettings() { }
	  
	  /* Destructor for RCUSettings. */
	  virtual ~RCUSettings() {}

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
	   * Setting bitfield for an RCU.
	   */
	  union RCURegisterType
	  {
	      typedef struct
	      {
		  uint8 filter_0:1;
		  uint8 filter_1:1;
		  uint8 filter_2:1;
		  uint8 filter_3:1;
		  uint8 lba_pwr:1;
		  uint8 hba_pwr:1;
		  uint8 rcu_pwr:1;
		  uint8 ovrflw:1;
	      } RCUBits;
	  
	      RCUBits Bits; 
	      uint8   Register;

	  };

	  blitz::Array<RCURegisterType, 1> settings;
      };
};
     
#endif /* RCUSETTING_H_ */
