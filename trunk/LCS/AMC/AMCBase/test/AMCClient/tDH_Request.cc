//#  tDH_Request.cc: test program for the DH_Request class.
//#
//#  Copyright (C) 2005
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <AMCBase/AMCClient/DH_Request.h>
#include <AMCBase/AMCClient/ConverterCommand.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    double usec = 1e-6;

    ConverterCommand sendCmd(ConverterCommand::J2000toAZEL);
    
    vector<SkyCoord> sendSky;
    sendSky.push_back(SkyCoord());
    sendSky.push_back(SkyCoord(0.4, -0.19));
  
    vector<EarthCoord> sendEarth;
    sendEarth.push_back(EarthCoord());
    sendEarth.push_back(EarthCoord(0.25*M_PI, -0.33*M_PI));
    sendEarth.push_back(EarthCoord(-0.67*M_PI, 0.75*M_PI, 249.98));
  
    vector<TimeCoord>  sendTime;
    sendTime.push_back(TimeCoord());
    sendTime.push_back(TimeCoord(0));

    TH_Mem aTH;
    DH_Request sendReq;
    DH_Request recvReq;
    Connection conn("conn", &sendReq, &recvReq, &aTH, false);
    

    sendReq.writeBuf(sendCmd, sendSky, sendEarth, sendTime);
    conn.write();
//    sendConn.waitForWrite();  // !!?? no TH_Mem::waitForSent() ??!!

    ConverterCommand recvCmd;
    vector<SkyCoord> recvSky;
    vector<EarthCoord> recvEarth;
    vector<TimeCoord> recvTime;

    conn.read();
//     recvConn.waitForRead();
    recvReq.readBuf(recvCmd, recvSky, recvEarth, recvTime);

    ASSERT(sendCmd.get() == recvCmd.get());
    ASSERT(sendSky.size() == recvSky.size());
    for (uint i=0; i < sendSky.size(); ++i) {
      ASSERT(sendSky[i].angle0() == recvSky[i].angle0());
      ASSERT(sendSky[i].angle1() == recvSky[i].angle1());
    }
    ASSERT(sendEarth.size() == recvEarth.size());
    for (uint i=0; i < sendEarth.size(); ++i) {
      ASSERT(sendEarth[i].longitude() == recvEarth[i].longitude());
      ASSERT(sendEarth[i].latitude()  == recvEarth[i].latitude());
      ASSERT(sendEarth[i].height()    == recvEarth[i].height());
    }
    ASSERT(sendTime.size() == recvTime.size());
    for (uint i=0; i < sendTime.size(); ++i) {
      ASSERT(sendTime[i].mjd() - recvTime[i].mjd() < usec);
    }

  }
  catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }

  return 0;
}
