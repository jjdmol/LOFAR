//#  -*- mode: c++ -*-
//#
//#  Versions.h: FPGA firmware version information from the RSP board.
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

#ifndef VERSIONS_H_
#define VERSIONS_H_

#include <APL/RTCCommon/MarshallBlitz.h>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <complex>
#include <string>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

    class Versions
    {
    public:
      /**
       * Constructors for a Versions object.
       * Currently the tv_usec part is always set to 0 irrespective
       * of the value passed in.
       */
      Versions() { }
	  
      /* Destructor for Versions. */
      virtual ~Versions() {}

      /* get references to the version arrays */
      blitz::Array<EPA_Protocol::RSRVersion, 1>& bp();
      blitz::Array<EPA_Protocol::RSRVersion, 1>& ap();

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
       * Versions
       *
       * Dimensions of the arrays are:
       *  - m_bp_versions   [N_RSPBOARDS]
       *  - m_ap_versions   [N_RSPBOARDS * N_BLPS]
       */
      blitz::Array<EPA_Protocol::RSRVersion, 1> m_bp_versions;
      blitz::Array<EPA_Protocol::RSRVersion, 1> m_ap_versions;
    };

    inline blitz::Array<EPA_Protocol::RSRVersion, 1>& Versions::bp()  { return m_bp_versions; }
    inline blitz::Array<EPA_Protocol::RSRVersion, 1>& Versions::ap()  { return m_ap_versions; }
  };
}; // namespace LOFAR

#endif /* VERSIONS_H_ */
