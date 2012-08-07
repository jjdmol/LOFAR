//#  Job.h
//#
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
//#  $Id: ION_main.cc 15296 2010-03-24 10:19:41Z romein $


//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#if !defined LOFAR_RTCP_JOB_H
#define LOFAR_RTCP_JOB_H

#include <InputSection.h>
#include <Interface/Parset.h>
#include <Interface/SmartPtr.h>
#include <Interface/MultiDimArray.h>
#include <JobQueue.h>
#include <Stream/Stream.h>
#include <WallClockTime.h>
#include <Common/Thread/Mutex.h>
#include <Common/Thread/Queue.h>
#include <Common/Thread/Thread.h>
#include <PLCClient.h>
#include <SSH.h>

#include <sys/time.h>

#include <vector>
#include <string>


namespace LOFAR {
namespace RTCP {


class Job : public PLCRunnable
{
  public:
					 Job(const char *parsetName);
					 ~Job();

    void				 cancel();
    void				 printInfo() const;

    const Parset			 itsParset;
    const unsigned			 itsJobID, itsObservationID;

    // implement PLCRunnable
    virtual bool			 define();
    virtual bool			 init();
    virtual bool			 run();
    virtual bool			 pause(const double &when);
    virtual bool			 quit();
    virtual bool			 observationRunning();

  private:
    bool				 checkParset() const;
    void				 createCNstreams();
    bool				 configureCNs();
    void				 unconfigureCNs();

    void				 createIONstreams();
    void				 barrier();
    bool                                 agree(bool iAgree);
    template <typename T> void		 broadcast(T &);

    void				 claimResources();

    bool				 anotherRun();

    void				 jobThread();
    template <typename SAMPLE_TYPE> void doObservation();

    template <typename SAMPLE_TYPE> void attachToInputSection();
    template <typename SAMPLE_TYPE> void detachFromInputSection();

    void				 startStorageProcesses();
    void				 stopStorageProcesses();

    class StorageProcess {
    public:
      StorageProcess( const Parset &parset, const string &logPrefix, int rank, const string &hostname );
      ~StorageProcess();

      void start();
      void stop( struct timespec deadline );
    private:
      void                               controlThread();

#ifdef HAVE_LIBSSH2
      SmartPtr<SSHconnection>            itsSSHconnection;
#else      
      int itsPID;
#endif

      const Parset &itsParset;
      const std::string itsLogPrefix;

      const int itsRank;
      const std::string itsHostname;

      SmartPtr<Thread> itsThread;
    };

    void				 waitUntilCloseToStartOfObservation(time_t secondsPriorToStart);

    SmartPtr<Stream>			 itsPLCStream;
    SmartPtr<PLCClient>			 itsPLCClient;

    std::string                          itsLogPrefix;

    std::vector<SmartPtr<StorageProcess> > itsStorageProcesses;

    std::vector<Stream *>		 itsCNstreams, itsPhaseThreeCNstreams;
    Matrix<Stream *>			 itsPhaseOneTwoCNstreams;
    std::vector<SmartPtr<Stream> >	 itsIONstreams;
    bool				 itsHasPhaseOne, itsHasPhaseTwo, itsHasPhaseThree;
    bool				 itsIsRunning, itsDoCancel;

    unsigned                             itsBlockNumber;
    double                               itsRequestedStopTime, itsStopTime;
    unsigned				 itsNrBlockTokens, itsNrBlockTokensPerBroadcast;

    static unsigned			 nextJobID;

    WallClockTime			 itsWallClockTime;

    static void				 *theInputSection;
    static Mutex			 theInputSectionMutex;
    static unsigned			 theInputSectionRefCount;

    SmartPtr<Thread>			 itsJobThread;
};


extern Queue<Job *> finishedJobs;


} // namespace RTCP
} // namespace LOFAR

#endif
