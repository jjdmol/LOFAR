#ifndef LOFAR_RTCP_STORAGE_PROCESSES
#define LOFAR_RTCP_STORAGE_PROCESSES
 
#include <sys/time.h>
#include <Common/Thread/Semaphore.h>
#include <Common/Thread/Thread.h>
#include <Interface/Parset.h>
#include <Interface/SmartPtr.h>
#include <Interface/Stream.h>
#include <Interface/FinalMetaData.h>
#include <Storage/SSH.h>
#include <string>
#include <vector>

namespace LOFAR {
namespace RTCP {

class StorageProcesses;

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
      StorageProcess( StorageProcesses &manager, const Parset &parset, const std::string &logPrefix, int rank, const std::string &hostname );

      // calls stop(0)
      ~StorageProcess();

      void start();
      void stop( struct timespec deadline );
      bool isDone();

    private:
      void                               controlThread();

      StorageProcesses                   &itsManager;

      SmartPtr<SSHconnection>            itsSSHconnection;

      const Parset &itsParset;
      const std::string itsLogPrefix;

      const int itsRank;
      const std::string itsHostname;

      SmartPtr<Thread> itsThread;
};

/*
 * Manage a set of StorageProcess objects. The control sequence is as follows:
 *
 * 1. StorageProcess() creates and starts the StorageProcess objects from the
 *    parset.
 * 2. ... process observation ...
 * 3. forwardFinalMetaData(deadline) starts the FinalMetaDataGatherer, reads the
 *    final meta data and forwards it to the StorageProcess objects.
 * 4. stop(deadline) stops the StorageProcesses with a termination period.
 *
 * FinalMetaDataGatherer is started as:
 *     FinalMetaDataGatherer observationID
 *
 * A Storage process is expected to follow the following protocol: 
 *
  // establish control connection
  string resource = getStorageControlDescription(observationID, -1);
  PortBroker::ServerStream stream(resource);

  // read parset
  Parset parset(&stream);

  // write meta data
  FinalMetaData finalMetaData;
  finalMetaData.write(stream);
 */

class StorageProcesses {
public:
    // calls start()
    StorageProcesses( const Parset &parset, const std::string &logPrefix );

    // calls stop(0)
    ~StorageProcesses();

    // start the FinalMetaDataGatherer process and forward the obtained
    // meta data to the Storage processes. The deadline is an absolute time out.
    void forwardFinalMetaData( time_t deadline );

    // stop the processes and control threads, given an absolute time out.
    void stop( time_t deadline );

private:
    const Parset			 &itsParset;
    const std::string                    itsLogPrefix;

    std::vector<SmartPtr<StorageProcess> > itsStorageProcesses;
    FinalMetaData                        itsFinalMetaData;
    Semaphore                            itsFinalMetaDataAvailable;

    // start the processes and control threads
    void start();

    void finalMetaDataThread();

    // to access itsFinalMetaDataAvailable
    friend class StorageProcess;
};

}
}

#endif
