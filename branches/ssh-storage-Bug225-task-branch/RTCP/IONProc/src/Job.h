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
#include <Interface/Mutex.h>
#include <Interface/Parset.h>
#include <Interface/Thread.h>
#include <JobQueue.h>
#include <Stream/Stream.h>
#include <WallClockTime.h>

#include <sys/time.h>

#include <vector>


namespace LOFAR {
namespace RTCP {


class Job
{
  public:
					 Job(const char *parsetName);
					 ~Job();

    void				 cancel();

    const Parset			 itsParset;

  private:
    friend class JobQueue;

    void				 checkParset() const;
    void				 createCNstreams();
    void				 configureCNs(), unconfigureCNs();

    void				 createIONstreams(), deleteIONstreams();
    void				 barrier();
    template <typename T> void		 broadcast(T &);

    bool				 isCancelled();

    void				 jobThread();
    template <typename SAMPLE_TYPE> void doObservation();

    template <typename SAMPLE_TYPE> void attachToInputSection();
    template <typename SAMPLE_TYPE> void detachFromInputSection();

    static void				 execSSH(const char *sshKey, const char *userName, const char *hostName, const char *executable, const char *rank, const char *parset);
    static void				 forkSSH(const char *sshKey, const char *userName, const char *hostName, const char *executable, const char *rank, const char *parset, int &storagePID);
    static void				 joinSSH(int childPID, const std::string &hostName);

    void				 startStorageProcesses();
    void				 stopStorageProcesses();

    void				 waitUntilCloseToStartOfObservation(time_t secondsPriorToStart);

    std::vector<std::string>		 itsStorageHostNames;
    std::vector<int>			 itsStoragePIDs;

    std::vector<Stream *>		 itsCNstreams, itsIONstreams;
    unsigned				 itsNrRuns;
    Thread				 *itsJobThread;
    bool				 itsHasPhaseOne, itsHasPhaseTwo, itsHasPhaseThree;
    bool				 itsIsRunning, itsDoCancel;

    unsigned				 itsNrRunTokens, itsNrRunTokensPerBroadcast;

    unsigned				 itsJobID, itsObservationID;
    static unsigned			 nextJobID;

    WallClockTime			 itsWallClockTime;

    static void				 *theInputSection;
    static Mutex			 theInputSectionMutex;
    static unsigned			 theInputSectionRefCount;
};


} // namespace RTCP
} // namespace LOFAR

#endif
