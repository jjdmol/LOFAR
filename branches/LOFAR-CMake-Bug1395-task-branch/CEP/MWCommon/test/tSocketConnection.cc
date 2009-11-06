//#  tSocketConnection.cc: Program to test SocketConnection
//#
//#  Copyright (C) 2007
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
//#  $Id$

#include <lofar_config.h>

#include <MWCommon/SocketConnection.h>
#include <MWCommon/SocketListener.h>
#include <Common/LofarLogger.h>
#include <iostream>
#include <unistd.h>

using namespace LOFAR::CEP;
using namespace std;

void doClient (const string& host, const string& port)
{
  //  sleep (2);
  cout << "Client connection on host " << host
       << ", port " << port << endl;
  SocketConnection socket(host, port);
  double dv = 1;
  socket.send (&dv, sizeof(dv));
  cout << "sent " << dv << endl;
  float fv = 0;
  socket.receive (&fv, sizeof(fv));
  ASSERT (fv == 2);
  cout << "received " << fv << endl;
  sleep(2);
  socket.receive (&fv, sizeof(fv));
  ASSERT (fv == 3);
  cout << "received " << fv << endl;
  dv = 2;
  socket.send (&dv, sizeof(dv));
  cout << "sent " << dv << endl;
}

void doServer (const string& port)
{
  cout << "Server connection on port " << port << endl;
  SocketListener listener(port);
  SocketConnection::ShPtr socket = listener.accept();
  double dv = 0;
  socket->receive (&dv, sizeof(dv));
  ASSERT (dv == 1);
  cout << "received " << dv << endl;
  float fv = 2;
  socket->send (&fv, sizeof(fv));
  cout << "sent " << fv << endl;
  fv = 3;
  socket->send (&fv, sizeof(fv));
  cout << "sent " << fv << endl;
  socket->receive (&dv, sizeof(dv));
  ASSERT (dv == 2);
  cout << "received " << dv << endl;
}

int main(int argc, const char* argv[])
{
  int status = 0;
  try {
    if (argc < 2) {
      cerr << "Run as:" << endl;
      cerr << "  as server:    tSocketConnection port" << endl;
      cerr << "  as client:    tSocketConnection port host" << endl;
    } else if (argc == 2) {
      doServer (argv[1]);
    } else {
      doClient (argv[2], argv[1]);
    }
  } catch (std::exception& x) {
    cout << "Unexpected exception in " << argv[0] << ": " << x.what() << endl;
    status = 1;
  }
  exit(status);
}
