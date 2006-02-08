//#  -*- mode: c++ -*-
//#
//#  Clocks.h: FPGA firmware version information from the RSP board.
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

#ifndef CLOCKS_H_
#define CLOCKS_H_

#include <APL/RTCCommon/Marshalling.h>

#include <complex>
#include <string>
#include <blitz/array.h>
#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace RSP_Protocol {

    class Clocks
    {
    public:
      /**
       * Constructors for a Clocks object.
       */
      Clocks() : m_modified(false), m_readcounter(0) { }
	  
      /* Destructor for Clocks. */
      virtual ~Clocks() {}

      /* get references to the version arrays */
      blitz::Array<uint32, 1>& operator()();

      /**
       * Access methods.
       * clearModifiedConditional only clears the modified flag if
       * it has been read at least once while it was true. This
       * ensures that the flag is not cleared before action has been
       * taken on the changes (such as updating a hardware register).
       */
      void setModified()              { m_modified = true;  m_readcounter = 0; }
      void clearModified()            { m_modified = false; m_readcounter = 0; }
      void clearModifiedConditional() { if (m_readcounter > 0) clearModified(); }
      bool getModified()              { if (m_modified) m_readcounter++; return m_modified; }

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
       * Clocks
       *
       * Dimensions of the arrays are:
       *  - m_clocks  [N_TDBOARDS]
       */
      blitz::Array<uint32, 1> m_clocks;

      /**
       * Keep track of when clocks setting is modified and
       * how many times the modified flag has been read.
       */
      bool m_modified;
      int  m_readcounter;
    };

    inline blitz::Array<uint32, 1>& Clocks::operator()() { return m_clocks; }
  };
}; // namespace LOFAR

#endif /* CLOCKS_H_ */
