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

#include <lofar_config.h>

#include <CS1_Interface/Stub_BGL.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>


namespace LOFAR { 
namespace CS1 {

unsigned Stub_BGL::theirNrCells;
unsigned Stub_BGL::theirNrNodesPerCell;


Stub_BGL::Stub_BGL(bool iAmOnBGL, bool isInput, const ACC::APS::ParameterSet &pSet)
:
  itsIAmOnBGL(iAmOnBGL),
  itsIsInput(isInput),
  itsTHs(0),
  itsConnections(0)
{
  if (theirNrCells == 0) { // first time
    theirNrCells	   = pSet.getInt32("Observation.NSubbands") / pSet.getInt32("General.NSubbandsPerCell");
    theirNrNodesPerCell    = pSet.getInt32("BGLProc.NodesPerCell");
  }

  size_t size = theirNrCells * theirNrNodesPerCell;

  itsTHs = new TransportHolder * [size];
  memset(itsTHs, 0, sizeof(TransportHolder * [size]));

  itsConnections = new Connection * [size];
  memset(itsConnections, 0, sizeof(TransportHolder * [size]));
}


Stub_BGL::~Stub_BGL()
{
  if (itsTHs != 0 && itsConnections != 0) {
    size_t size = theirNrCells * theirNrNodesPerCell;
    for (uint i = 0; i < size; i ++) {
      delete itsTHs[i];
      delete itsConnections[i];
    }

    delete [] itsTHs;
    delete [] itsConnections;
  }
}


void Stub_BGL::connect(unsigned cellNr, unsigned nodeNr, TinyDataManager &dm, unsigned channel)
{
  size_t index = cellNr * theirNrNodesPerCell + nodeNr;

  ASSERTSTR(cellNr < (theirNrCells), "cellNr argument out of bounds; "
	    << cellNr << " / " << theirNrCells);
  ASSERTSTR(nodeNr < theirNrNodesPerCell, "nodeNr argument out of bounds; "
	    << nodeNr << " / " << theirNrNodesPerCell);
  ASSERTSTR(itsTHs[index] == 0, "already connected: cellNr = "
	    << cellNr << ", nodeNr = " << nodeNr);

  itsTHs[index] = itsIAmOnBGL ? newClientTH(cellNr, nodeNr) : newServerTH(cellNr, nodeNr);

  if (itsIsInput) {
    itsConnections[index] = new Connection("output", 0, dm.getGeneralInHolder(channel), itsTHs[index], false);
    dm.setInConnection(channel, itsConnections[index]);
  } else {
    itsConnections[index] = new Connection("input", dm.getGeneralOutHolder(channel), 0, itsTHs[index], false);
    dm.setOutConnection(channel, itsConnections[index]);
  }
};

} // namespace CS1
} // namespace LOFAR
