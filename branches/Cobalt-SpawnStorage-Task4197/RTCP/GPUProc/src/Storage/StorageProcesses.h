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

/* A single Storage process */

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
 * Manage a Storage_main process (RTCP/Storage). The control sequence is as follows:
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
