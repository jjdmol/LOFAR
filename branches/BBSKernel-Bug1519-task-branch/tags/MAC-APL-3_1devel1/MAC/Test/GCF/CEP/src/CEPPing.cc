//#  CEPPing.cc: a test program for the TH_Socket class
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

#include <DH_EchoPing.h>
#include <Transport/TH_Socket.h>
#include <GCF/PALlight/CEPPropertySet.h>
#include <GCF/GCF_PVInteger.h>
#include <sys/time.h>
#include <signal.h>

using namespace LOFAR;
using namespace LOFAR::GCF::CEPPMLlight;

/**
 * Function to calculate the elapsed time between two tiemval's.
 */
static double time_elapsed(timeval* start, timeval* stop) 
{
  return stop->tv_sec-start->tv_sec 
    + (stop->tv_usec-start->tv_usec)/(double)1e6; 
}

void ping ()
{
  CEPPropertySet pingPS("C", "TTypeF", PS_CAT_TEMPORARY);
  
  pingPS.enable();

  DH_EchoPing DH_Echo("echo");
  DH_EchoPing DH_Ping("ping");
  DH_Ping.setID(1);
  DH_Echo.setID(2);
  TH_Socket proto("localhost", "", 8923, false);
  TH_Socket proto2("", "localhost", 8923, true);
  DH_Ping.connectBidirectional (DH_Echo, proto, proto2, true);
  DH_Ping.init();
    
  sleep(1);
  uint seqnr(0);
  uint max(200);
  timeval echoTime;
  timeval pingTime;
  while(1)
  {
    gettimeofday(&pingTime, 0);

    DH_Ping.setSeqNr(seqnr++);
    DH_Ping.setPingTime(pingTime);

    fprintf(stderr, "PING sent (seqnr=%d)\n", DH_Ping.getSeqNr());

    usleep(300000);
    DH_Ping.write();
    if (seqnr >= max) 
    {
      pingPS.disable();
      seqnr = 0;
    }

    if (seqnr == 10)
    {
      pingPS.enable();
    }
    
    if (pingPS.isMonitoringOn())
    {
      pingPS["sn"].setValue(GCFPVInteger(DH_Ping.getSeqNr()));
    }

    DH_Ping.read();  
    gettimeofday(&echoTime, 0);
    pingTime = DH_Ping.getPingTime();
    fprintf(stderr, "ECHO received (seqnr=%d): elapsed = %f sec.\n", DH_Ping.getSeqNr(),
        time_elapsed(&pingTime, &echoTime));  
  }
}

int main (int , const char** )
{
  INIT_LOGGER("mac.log_prop");
  signal(SIGPIPE, SIG_IGN);
  try 
  {
    ping();
  } catch (std::exception& x) {
    cerr << "Unexpected exception in 'ping': " << x.what() << endl;
    return 1;
  }
  return 0;
}
