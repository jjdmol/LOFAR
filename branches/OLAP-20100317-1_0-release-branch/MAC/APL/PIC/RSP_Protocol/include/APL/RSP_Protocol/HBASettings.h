//#  -*- mode: c++ -*-
//#
//#  HBASettings.h: HBA control information
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

#ifndef HBASETTINGS_H_
#define HBASETTINGS_H_

#include <APL/RSP_Protocol/EPA_Protocol.ph>
//#include <APL/RTCCommon/RegisterState.h>

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

    class HBASettings
    {
    public:

      HBASettings() {}
      virtual ~HBASettings() {}

      /* get HBA delay settings array */
      blitz::Array<uint8, 2>& operator()();

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
      // two dimensional (nRcus x MEPHeader::N_HBA_DELAYS)
      blitz::Array<uint8, 2> m_delay;
    };
  
    inline blitz::Array<uint8, 2>& HBASettings::operator()() { return m_delay; }
  };
}; // namespace LOFAR

#endif /* HBASETTINGS_H_ */
