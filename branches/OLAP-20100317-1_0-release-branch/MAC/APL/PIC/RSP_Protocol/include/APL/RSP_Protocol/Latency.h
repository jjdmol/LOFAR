//#  -*- mode: c++ -*-
//#
//#  Latency.h: System status information
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

#ifndef LATENCY_H_
#define LATENCY_H_

#include <iostream>
#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

#include <APL/RSP_Protocol/EPA_Protocol.ph>

namespace LOFAR {
  namespace RSP_Protocol {

    class Latency
    {
    public:
      /**
       * Constructors for a TDStatus object.
       */
      Latency() { }
	  
      /* Destructor for TDStatus. */
      virtual ~Latency() {}

      /*@{*/
      /**
       * Member accessor functions.
       */
      blitz::Array<EPA_Protocol::RADLatency, 1>& operator()();
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
       * RAD lane and ring latency.
       *
       * Dimensions of the arrays are:
       *  - itsLatency  [N_RSPBOARDS]
       */
      blitz::Array<EPA_Protocol::RADLatency, 1> itsLatency;
      /*@}*/
    };

    inline blitz::Array<EPA_Protocol::RADLatency, 1>& Latency::operator()() { 
    	return (itsLatency);
    }

  };
}; // namespace LOFAR

#endif /* LATENCY_H_ */
