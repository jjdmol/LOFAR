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
#pragma aid Next Step Domain Data Num All Params Solved Index

namespace AppState
{
  // define additional control states for the solver
  //##ModelId=3E00AA5100D5
  typedef enum
  {
    NEXT_SOLUTION  = 0x101,   // should go on to next solution
    NEXT_DOMAIN    = 0x102,   // should go on to next domain
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
    //    Normally posted when the application indicates end-of-data
    EndDataEvent        = AidEnd|AidData,
    
    //    Posted when a solution is interrupted 
    StopSolutionEvent   = AidStop|AidSolution,
    
    // Constants for field names
    //    Convergence parameter (in status record)
    FConvergence        = AidConvergence,
    //    Iteration number (in status record)
    FIterationNumber    = AidIteration|AidNum,
    //    Iteration number (in status record)
    FDomainNumber       = AidDomain|AidIndex,
    //    Ending message (in status record)
    FMessage            = AidMessage,
    //    End of data condition (in status record)
    FEndData            = AidEnd|AidData,
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
//## 
//## The solver control agent generally hides the getCommand() interface
//## from the user. Instead, it transpartently processes commands, and changes
//## its state accordingly. Most functions below will return the current 
//## state.
//## 
//## The three terminal states are inherited from AppControlAgent, and are:
//##
//##   INIT     (=0)    REINIT event received, application must call
//##                    getInitRecord() to obtain the new init record.
//##   STOPPED  (<0)    drop everything, close agents, and wait for a
//##                    REINIT event. App may call getInitRecord() to block
//##                    until this event.
//##   HALTED   (<0)    drop everything, close agents, and exit.
//## 
//## Most functions will check for a state-changing command and return a
//## terminal state if the appropriate command is received.
//## Note that all terminal states are <=0, and all non-terminal states are >0.
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
    //## (The data argument is meant to identify the domain.)
    //## Clear all parameters, etc.
    //## Base version simply posts a StartDomainEvent with the specified
    //## data record.
    //## Returns the current state, which is one of:
    //##    NEXT_SOLUTION  (>0):  go on, solutions required, call startSolution()
    //##    terminal state (<=0): see class documentation above.
    virtual int startDomain   (const DataRecord::Ref &data = DataRecord::Ref());
  
    //##ModelId=3E01F9A203A4
    //##Documentation
    //## Called by application when ready to start a solution within a domain
    //## Should clear all solution parameters and reset iteration count to 0
    //## (by calling initSolution(), below.)
    //## If another solution has been requested, stores a record of the solve
    //## parameters into 'params', and returns state=RUNNING.
    //## Returns the current state:
    //##    RUNNING        (>0):  go on, solve for 'params'
    //##    NEXT_DOMAIN    (>0):  go to next solve domain
    //##    terminal state (<=0): see class documentation above.
    virtual int startSolution (DataRecord::Ref &params);
    
    //##ModelId=3E01F9A302C5
    //##Documentation
    //## Called by application to indicate that it has finished with a particular
    //## domain.
    //## Base version simply posts a EndDomainEvent with the specified
    //## data record. The record is meant to identify the domain.
    //## Returns the current state:
    //##    NEXT_DOMAIN    (>0):  go to next solve domain
    //##    terminal state (<=0): see class documentation above.
    virtual int endDomain (const DataRecord::Ref &data = DataRecord::Ref());
    
    //##ModelId=3E4BA06802E9
    //##Documentation
    //## Called by application to indicate that it has run out of input data.
    //## Base version posts the specified event and raises the endOfData_ flag; 
    //## this flag would normally cause the control agent to transit to the 
    //## STOPPED state (instead of NEXT_DOMAIN) when all remaining solutions 
    //## are completed.
    //## Returns current state (which could be terminal if a state-changing 
    //## event is received).
    virtual int endData (const HIID &event = EndDataEvent);
    
    //##ModelId=3E005C9C0382
    //##Documentation
    //## Called by the solver application at end of every iteration.
    //## This updates the convergence parameter, increments the iteration
    //## count, stuffs them into the status record, and posts an
    //## EndIterationEvent containing the status record.
    //## (NB: the application may also fill the status record with any other
    //## relevant information prior to calling endIteration())
    //## 
    //## Returns the current state, which is one of:
    //##    RUNNING        (>0):  go on for another iteration
    //##    NEXT_SOLUTION  (>0):  end of this soltuion, another is required,
    //##                          call startSolution() now.
    //##    NEXT_DOMAIN    (>0):  end of this solutionm, go to next domain.
    //##    terminal state (<=0): see class documentation above.
    //##
    //## Base version simply returns the current state.
    virtual int endIteration (double conv);

    //##ModelId=3DFF2D6400EA
    //##Documentation
    //## Clears flags and solution parameters
    virtual void close ();
    
    //##ModelId=3DFF5B240397
    //##Documentation
    //## Called to interrupt a solution. Sets the 'newstate' state and generates
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
    
    //##ModelId=3E4BCA3300AA
    //##Documentation
    //## Returns last command received by the control agent. If no command
    //## has been received yet, return False. If flush=True (default), clears
    //## the cached command (so that the next call will return False, unless
    //## another command is received)
    bool getLastCommand (HIID &id, DataRecord::Ref &data, bool flush = True);

    //##ModelId=3E4BCA7003CA
    //##Documentation
    //## Called after the state() changes to INIT (i.e., reinitialized via
    //## external command) to get the init record delivered via that command.
    //## If state is not INIT, will set the state to STOPPED, and block until 
    //## a reinit or halt command is received.
    //## Returns: INIT or HALTED.
    int getInitRecord (DataRecord::Ref &initrec);
    
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
    
    //##ModelId=3E4BA15D0208
    //## Returns value of the end-of-data flag
    bool endOfData() const;
        
    //##ModelId=3E005CCD0018
    //##Documentation
    //## Returns the status record
    const DataRecord & status () const;
    //##ModelId=3E4BD27C01CB
    //##Documentation
    //## Returns the status record
    DataRecord & status ();

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
    
    //##ModelId=3E4BB7C203B7
    //##Documentation
    //## Calls getCommand() to check for change of state (i.e. 
    //## reinit, stop, halt, pause/resume, etc.)
    int checkState (int wait = AppEvent::NOWAIT);

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
    //##ModelId=3E4BA14102A7
    //##Documentation
    //## Flag: end of input data has been reached
    bool endOfData_;
    
    //##ModelId=3E4BC9A400C2
    HIID last_command_id_;

    //##ModelId=3E4BC9AF01BA
    DataRecord::Ref last_command_data_;

    //##ModelId=3E4BC9BA0249
    DataRecord::Ref initrec_;

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

//##ModelId=3E005CCD0018
inline const DataRecord & SolverControlAgent::status () const
{
  return status_.deref();
}

//##ModelId=3E4BD27C01CB
inline DataRecord & SolverControlAgent::status ()
{
  return status_.dewr();
}

//##ModelId=3E4BA15D0208
inline bool SolverControlAgent::endOfData() const
{
  return endOfData_;
}

} // namespace SolverControl


#endif /* SOLVERCONTROL_SRC_SOLVERCONTROLAGENT_H_HEADER_INCLUDED_E43EA503 */
