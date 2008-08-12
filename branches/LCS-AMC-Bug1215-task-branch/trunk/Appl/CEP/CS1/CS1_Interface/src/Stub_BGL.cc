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

#if defined HAVE_TINYCEP && defined HAVE_APS

#include <CS1_Interface/Stub_BGL.h>
#include <Transport/BGLConnection.h>
#include <Transport/TH_File.h>
#include <Transport/TH_Null.h>
#include <Transport/TH_Socket.h>

#include <cstring>


namespace LOFAR { 
namespace CS1 {

Stub_BGL::Stub_BGL(bool iAmOnBGL, bool isInput, const char *connectionName, const CS1_Parset *pSet)
:
  itsCS1PS(pSet),
  itsIAmOnBGL(iAmOnBGL),
  itsIsInput(isInput),
  itsPrefix(string("OLAP.OLAP_Conn.") + connectionName)
{
}


Stub_BGL::~Stub_BGL()
{
  for (map<pair<unsigned, unsigned>, TransportHolder *>::iterator it = itsTHs.begin(); it != itsTHs.end(); it ++)
    delete it->second;

  for (map<pair<unsigned, unsigned>, Connection *>::iterator it = itsConnections.begin(); it != itsConnections.end(); it ++)
    delete it->second;
}


void Stub_BGL::connect(unsigned psetNr, unsigned coreNr, TinyDataManager &dm, unsigned channel)
{
  pair<unsigned, unsigned> index(psetNr, coreNr);

  ASSERTSTR(itsTHs.find(index) == itsTHs.end(), "already connected: psetNr = " << psetNr << ", coreNr = " << coreNr);
   
  TransportHolder *th;
  string transportType = itsCS1PS->getString(itsPrefix + "_Transport");

  if (transportType == "TCP") {
    string server  = itsCS1PS->getStringVector(itsPrefix + "_ServerHosts")[psetNr];
    string service = itsCS1PS->getPortsOf(itsPrefix)[coreNr];
    th = itsIAmOnBGL ? new TH_Socket(server, service, false, Socket::TCP, false) : new TH_Socket(service, false, Socket::TCP, 5, false);
  } else if (transportType == "FILE") {
    string baseFileName = itsCS1PS->getString(itsPrefix + "_BaseFileName");
    char fileName[baseFileName.size() + 32];
    sprintf(fileName, "%s.%u.%u", baseFileName.c_str(), psetNr, coreNr);
    th = new TH_File(string(fileName), itsIsInput ? TH_File::Read : TH_File::Write);
#if 0
  } else if (transportType == "ZOID") {
    th = itsIAmOnBGL ? TH_ZoidClient() : TH_ZoidServer(coreNr);
#endif
  } else if (transportType == "NULL") {
    th = new TH_Null();
  } else {
    ASSERTSTR(false, transportType << ": unknown connector");
  }

  itsTHs[index] = th;

  if (itsIsInput) {
    Connection *conn = new BGLConnection("output", 0, dm.getGeneralInHolder(channel), th);
    itsConnections[index] = conn;
    dm.setInConnection(channel, conn);
  } else {
    Connection *conn = new BGLConnection("input", dm.getGeneralOutHolder(channel), 0, th);
    itsConnections[index] = conn;
    dm.setOutConnection(channel, conn);
  }
};

} // namespace CS1
} // namespace LOFAR

#endif // defined HAVE_TINYCEP && defined HAVE_APS
