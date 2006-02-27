//# Stub_BGL_RFI_Mitigation.cc: Stub for connection to DFTServer and DFTRequest
//#
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


#include <CS1_Interface/Stub_BGL_RFI_Mitigation.h>
#include <Transport/TH_Null.h>
#include <Transport/Connection.h>


using namespace LOFAR;

namespace LOFAR { 

TransportHolder *Stub_BGL_RFI_Mitigation::newClientTH(unsigned, unsigned)
{
  return new TH_Null();
}


TransportHolder *Stub_BGL_RFI_Mitigation::newServerTH(unsigned, unsigned)
{
  return new TH_Null();
}

} //namespace
