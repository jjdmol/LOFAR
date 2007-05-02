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
#include <Transport/BGLConnection.h>
#include <Transport/TH_File.h>
#include <Transport/TH_Null.h>
#include <Transport/TH_Socket.h>

#include <cstring>


namespace LOFAR { 
namespace CS1 {

Stub_BGL::Stub_BGL(bool iAmOnBGL, bool isInput, const char *connectionName, const ACC::APS::ParameterSet &pSet)
:
  itsPS(pSet)
{
  itsIAmOnBGL	    = iAmOnBGL;
  itsIsInput	    = isInput;
  itsPrefix	    = string("Connections.") + connectionName;
  itsTHs	    = 0;
  itsConnections    = 0;
  int psetsPerCell  = pSet.getInt32("BGLProc.PsetsPerCell");
  itsNrCells	    = pSet.getInt32("Observation.NSubbands") / (pSet.getInt32("General.SubbandsPerPset") * psetsPerCell);
  itsNrNodesPerCell = pSet.getInt32("BGLProc.NodesPerPset") * psetsPerCell;
  itsNrPsetsPerStorage = pSet.getInt32("Storage.PsetsPerStorage");

  size_t size = itsNrCells * itsNrNodesPerCell;
  if ((!itsIAmOnBGL && isInput) || (itsIAmOnBGL && !itsIsInput)) {
    size *= itsNrPsetsPerStorage;
  }

  itsTHs = new TransportHolder * [size];
  memset(itsTHs, 0, sizeof(TransportHolder * [size]));

  itsConnections = new Connection * [size];
  memset(itsConnections, 0, sizeof(TransportHolder * [size]));
}


Stub_BGL::~Stub_BGL()
{
  if (itsTHs != 0 && itsConnections != 0) {
    size_t size = itsNrCells * itsNrNodesPerCell;
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

  size_t index;
  if ((!itsIAmOnBGL && itsIsInput) || (itsIAmOnBGL && !itsIsInput)) {
    index = cellNr * itsNrNodesPerCell * itsNrPsetsPerStorage + nodeNr;
  } else {
    index = cellNr * itsNrNodesPerCell + nodeNr;
  }

  ASSERTSTR(cellNr < itsNrCells, "cellNr argument out of bounds; " << cellNr << " / " << itsNrCells);
  ASSERTSTR(itsTHs[index] == 0, "already connected: cellNr = " << cellNr << ", nodeNr = " << nodeNr);

  if ((!itsIAmOnBGL && itsIsInput) || (itsIAmOnBGL && !itsIsInput)) {
    ASSERTSTR(nodeNr < itsNrNodesPerCell * itsNrPsetsPerStorage, "nodeNr argument out of bounds; " << nodeNr << "/" << itsNrNodesPerCell * itsNrPsetsPerStorage);
  } else {
    ASSERTSTR(nodeNr < itsNrNodesPerCell, "nodeNr argument out of bounds; " << nodeNr << " / " << itsNrNodesPerCell);
  }


  TransportHolder *th;
  string transportType = itsPS.getString(itsPrefix + ".TransportHolder");

  if (transportType == "TCP") {
    string server  = itsPS.getStringVector(itsPrefix + ".ServerHosts")[cellNr];
    string service = itsPS.getStringVector(itsPrefix + ".Ports")[nodeNr];

    th = itsIAmOnBGL ? new TH_Socket(server, service, false, Socket::TCP, false) : new TH_Socket(service, false, Socket::TCP, 5, false);
  } else if (transportType == "FILE") {
    string baseFileName = itsPS.getString(itsPrefix + ".BaseFileName");
    char fileName[baseFileName.size() + 32];
    sprintf(fileName, "%s.%u.%u", baseFileName.c_str(), cellNr, nodeNr);
    th = new TH_File(string(fileName), itsIsInput ? TH_File::Read : TH_File::Write);
#if 0
  } else if (transportType == "ZOID") {
    th = itsIAmOnBGL ? TH_ZoidClient() : TH_ZoidServer(nodeNr);
#endif
  } else if (transportType == "NULL") {
    th = new TH_Null();
  } else {
    ASSERTSTR(false, transportType << ": unknown connector");
  }

  itsTHs[index] = th;

  if (itsIsInput) {
    itsConnections[index] = new BGLConnection("output", 0, dm.getGeneralInHolder(channel), th);
    dm.setInConnection(channel, itsConnections[index]);
  } else {
    itsConnections[index] = new BGLConnection("input", dm.getGeneralOutHolder(channel), 0, th);
    dm.setOutConnection(channel, itsConnections[index]);
  }
};

} // namespace CS1
} // namespace LOFAR
