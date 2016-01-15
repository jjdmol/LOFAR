//# DummyStorage.cc
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#include <lofar_config.h>

#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <boost/lexical_cast.hpp>

#include <Common/LofarLogger.h>
#include <Common/Thread/Mutex.h>
#include <Stream/PortBroker.h>
#include <CoInterface/Stream.h>
#include <CoInterface/FinalMetaData.h>
#include <CoInterface/Parset.h>
#include <Common/Exception.h>

using namespace LOFAR;
using namespace Cobalt;
using namespace std;

int observationID;
unsigned rank;

Exception::TerminateHandler th(Exception::terminate);

void emulateStorage()
{
  // establish control connection
  string resource = getStorageControlDescription(observationID, rank);
  PortBroker::ServerStream stream(resource);

  // read and print parset
  Parset parset(&stream);
  cout << "Storage: Parset received." << endl;

  // read and print meta data
  FinalMetaData finalMetaData;
  finalMetaData.read(stream);

  //ASSERT(finalMetaData.brokenRCUsAtBegin == origFinalMetaData.brokenRCUsAtBegin);
  //ASSERT(finalMetaData.brokenRCUsDuring  == origFinalMetaData.brokenRCUsDuring);

  cout << "Storage: FinalMetaData received and matches." << endl;

  // write completion signal
  bool sentFeedback = false;
  stream.write(&sentFeedback, sizeof sentFeedback);
  bool success = true;
  stream.write(&success, sizeof success);
}

int main(int argc, char **argv)
{
  INIT_LOGGER("DummyStorage");

  if (argc != 3) {
    cerr << "Usage: DummyStorage obsid rank" << endl;
    cerr << endl;
    cerr << "Emulates both an OutputProc and a FinalMetaDataGatherer instance." << endl;
    cerr << "Only the protocol with GPUProc is implemented, that is," << endl;
    cerr << "no data is actually received or written to disk." << endl;
    exit(1);
  }

  // Make sure DummyStorage always dies, even if the test
  // malfunctions.
  alarm(20);

  observationID = boost::lexical_cast<int>(argv[1]);
  rank = boost::lexical_cast<unsigned>(argv[2]);

  // set up broker server
  PortBroker::createInstance(storageBrokerPort(observationID));

  try {
    emulateStorage();
  } catch (Exception &ex) {
    cout << "Storage caught exception: " << ex << endl;
  }
}

