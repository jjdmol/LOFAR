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

#include <MWCommon/SocketConnectionSet.h>
#include <MWCommon/MWBlobIO.h>
#include <Common/LofarLogger.h>
#include <iostream>
#include <unistd.h>

using namespace LOFAR::CEP;
using namespace LOFAR;
using namespace std;

void doClient (const string& host, const string& port, const string& name)
{
  cout << "Client connection on host " << host
       << ", port " << port << ", name=" << name << endl;
  SocketConnection conn(host, port);
  BlobString buf;
  MWBlobOut bbo(buf, 1, 2, 3);
  bbo.blobStream() << name;
  bbo.finish();
  cout << "client " << name << " sends " << buf.size() << " bytes" << endl;
  conn.write(buf);
  conn.read (buf);
  cout << "client " << name << " received " << buf.size() << " bytes" << endl;
  MWBlobIn bbi(buf);
  double dv;
  int32 iv;
  bbi.blobStream() >> dv >> iv;
  bbi.finish();
  ASSERT (dv == 1  &&  iv == 4);
}

void doServer (const string& port)
{
  SocketListener listener(port);
  SocketConnectionSet sockSet(listener);
  cout << "Server connectionset on port " << port << endl;
  sockSet.addConnections (2);
  cout << ">>>" << endl;
  for (int i=0; i<sockSet.size(); ++i) {
    BlobString buf;
    sockSet.read (i, buf);
    cout << "server received " << buf.size() << " bytes" << endl;
    MWBlobIn bbi(buf);
    string name;
    bbi.blobStream() >> name;
    bbi.finish();
    cout << "name=" << name << endl;
  }
  cout << "<<<" << endl;
  BlobString buf;
  MWBlobOut bbo(buf, 2, 3, 4);
  bbo.blobStream() << double(1) << int32(4);
  bbo.finish();
  cout << "server sends " << buf.size() << " bytes" << endl;
  sockSet.writeAll (buf);
}

int main(int argc, const char* argv[])
{
  int status = 0;
  try {
    if (argc < 2) {
      cerr << "Run as:" << endl;
      cerr << "  as server:    tSocketConnection port" << endl;
      cerr << "  as client:    tSocketConnection port host name" << endl;
    } else if (argc == 2) {
      doServer (argv[1]);
    } else {
      doClient (argv[2], argv[1], argv[3]);
    }
  } catch (std::exception& x) {
    cout << "Unexpected exception in " << argv[0] << ": " << x.what() << endl;
    status = 1;
  }
  exit(status);
}
