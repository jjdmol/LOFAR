//  SolverControlAgent.h: abstract base for solver control agents
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#ifndef SOLVERCONTROL_SRC_SOLVERCONTROLAGENT_H_HEADER_INCLUDED_E43EA503
#define SOLVERCONTROL_SRC_SOLVERCONTROLAGENT_H_HEADER_INCLUDED_E43EA503
    
#include <AppAgent/AppControlAgent.h>
#include <SolverControl/AID-SolverControl.h>
#include <Common/Thread/Mutex.h>

#pragma aidgroup SolverControl
#pragma aid Start End Stop Iteration Solution Solver Control Message Convergence 
#pragma aid Next Step Domain Num All Params Solved Index

namespace AppState
{
  // define additional control states for the solver
  //##ModelId=3E00AA5100D5
  typedef enum
  {
    NEXT_SOLUTION  = 0x100,   // should go on to next solution
    NEXT_DOMAIN    = 0x101,   // should go on to next domain

  }
  ExtraSolverControlStates;
};

    
namespace SolverControl {

using namespace AppControlAgentVocabulary;

const HIID 
    FSolverControlParams = AidSolver|FControlParams,
    
    
    // Constants for event names
    //    Posted at start of every solve domain
    StartDomainEvent    = AidStart|AidDomain,
    //    Posted at start of every solution
    StartSolutionEvent  = AidStart|AidSolution,
    //    Posted at end of every iteration
    EndIterationEvent   = AidEnd|AidIteration,
    //    Posted when a solution is ended 
    EndSolutionEvent    = AidEnd|AidSolution,
    //    Posted when a domain has been solved for
    EndDomainEvent      = AidEnd|AidDomain,
    //    Posted on close(), when the solver is finished
    SolverEndEvent      = AidSolver|AidEnd,
    
    //    Posted when a solution is interrupted 
    StopSolutionEvent   = AidStop|AidSolution,
    
    //    Returned to app to tell it to proceed with to the next solution
    NextSolutionEvent   = AidNext|AidSolution,
    //    Returned to app to tell it to proceed with to the next domain
    NextDomainEvent     = AidNext|AidSolution,
    //    Returned to app to tell it to halt everything
    // AppControlAgent::StopEvent() 

    // Constants for field names
    //    Convergence parameter (in status record)
    FConvergence        = AidConvergence,
    //    Iteration number (in status record)
    FIterationNumber    = AidIteration|AidNum,
    //    Iteration number (in status record)
    FDomainNumber       = AidDomain|AidIndex,
    //    Ending message (in status record)
    FMessage            = AidMessage,
    //    Solution parameters (in status record)
    FSolutionParams     = AidSolution|AidParams;
    
//##ModelId=3DFF2B6D01FF
//##Documentation
//## This is an abstract prototype class for solver control agents. It
//## implements a few features common to all such agents; specifically, it
//## keeps track of solution parameters (convergence & such). See method
//## documentation for details.
//## 
//## The class also includes a mutex member; all functions obtain a lock on
//## this mutex prior to operation. This makes the control agent thread-safe.
class SolverControlAgent : public AppControlAgent
{
  public:
    //##ModelId=3E01FD1D03DB
    //##Documentation
    //## inits various counters
    virtual bool init (const DataRecord &data);
  
    //##ModelId=3DFF2D300027
    //##Documentation
    //## Called by application to start solving for a new domain.
    //## Should clear all parameters and reset the solution queue to beginning.
    //## Base version simply posts a StartDomainEvent with the specified
    //## data record. The record is meant to identify the domain.
    virtual int startDomain   (const DataRecord::Ref &data = DataRecord::Ref());
  
    //##ModelId=3E01F9A203A4
    //##Documentation
    //## Called by application to start a solution within a domain
    //## Should clear all solution parameters and reset iteration count to 0
    //## (by calling initSolution(), below.)
    //## Returns a record of the solve parameters for the next solution job.
    //## Base version simply posts a StartSolutionEvent
    virtual int startSolution (DataRecord::Ref &params) =0;
    
    //##ModelId=3E01F9A302C5
    //##Documentation
    //## Called by application to indicate that it has finished with a particular
    //## domain.
    //## Base version simply posts a EndDomainEvent with the specified
    //## data record. The record is meant to identify the domain.
    virtual int endDomain (const DataRecord::Ref &data = DataRecord::Ref());
    
    //##ModelId=3E005C9C0382
    //##Documentation
    //## Called by the solver application at end of every iteration.
    //## This updates the convergence parameter, increments the iteration
    //## count, stuffs them into the status record, and posts an
    //## EndIterationEvent containing the status record.
    //## (NB: the application may also fill the status record with any other
    //## relevant information prior to calling endIteration().)
    //## 
    //## Returns True if solving is to continue, or False to stop. Base version
    //## always returns True, but subclasses will override this function to
    //## provide more meaningful behavior. Note that if False is returned here
    //## by a child class, then stop_flag_ should also be raised. The
    //## application may watch for StopEvents() and/or check the isStopped()
    //## function and/or rely on the return code.
    virtual int endIteration (double conv);

    
    //##ModelId=3DFF2D6400EA
    //##Documentation
    //## Clears flags and solution parameters
    virtual void close ();
    
    //##ModelId=3DFF5B240397
    //##Documentation
    //## Called to interrupt a solution. Sets the new state and generates
    //## the given event on behalf of the application. The current status 
    //## record will be attached to the event. If a message is supplied,
    //## it will be placed into the status record as [FMessage].
    void stopSolution (const string &msg = "", int newstate = AppState::NEXT_SOLUTION,
                        const HIID &event = StopSolutionEvent );
    
    //##ModelId=3E00650B036F
    //## Alias for stopSolution(msg,newstate,EndSolutionEvent)
    //## Meant to be called to end a successful (i.e. converged) solution
    void endSolution  (const string &msg = "", int newstate = AppState::NEXT_SOLUTION)
    { stopSolution(msg,newstate,EndSolutionEvent); }
    
    //##ModelId=3DFF37F3018B
    //##Documentation
    //## Returns current convergence parameter
    double convergence () const;

    //##ModelId=3DFF37F30341
    //##Documentation
    //## Returns value of iteration counter
    int iterationNum () const;
    
    //##ModelId=3E01FD0E011A
    //##Documentation
    //## Returns value of domain counter
    int domainNum () const;
    
    //##ModelId=3E005CBB0301
    //##Documentation
    //## Returns the status record
    const DataRecord &status () const;
    //##ModelId=3E005CCD0018
    //##Documentation
    //## Returns the status record
    DataRecord &status ();

    //##ModelId=3DFF5AAE0003
    //##Documentation
    //## Returns object mutex by reference
    Thread::Mutex & mutex () const;

    //##ModelId=3E00AA51022A
    //##Documentation
    //## Declares a local debug context for SolverControl
    LocalDebugContext;
    
    //##ModelId=3E00C8540129
    string sdebug ( int detail = 1,const string &prefix = "",
                    const char *name = 0 ) const;
    
  protected:
    //##ModelId=3E00592800CD
    //##Documentation
    //## Constructor is protected; objects of this class may not be
    //## instantiated directly.
    SolverControlAgent (const HIID &initf)
      : AppControlAgent(initf) {}
  
    //##ModelId=3E4276F00147
    SolverControlAgent (AppEventSink &sink,const HIID &initf)
      : AppControlAgent(sink,initf) {}
  
    //##ModelId=3E0095CB0143
    //##Documentation
    //## Initializes for a new solution. Clears everything and posts
    //## a StartSolutionEvent. Meant to be called from a child
    //## class's startSolution()
    void initSolution (const DataRecord::Ref &params);
    

  private:
    //##ModelId=3E42770000EC
    SolverControlAgent();
    //##ModelId=3E4277B5029C
    SolverControlAgent(const SolverControlAgent& right);
    //##ModelId=3E427701009D
    SolverControlAgent& operator=(const SolverControlAgent& right);

    //##ModelId=3E005C7A016B
    //##Documentation
    //## Record containing current soution status
    DataRecord::Ref status_;
    //##ModelId=3E01FD0D01EC
    //##Documentation
    //## Current domain number
    int domainNum_;
    //##ModelId=3DFF365B0304
    //##Documentation
    //## Current iteration number
    int iterationNum_;
    //##ModelId=3DFF2D790018
    //##Documentation
    //## Current convergence value
    double convergence_;
    //##ModelId=3DFF5A88032E
    //##Documentation
    //## Mutex for thread-safety
    mutable Thread::Mutex mutex_;
};  

//##ModelId=3DFF37F3018B
inline double SolverControlAgent::convergence() const
{
    return convergence_;
}


//##ModelId=3DFF37F30341
inline int SolverControlAgent::iterationNum() const
{
    return iterationNum_;
}

//##ModelId=3E01FD0E011A
inline int SolverControlAgent::domainNum() const
{
    return domainNum_;
}

//##ModelId=3DFF5AAE0003
inline Thread::Mutex & SolverControlAgent::mutex() const
{
    return mutex_;
}

//##ModelId=3E005CBB0301
inline const DataRecord & SolverControlAgent::status () const
{
  return status_.deref();
}

//##ModelId=3E005CCD0018
inline DataRecord & SolverControlAgent::status ()
{
  return status_.dewr();
}






} // namespace SolverControl


#endif /* SOLVERCONTROL_SRC_SOLVERCONTROLAGENT_H_HEADER_INCLUDED_E43EA503 */
