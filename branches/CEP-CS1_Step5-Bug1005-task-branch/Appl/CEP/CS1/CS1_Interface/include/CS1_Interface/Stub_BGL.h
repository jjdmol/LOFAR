//# Stub_BGL.h: Stub for connection of BGL with outside world
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

#ifndef LOFAR_CS1_INTERFACE_STUB_BGL_H
#define LOFAR_CS1_INTERFACE_STUB_BGL_H

#if defined HAVE_TINYCEP && defined HAVE_APS

#include <tinyCEP/TinyDataManager.h>
#include <Transport/Connection.h>
#include <Transport/TransportHolder.h>
#include <CS1_Interface/CS1_Parset.h>

#include <map>
#include <string>

namespace LOFAR {
namespace CS1 {

class Stub_BGL
{
  // This is a base class that can be used to make connections from the BGL
  // application to the outside world.  Details are filled in by derived
  // classes that must provide the necessary TransportHolders.

  public:
    Stub_BGL(bool iAmOnBGL, bool isInput, const char *connectionName, const CS1_Parset *pSet);
    ~Stub_BGL();

    void connect(unsigned psetNr, unsigned coreNr, TinyDataManager &dm, unsigned channel);

  private:
    const CS1_Parset				     *itsCS1PS;
    bool					     itsIAmOnBGL, itsIsInput;
    string					     itsPrefix;
    map<pair<unsigned, unsigned>, TransportHolder *> itsTHs;
    map<pair<unsigned, unsigned>, Connection *>      itsConnections;
};

} // namespace CS1
} // namespace LOFAR

#endif // defined HAVE_TINYCEP && defined HAVE_APS
#endif
