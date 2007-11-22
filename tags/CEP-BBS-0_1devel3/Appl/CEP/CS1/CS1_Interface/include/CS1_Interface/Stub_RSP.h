//# Stub_RSP.h: Stub for connection of SB filter with outside world
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_CS1_INTERFACE_STUB_RSP_H
#define LOFAR_CS1_INTERFACE_STUB_RSP_H

#include <CS1_Interface/CS1_Parset.h>

namespace LOFAR 
{
  namespace CS1
  {

    class Stub_RSP
    {
    public:
      // Create the stub. Get its parameters from the given file name.
      explicit Stub_RSP (bool onServer, const CS1_Parset *ps);

      ~Stub_RSP();

      // Connect the given objects to the stubs.
      void connect ();

    private:
      bool			       itsStubOnServer;
      const CS1_Parset                *itsCS1PS;
    };

  } // namespace CS1

} // namespace LOFAR

#endif

