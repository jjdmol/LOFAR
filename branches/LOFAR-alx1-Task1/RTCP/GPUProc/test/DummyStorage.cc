#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Stream/PortBroker.h>
#include <Interface/Stream.h>
#include <Interface/FinalMetaData.h>
#include <string>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <Common/Thread/Mutex.h>

using namespace LOFAR;
using namespace RTCP;
using namespace std;

int observationID;
unsigned rank;

FinalMetaData origFinalMetaData;

Mutex logMutex;

void emulateStorage()
{
  // establish control connection
  string resource = getStorageControlDescription(observationID, rank);
  PortBroker::ServerStream stream(resource);

  // read and print parset
  Parset parset(&stream);
  {
    ScopedLock sl(logMutex);
    cout << "Storage: Parset received." << endl;
  }

  // read and print meta data
  FinalMetaData finalMetaData;
  finalMetaData.read(stream);
  {
    ScopedLock sl(logMutex);

    ASSERT(finalMetaData.brokenRCUsAtBegin == origFinalMetaData.brokenRCUsAtBegin);
    ASSERT(finalMetaData.brokenRCUsDuring  == origFinalMetaData.brokenRCUsDuring);

    cout << "Storage: FinalMetaData received and matches." << endl;
  }
}

void emulateFinalMetaDataGatherer()
{
  // establish control connection
  string resource = getStorageControlDescription(observationID, -1);
  PortBroker::ServerStream stream(resource);

  // read and print parset
  Parset parset(&stream);
  {
    ScopedLock sl(logMutex);
    cout << "FinalMetaDataGatherer: Parset received." << endl;
  }

  // set and write meta data
  origFinalMetaData.brokenRCUsAtBegin.push_back( FinalMetaData::BrokenRCU("CS001", "LBA", 2, "2012-01-01 12:34") );
  origFinalMetaData.brokenRCUsAtBegin.push_back( FinalMetaData::BrokenRCU("RS205", "HBA", 1, "2012-01-01 12:34") );
  origFinalMetaData.brokenRCUsDuring.push_back( FinalMetaData::BrokenRCU("DE601", "RCU", 3, "2012-01-01 12:34") );
  origFinalMetaData.write(stream);

  {
    ScopedLock sl(logMutex);
    cout << "FinalMetaDataGatherer: FinalMetaData sent." << endl;
  }
}

int main(int argc, char **argv)
{
  INIT_LOGGER("DummyStorage");

  ASSERT(argc == 4);

  observationID = boost::lexical_cast<int>(argv[1]);
  rank = boost::lexical_cast<unsigned>(argv[2]);
  //bool isBigEndian = boost::lexical_cast<bool>(argv[3]);

  // set up broker server
  PortBroker::createInstance(storageBrokerPort(observationID));

#pragma omp parallel sections
  {
#   pragma omp section
    try {
      emulateStorage();
    } catch (Exception &ex) {
      cout << "Storage caught exception: " << ex << endl;
    }

#   pragma omp section
    try {
      emulateFinalMetaDataGatherer();
    } catch (Exception &ex) {
      cout << "FinalMetaDataGatherer caught exception: " << ex << endl;
    }
  }
}

