//#  -*- mode: c++ -*-
//#
//#  SDOmode.h: Subband Data Output mode.
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
//#  $Id: SDOmode.h 21314 2012-06-26 14:01:44Z overeem $

#ifndef SDOMODE_H_
#define SDOMODE_H_

#include <APL/RTCCommon/MarshallBlitz.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <complex>
#include <string>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

    class SDOModeInfo
    {
    public:
      /**
       * Constructors for a Bitmode object.
       */
      SDOModeInfo() { }
	  
      /* Destructor for Bitmode. */
      virtual ~SDOModeInfo() {}
      
      /*@{*/
      /**
       * Member accessor functions.
       */
      blitz::Array<EPA_Protocol::RSRSDOMode, 1>& operator()();
      /*@}*/

    public:
      /*@{*/
      /**
       * marshalling methods
       */
	size_t getSize() const;
	size_t pack  (char* buffer) const;
	size_t unpack(const char *buffer);
      /*@}*/

    private:
      /**
       * Bitmode
       *
       * Dimensions of the arrays are:
       *  - itsSDOModeInfo [N_RSPBOARDS]
       */
      blitz::Array<EPA_Protocol::RSRSDOMode, 1> itsSDOModeInfo;
    };

    inline blitz::Array<EPA_Protocol::RSRSDOMode, 1>& SDOModeInfo::operator()() { 
    	return (itsSDOModeInfo);
    }
  };
}; // namespace LOFAR

#endif /* SDOMODE_H_ */
