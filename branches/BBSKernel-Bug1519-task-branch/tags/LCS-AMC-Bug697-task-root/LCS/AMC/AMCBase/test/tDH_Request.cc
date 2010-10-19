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
#include <AMCBase/DH_Request.h>
#include <AMCBase/ConverterCommand.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/EarthCoord.h>
#include <AMCBase/TimeCoord.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  try {

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

    ConverterCommand recvCmd;
    vector<SkyCoord> recvSky;
    vector<EarthCoord> recvEarth;
    vector<TimeCoord> recvTime;

    TH_Mem aTH;
    DH_Request sendDhReq;
    DH_Request recvDhReq;
    Connection conn("conn", &sendDhReq, &recvDhReq, &aTH, false);

    RequestData sendReqData(sendSky, sendEarth, sendTime);
    RequestData recvReqData(recvSky, recvEarth, recvTime);

    sendDhReq.writeBuf(sendCmd, sendReqData);
    conn.write();
    conn.read();
    recvDhReq.readBuf(recvCmd, recvReqData);

    ASSERT(sendCmd == recvCmd);
    ASSERT(sendReqData.skyCoord.size() == recvReqData.skyCoord.size());
    for (uint i=0; i < sendReqData.skyCoord.size(); ++i) {
      ASSERT(sendReqData.skyCoord[i] == recvReqData.skyCoord[i]);
    }
    ASSERT(sendReqData.earthCoord.size() == recvReqData.earthCoord.size());
    for (uint i=0; i < sendReqData.earthCoord.size(); ++i) {
      ASSERT(sendReqData.earthCoord[i] == recvReqData.earthCoord[i]);
    }
    ASSERT(sendReqData.timeCoord.size() == recvReqData.timeCoord.size());
    for (uint i=0; i < sendReqData.timeCoord.size(); ++i) {
      ASSERT(sendReqData.timeCoord[i] == recvReqData.timeCoord[i]);
    }
  }
  catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }

  return 0;
}
