//# Stub_BGL_Subband.h: Stub for connection of BGL Input with outside world
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_INTERFACE_STUB_BGL_SUBBAND_H
#define LOFAR_APPL_CEP_CS1_CS1_INTERFACE_STUB_BGL_SUBBAND_H

#include <CS1_Interface/Stub_BGL.h>

namespace LOFAR {

class Stub_BGL_Subband : public Stub_BGL
{
public:
  Stub_BGL_Subband(bool iAmOnBGL, unsigned dhIndex, const ACC::APS::ParameterSet &);

protected:
  virtual TransportHolder *newClientTH(unsigned subband, unsigned slave);
  virtual TransportHolder *newServerTH(unsigned subband, unsigned slave);
  
private:
  vector<string> servers, services;
};

} //namespace

#endif //include guard 

