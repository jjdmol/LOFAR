//#  BeamletWeights.h: beamlet weights class
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

#ifndef BEAMLETWEIGHTS_H_
#define BEAMLETWEIGHTS_H_

#include <complex>
#include <blitz/array.h>

namespace RSP_Protocol
{
  class BeamletWeights
      {
      public:
	  /**
	   * Constructors for a BeamletWeights object.
	   * Currently the tv_usec part is always set to 0 irrespective
	   * of the value passed in.
	   */
	  BeamletWeights() { }
	  
	  /* Destructor for BeamletWeights. */
	  virtual ~BeamletWeights() {}

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
	   * The beamlet weights.
	   */
	  blitz::Array<std::complex<double>, 3> m_weights;
      };
};
     
#endif /* BEAMLETWEIGHTS_H_ */
