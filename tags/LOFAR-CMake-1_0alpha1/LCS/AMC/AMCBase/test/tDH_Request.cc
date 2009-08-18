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
#include <AMCBase/Direction.h>
#include <AMCBase/Position.h>
#include <AMCBase/Epoch.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>
#include <Transport/TH_Mem.h>
#include <Transport/Connection.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

using namespace LOFAR;
using namespace LOFAR::AMC;

ostream& operator<<(ostream& os, const vector<double>& v)
{
  os << '[';
  for (uint i = 0; i < v.size()-1; ++i) {
    os << v[i] << ", ";
  }
  os << v[v.size()-1] << ']' << endl;
  return os;
}

int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    ConverterCommand sendCmd(ConverterCommand::J2000toAZEL);

    vector<Direction> sendDirection;
    sendDirection.push_back(Direction());
    sendDirection.push_back(Direction(0.4, -0.19));
  
    vector<Position> sendPosition;
    sendPosition.push_back(Position());
    sendPosition.push_back(Position(0.25*M_PI, -0.33*M_PI, 1));
    sendPosition.push_back(Position(-0.67*M_PI, 0.75*M_PI, 249.98));
  
    vector<Epoch>  sendEpoch;
    sendEpoch.push_back(Epoch());
    sendEpoch.push_back(Epoch(0));

    ConverterCommand recvCmd;
    vector<Direction> recvDirection;
    vector<Position> recvPosition;
    vector<Epoch> recvEpoch;

    TH_Mem aTH;
    DH_Request sendDhReq;
    DH_Request recvDhReq;
    Connection conn("conn", &sendDhReq, &recvDhReq, &aTH, false);

    RequestData sendReqData(sendDirection, sendPosition, sendEpoch);
    RequestData recvReqData(recvDirection, recvPosition, recvEpoch);

    sendDhReq.writeBuf(sendCmd, sendReqData);
    conn.write();
    conn.read();
    recvDhReq.readBuf(recvCmd, recvReqData);

    ASSERT(sendCmd == recvCmd);
    ASSERT(sendReqData.direction.size() == recvReqData.direction.size());
    for (uint i=0; i < sendReqData.direction.size(); ++i) {
      ASSERT(sendReqData.direction[i] == recvReqData.direction[i]);
    }
    ASSERT(sendReqData.position.size() == recvReqData.position.size());
    for (uint i=0; i < sendReqData.position.size(); ++i) {
      ASSERT(sendReqData.position[i] == recvReqData.position[i]);
    }
    ASSERT(sendReqData.epoch.size() == recvReqData.epoch.size());
    for (uint i=0; i < sendReqData.epoch.size(); ++i) {
      ASSERT(sendReqData.epoch[i] == recvReqData.epoch[i]);
    }
  }
  catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }

  return 0;
}
