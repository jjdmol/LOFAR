//#  SubbandSelection.h: subband selection class.
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

#ifndef SUBBANDSELECTION_H_
#define SUBBANDSELECTION_H_

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace RSP_Protocol
{
  class SubbandSelection
      {
      public:
	  /**
	   * Constructors for a SubbandSelection object.
	   * Currently the tv_usec part is always set to 0 irrespective
	   * of the value passed in.
	   */
	  SubbandSelection() { }
	  
	  /* Destructor for SubbandSelection. */
	  virtual ~SubbandSelection() {}

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
	  /**
	   * Subband selection array.
	   * dim 1 = n_settings (== 1 on SETSUBBANDS, == count(rcumask) on GETSUBBANDS_ACK)
	   * dim 2 = n_beamlets
	   */
	  blitz::Array<int16, 2> m_subbands;
      };
};
     
#endif /* SUBBANDSELECTION_H_ */
