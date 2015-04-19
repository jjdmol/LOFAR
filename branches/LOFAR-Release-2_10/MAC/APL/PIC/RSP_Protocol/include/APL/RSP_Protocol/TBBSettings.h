//#  -*- mode: c++ -*-
//#
//#  TBBSettings.h: TBB control information
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef TBBSETTINGS_H_
#define TBBSETTINGS_H_

#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <complex>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  using namespace EPA_Protocol;
  namespace RSP_Protocol {

    class TBBSettings
    {
    public:

      TBBSettings() {}
      virtual ~TBBSettings() {}

      /* get TBB delay settings array */
      blitz::Array<bitset<MEPHeader::N_SUBBANDS>, 1>& operator()();

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
      // one dimensional dimension1 = nRcus
      blitz::Array<bitset<MEPHeader::N_SUBBANDS>, 1> m_bandsel;
    };
  
    inline blitz::Array<bitset<MEPHeader::N_SUBBANDS>, 1>& TBBSettings::operator()() { return m_bandsel; }
  };
}; // namespace LOFAR

#endif /* TBBSETTINGS_H_ */
