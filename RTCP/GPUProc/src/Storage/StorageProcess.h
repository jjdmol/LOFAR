#ifndef LOFAR_RTCP_STORAGE_PROCESS
#define LOFAR_RTCP_STORAGE_PROCESS
 
#include <sys/time.h>
#include <Common/Thread/Trigger.h>
#include <Common/Thread/Thread.h>
#include <Interface/Parset.h>
#include <Interface/SmartPtr.h>
#include <Interface/FinalMetaData.h>
#include <string>

namespace LOFAR {
namespace RTCP {

/* A single Storage process.
 *
 * Storage is started as:
 *     Storage_main observationID rank isBigEndian
 *
 * A Storage process is expected to follow the following protocol: 
 *
  // establish control connection
  string resource = getStorageControlDescription(observationID, rank);
  PortBroker::ServerStream stream(resource);

  // read parset
  Parset parset(&stream);

  ... process observation ...

  // read meta data
  FinalMetaData finalMetaData;
  finalMetaData.read(stream);
 */

class StorageProcess {
    public:
      // user must call start()
      StorageProcess( const Parset &parset, const std::string &logPrefix, int rank, const std::string &hostname, FinalMetaData &finalMetaData, Trigger &finalMetaDataAvailable );

      // calls stop(0)
      ~StorageProcess();

      void start();
      void stop( struct timespec deadline );
      bool isDone();

    private:
      void                               controlThread();

      const Parset &itsParset;
      const std::string itsLogPrefix;

      const int itsRank;
      const std::string itsHostname;

      FinalMetaData                      &itsFinalMetaData;
      Trigger                            &itsFinalMetaDataAvailable;

      SmartPtr<Thread> itsThread;
};

}
}

#endif
