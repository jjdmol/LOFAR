//#  CEPEcho.cc: a test program for the TH_Socket class
//#
//#  Copyright (C) 2002-2003
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

#include <lofar_config.h>

#include <DH_EchoPing.h>
#include <Transport/TH_Socket.h>
#include <Transport/Connection.h>

namespace LOFAR
{
 namespace GCF
 {
  namespace Test
  {

void echo ()
{
  DH_EchoPing DH_Echo;
  DH_Echo.init();
  TH_Socket proto(string("localhost"), string("8923"), true);
  proto.init();
  Connection readConn("read",  0, &DH_Echo, &proto, true);
  Connection writeConn("write", &DH_Echo, 0, &proto, true);

  while(1)
  {
    readConn.read();
    
    fprintf(stderr, "PING received (seqnr=%d)\n", DH_Echo.getSeqNr());
    writeConn.write();
  }
}
  } // namespace Test
 } // namespace GCF
} // namespace LOFAR

using namespace LOFAR::GCF;

int main (int , const char** )
{
  INIT_LOGGER("mac.log_prop");

  LOG_INFO("MACProcessScope: GCF.TEST.CEP.CEPEcho");

  try 
  {
    Test::echo();
  } 
  catch (std::exception& x) 
  {
    fprintf(stderr, "Unexpected exception in 'echo': %s\n", x.what());
    return 1;
  }
  return 0;
}
