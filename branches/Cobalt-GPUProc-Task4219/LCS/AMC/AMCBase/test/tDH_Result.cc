//# tDH_Result.cc: test program for the DH_Result class.
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <AMCBase/ConverterStatus.h>
#include <AMCBase/DH_Result.h>
#include <AMCBase/Direction.h>
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

    ConverterStatus sendStatus(ConverterStatus::ERROR, "Uh-oh, oops!");

    vector<Direction> sendDirection;
    sendDirection.push_back(Direction());
    sendDirection.push_back(Direction(0.4, -0.19));

    ConverterStatus recvStatus;
    vector<Direction> recvDirection;

    TH_Mem aTH;
    DH_Result sendDhRes;
    DH_Result recvDhRes;
    Connection conn("conn", &sendDhRes, &recvDhRes, &aTH, false);
    
    ResultData sendResData(sendDirection);
    ResultData recvResData(recvDirection);

    sendDhRes.writeBuf(sendStatus, sendResData);
    conn.write();
    conn.read();
    recvDhRes.readBuf(recvStatus, recvResData);

    ASSERT(sendStatus.get()  == recvStatus.get() &&
           sendStatus.text() == recvStatus.text());
    ASSERT(sendResData.direction.size() == recvResData.direction.size());
    for (uint i=0; i < sendResData.direction.size(); ++i) {
      ASSERT(sendResData.direction[i] == recvResData.direction[i]);
    }
  }
  catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }

  return 0;
}
