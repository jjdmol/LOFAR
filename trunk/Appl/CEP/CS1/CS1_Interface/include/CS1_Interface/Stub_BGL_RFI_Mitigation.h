//# Stub_BGL_RFI_Mitigation.h: Stub for connection of BGL with outside world
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

#if 0 // currently unused
#ifndef LOFAR_CS1_INTERFACE_STUB_BGL_RFI_MITIGATION_H
#define LOFAR_CS1_INTERFACE_STUB_BGL_RFI_MITIGATION_H

#include <CS1_Interface/Stub_BGL.h>

namespace LOFAR 
{
  namespace CS1
  {

    class Stub_BGL_RFI_Mitigation : public Stub_BGL
    {
    public:
      Stub_BGL_RFI_Mitigation(bool iAmOnBGL, const ACC::APS::ParameterSet &pSet)
        : Stub_BGL(iAmOnBGL, iAmOnBGL, pSet) {}

    protected:
      virtual TransportHolder *newClientTH(unsigned cell, unsigned node);
      virtual TransportHolder *newServerTH(unsigned cell, unsigned node);
    };

  } //namespace CS1

} //namespace LOFAR

#endif
#endif
