//#  tDH_Result.cc: test program for the DH_Result class.
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
#include <AMCBase/ConverterStatus.h>
#include <AMCBase/DH_Result.h>
#include <AMCBase/SkyCoord.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    ConverterStatus sendStat(ConverterStatus::ERROR, "Uh-oh, oops!");

    vector<SkyCoord> sendSky;
    sendSky.push_back(SkyCoord());
    sendSky.push_back(SkyCoord(0.4, -0.19));

    ConverterStatus recvStat;
    vector<SkyCoord> recvSky;

    TH_Mem aTH;
    DH_Result sendDhRes;
    DH_Result recvDhRes;
    Connection conn("conn", &sendDhRes, &recvDhRes, &aTH, false);
    
    ResultData sendResData(sendSky);
    ResultData recvResData(recvSky);

    sendDhRes.writeBuf(sendStat, sendResData);
    conn.write();
    conn.read();
    recvDhRes.readBuf(recvStat, recvResData);

    ASSERT(sendStat.get()  == recvStat.get() &&
           sendStat.text() == recvStat.text());
    ASSERT(sendResData.skyCoord.size() == recvResData.skyCoord.size());
    for (uint i=0; i < sendResData.skyCoord.size(); ++i) {
      ASSERT(sendResData.skyCoord[i] == recvResData.skyCoord[i]);
    }
  }
  catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }

  return 0;
}
