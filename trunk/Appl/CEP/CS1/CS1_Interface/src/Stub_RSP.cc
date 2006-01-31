//# Stub_RSP.cc: Stub for connection to DFTServer and DFTRequest
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


#include <CS1_Interface/Stub_RSP.h>
#include <Transport/TH_Socket.h>


using namespace LOFAR;
using namespace LOFAR::ACC::APS;

namespace LOFAR { 

  Stub_RSP::Stub_RSP (bool stubOnServer, const ACC::APS::ParameterSet& ps)
    : itsStubOnServer (stubOnServer),
      itsPS(ps)
  {
    // todo: add DH_?? for pre-correlation correction factors 
    //    for (int i=0; i<itsNSBF; i++) {
      //      itsSB.push_back(new DH_SubBand("noname",1)); //todo: get correct SubbandID
    //    }
  }

  Stub_RSP::~Stub_RSP()
  {}

  void Stub_RSP::connect ()
  {
  };

  //todo: add connections for pre-correlation correction DH_?? 

} //namespace

