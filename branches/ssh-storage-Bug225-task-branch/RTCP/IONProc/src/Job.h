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

#include <Common/Semaphore.h>
#include <Interface/Mutex.h>
#include <Interface/Parset.h>
#include <Interface/RSPTimeStamp.h>
#include <Interface/Thread.h>
#include <Stream/Stream.h>

#include <vector>


namespace LOFAR {
namespace RTCP {


class Job
{
  public:
					Job(const char *parsetName);
					~Job();

    static void				waitUntilAllJobsAreFinished();

    const Parset			itsParset;

  private:
    void				checkParset() const;
    void				createCNstreams();
    void				configureCNs(), unconfigureCNs();

    void				createIONstreams(), deleteIONstreams();

    void				jobThread();
    template <typename SAMPLE_TYPE>	void CNthread();

    void				allocateResources();
    void				deallocateResources();

    void				attachToInputSection();
    void				detachFromInputSection();

    void				barrier();

    std::vector<Stream *>		itsCNstreams, itsIONstreams;
    unsigned				itsNrRuns;
    TimeStamp                           itsStopTime;
    Thread				*itsJobThread, *itsCNthread;
    bool				itsHasPhaseOne, itsHasPhaseTwo, itsHasPhaseThree;
    unsigned				itsJobID, itsObservationID;
    static unsigned			nextJobID;

    static void				*theInputSection;
    static Mutex			theInputSectionMutex;
    static unsigned			theInputSectionRefCount;

    static unsigned			theNrJobsCreated;
    static Semaphore			theNrJobsFinished;

    static Mutex			theJobQueueMutex;
    static Condition			theReevaluateJobQueue;
    static std::vector<Job *>		theRunningJobs;
};


} // namespace RTCP
} // namespace LOFAR

#endif
