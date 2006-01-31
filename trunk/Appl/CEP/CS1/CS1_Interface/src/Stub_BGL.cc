//# Stub_BGL.cc: Base class for BG/L stubs
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


#include <CS1_Interface/Stub_BGL.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>


using namespace LOFAR;

namespace LOFAR { 

unsigned Stub_BGL::itsNrSubbands, Stub_BGL::itsNrSlavesPerSubband;


Stub_BGL::Stub_BGL(bool iAmOnBGL, bool isInput, unsigned channel, const ACC::APS::ParameterSet &pSet)
  : itsIAmOnBGL(iAmOnBGL),
    itsIsInput(isInput),
    itsChannel(channel),
    itsTHs(0),
    itsConnections(0)
{
  if (itsNrSubbands == 0) { // first time
    itsNrSubbands	  = pSet.getInt32("Data.NSubbands");
    itsNrSlavesPerSubband = pSet.getInt32("BGLProc.SlavesPerSubband");
  }

  size_t size = itsNrSubbands * itsNrSlavesPerSubband;

  itsTHs = new TransportHolder * [size];
  memset(itsTHs, 0, sizeof(TransportHolder * [size]));

  itsConnections = new Connection * [size];
  memset(itsConnections, 0, sizeof(TransportHolder * [size]));
}


Stub_BGL::~Stub_BGL()
{
  if (itsTHs != 0 && itsConnections != 0) {
    for (int i = 0; i < itsNrSubbands * itsNrSlavesPerSubband; i ++) {
      delete itsTHs[i];
      delete itsConnections[i];
    }

    delete [] itsTHs;
    delete [] itsConnections;
  }
}


void Stub_BGL::connect(unsigned subband, unsigned slave, TinyDataManager &dm)
{
  size_t index = subband * itsNrSlavesPerSubband + slave;

  ASSERTSTR(subband < itsNrSubbands, "subband argument out of bounds; "
	   << subband << " / " << itsNrSubbands);
  ASSERTSTR(slave < itsNrSlavesPerSubband, "slave argument out of bounds; "
	   << slave << " / " << itsNrSlavesPerSubband);
  ASSERTSTR(itsTHs[index] == 0, "already connected: subband = "
	   << subband << ", slave = " << slave << ", channel = " << itsChannel);

  itsTHs[index] = itsIAmOnBGL ? newClientTH(subband, slave) : newServerTH(subband, slave);

  if (itsIsInput) {
    itsConnections[index] = new Connection("output", 0, dm.getGeneralInHolder(itsChannel), itsTHs[index], true);
    dm.setInConnection(itsChannel, itsConnections[index]);
  } else {
    itsConnections[index] = new Connection("input", dm.getGeneralOutHolder(itsChannel), 0, itsTHs[index], true);
    dm.setOutConnection(itsChannel, itsConnections[index]);
  }
};

} //namespace
