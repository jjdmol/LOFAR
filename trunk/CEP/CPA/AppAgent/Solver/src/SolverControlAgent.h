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
#include <Solver/AID-Solver.h>
#include <Common/Thread/Mutex.h>
class AppEventSink;

#pragma aidgroup Solver
#pragma aid Start End Stop Iteration Solution Solver Control Message Convergence 
#pragma aid Next Step Domain Data Num All Params Solved Index Peel Accept
#pragma aid When Max Converged Iter Command Add Queue Size Single Step Defer Fail


namespace AppState
{
  // define additional control states for the solver
  #pragma aid IDLE ENDSOLVE NEXT_DOMAIN
  //##ModelId=3E8C1A5B009F
  typedef enum
  {
    IDLE        = -AidIDLE_int,        // waiting for start of solution
    ENDSOLVE    = -AidENDSOLVE_int,    // waiting for end of solution
    NEXT_DOMAIN = -AidNEXT_DOMAIN_int  // waiting for startDomain
  }
  ExtraSolverControlStates;
};

    
namespace SolverControl 
{

using namespace AppControlAgentVocabulary;

const HIID 
//    FSolverControlParams = AidSolver|FControlParams,
    
    FStopWhenEnd         = AidStop|AidWhen|AidEnd, 
    
    // Constants for event names
    //    Posted at start of every solve domain
    StartDomainEvent    = AidStart|AidDomain,
    //    Posted at start of every solution
    StartSolutionEvent  = AidStart|AidSolution,
    //    Posted at end of every iteration
    EndIterationEvent   = AidEnd|AidIteration,
    //    Posted when a solution is ended 
    EndSolutionEvent    = AidEnd|AidSolution,
    //    Posted when a solution is ended 
    FailSolutionEvent   = AidFail|AidSolution,
    //    Posted when a domain has been solved for
    EndDomainEvent      = AidEnd|AidDomain,
    //    Posted when a next-domain transition has been deferred
    NextDomainDeferEvent= AidNext|AidDomain|AidDefer,
    //    Posted on close(), when the solver is finished
    SolverEndEvent      = AidSolver|AidEnd,
    //    Normally posted when the application indicates end-of-data
    EndDataEvent        = AidEnd|AidData,
    //    Posted when the solver auto-pauses itself
    SolverPausedEvent   = AidSolver|AidPaused,
    //    Posted when the solver exits with an error
    SolverErrorEvent    = AidSolver|AidError,
    
    //    Generic catch-all message event
    SolverNotifyEvent   = AidSolver|AidNotify,
    
    //    Posted when a solution is terminated with an external event
    StopSolutionEvent   = AidStop|AidSolution,
    //    Posted when the solution is stopped due to convergence
    ConvergedEvent      = AidConverged,
    //    Posted when the solution is hit sthe max iteration count 
    MaxIterEvent        = AidMax|AidIter,
    //    Posted when an error occurs while processing a command
    CommandErrorEvent   = AidCommand|AidError,

    // standard prefix for solver control events
    SolverControlPrefix = ControlPrefix|AidSolver,
        
    //    External command to go to next domain
    NextDomainCommand   = SolverControlPrefix|AidNext|AidDomain,
    //    External command to end solution
    EndSolutionCommand  = SolverControlPrefix|AidEnd|AidSolution,
    //      Field names within EndSolutionCommand's data record
        FPause          = AidPause,       // bool: pause at end
    
        FAcceptSolution = AidAccept,            // bool: accept solution? (bool)
        FPeel           = AidPeel,              // record?: peel sources (optional, only if accepted)
        FNextDomain     = AidNext|AidDomain,    // bool: go to next domain (can be set instead of giving a NextDomainCommand)
        FNextSolution   = AidNext|AidSolution,  // record defining next solution (optional)
        
    //    Command to setup the next solution
    AddSolutionCommand = SolverControlPrefix|AidAdd|AidSolution,
        // will contain data record specifying the next solution
        
    // Solution parameter record fields
    //    Max number of iterations
    FMaxIterations      = AidMax|AidIter,
    FMaxIter            = FMaxIterations,
    //    Desired convergence
    FConvergence        = AidConvergence,
    //    Enable iteration stepping mode (i.e. will pause after this many
    //    iterations, 0 for disable)
    FIterStep           = AidIter|AidStep,
    //    ending record to generate when max iter count exceeded
    //    (if not specified, will pause and wait for an end command)
    FWhenMaxIter        = AidWhen|AidMax|AidIter,
    //    ending record to generate when convergence reached
    //    (if not specified, will pause and wait for an end command)
    FWhenConverged      = AidWhen|AidConverged,

    // current iteration number
    FIterationNumber    = AidIteration|AidNum,
    
    // Field names for status record
    //    Solution parameters subrecord 
    StSolutionParams     = AidSolution|AidParams,
    // solution 
    StSolution           = AidSolution,
    // solver control status sub-record
    StSolverControl      = AidSolver|AidControl,
    //    Domain number
    StDomain             = AidDomain,  
    StDomainNumber       = AidDomain|AidIndex,
    //    Ending message
    StMessage            = AidMessage,
    //    End of data condition
    StEndData            = AidEnd|AidData,
    //    # of jobs left in solution queue
    StQueueSize          = AidQueue|AidSize;
    
    
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
//## its state accordingly. Most functions below will check for and process
//## state-changing events, and always return the current state.
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
//## Note that some functions have specific entry states as a pre-condition, as
//## documented below. When called in a terminal state, they will return
//## that state immediately, without doing anything. When called in a 
//## non-terminal state that does not satisfy the preconditions, these 
//## functions will throw an exception (since this is usually a logic error).
//## 
//## Note also that all terminal states are <=0, and all non-terminal states 
//## are >0.
class SolverControlAgent : public AppControlAgent
{
  public:
    //##ModelId=3E56097B02EB
    //##Documentation
    SolverControlAgent (const HIID &initf = AidControl)
      : AppControlAgent(initf) {}
    //##ModelId=3E56097C00F3
    SolverControlAgent (AppEventSink &sink,const HIID &initf = AidControl)
      : AppControlAgent(sink,initf) {}
    //##ModelId=3E56097D003C
    SolverControlAgent(AppEventSink *sink, int dmiflags, const HIID &initf = AidControl)
      : AppControlAgent(sink,dmiflags,initf) {}
    
    //##ModelId=3E8C1A5D0385
    int start (DataRecord::Ref &initrec);
    
    //##ModelId=3DFF2D300027
    //##Documentation
    //## Called by application to start solving for a new domain.
    //## (The data argument is meant to identify the domain.)
    //## Clear all parameters, etc.
    //## Base version simply posts a StartDomainEvent with the specified
    //## data record.
    //## Entry state must be IDLE (else if terminal, function will return 
    //## immediately, or throw an exception in other states).
    //## Returns the current state, which will be one of:
    //##    IDLE (>0): proceed
    //##    a terminal state (<=0): see class documentation above.
    virtual int startDomain   (const DataRecord::Ref::Xfer &data = DataRecord::Ref());
  
    //##ModelId=3E01F9A203A4
    //##Documentation
    //## Called by application when ready to start a solution within a domain
    //## Should clear all solution parameters and reset iteration count to 0
    //## (by calling initSolution(), below.)
    //## If no solve jobs have been set up, blocks until an appropriate command
    //## is received.
    //## Entry state must be IDLE (else if terminal, function will return 
    //## immediately, or throw an exception in other states).
    //## Returns the current state, which will be one of:
    //##    RUNNING (>0):     start solving
    //##    NEXT_DOMAIN (>0): no more solutions this domain, start another one
    //##    a terminal state (<=0): see class documentation above.
    virtual int startSolution (DataRecord::Ref &params);
    
    //##ModelId=3E01F9A302C5
    //##Documentation
    //## Called by application to indicate that it has finished with a particular
    //## domain.
    //## Base version simply posts a EndDomainEvent with the specified
    //## data record. The record is meant to identify the domain.
    //## Entry state: any
    //## Updates and returns current state (i.e. updates if an external
    //## state-changing  event has been received).
    virtual int endDomain (const DataRecord::Ref::Xfer &data = DataRecord::Ref());
    
    //##ModelId=3E4BA06802E9
    //##Documentation
    //## Called by application to indicate that it has run out of input data.
    //## Base version posts the specified event and raises the endOfData_ flag; 
    //## this flag would normally cause the control agent to transit to the 
    //## STOPPED state (instead of IDLE) when all remaining solutions 
    //## are completed.
    //## Entry state: any
    //## Updates and returns current state (i.e. updates if an external
    //## state-changing  event has been received).
    virtual int endData (const HIID &event = EndDataEvent);
    
    //##ModelId=3E005C9C0382
    //##Documentation
    //## Called by the solver application at end of every iteration.
    //## This updates the convergence parameter, increments the iteration
    //## count, stuffs them into the status record, and posts an
    //## EndIterationEvent containing the status record.
    //## (NB: the application may also fill the status record with any other
    //## relevant information prior to calling endIteration())
    //## Entry state must be RUNNING or ENDSOLVE (else if terminal, function 
    //## will return immediately, or throw an exception in other states).
    //## Returns the current state, which will be one of:
    //##    RUNNING        (>0):  go on for another iteration
    //##    ENDSOLVE       (>0):  end of this solution, please call 
    //##                          endSolution() to pick up end record.
    //##    terminal state (<=0): see class documentation above.
    virtual int endIteration (const DataRecord::Ref::Copy &data);
    
    //##ModelId=3E00650B036F
    //##Documentation
    //## Called by the solver application to acknowledge end of solution.
    //## Posts an end-of-solution event.
    //## An end record will be attached to endrec upon return.
    //## Entry state must be ENDSOLVE (else if terminal, function will return 
    //## immediately, or throw an exception in other states).
    //## Returns the current state, which will be one of:
    //##    IDLE (>0): proceed
    //##    terminal state (<=0): see class documentation above.
    virtual int endSolution  (const DataRecord::Ref::Copy &data,DataRecord::Ref &endrec);
    
    
    // called if a solution has failed
    virtual int failSolution (const string &msg = "solution failed" );
    
    //##ModelId=3DFF2D6400EA
    //##Documentation
    //## Clears flags and solution parameters
    virtual void close ();
    
    //##ModelId=3E56097E031F
    //##Documentation
    //## Calls getCommand() to check for change of state (i.e. 
    //## reinit, stop, halt, pause/resume, etc.)
    int checkState (int wait = AppEvent::NOWAIT);

    //##ModelId=3E5647EB0294
    void addSolution (const DataRecord::Ref &params);

    //##ModelId=3E5B879E026D
    //##Documentation
    //## Called to prepare a solution for termination. Will set state to IDLE,
    //## post the specified event, and return the specified end record and
    //## solution command next time endIteration() is called.
    void stopSolution (const string &msg = "",
                            const DataRecord::Ref::Xfer &endrec = DataRecord::Ref(),
                            const HIID &event = StopSolutionEvent);
    
    //##ModelId=3E5609780194
    //##Documentation
    //## Returns last command received by the control agent. If no command
    //## has been received yet, return False. If flush=True (default), clears
    //## the cached command (so that the next call will return False, unless
    //## another command is received)
    bool getLastCommand (HIID &id, DataRecord::Ref &data, bool flush = True);

    //##ModelId=3E56097902C1
    //##Documentation
    //## Returns current convergence parameter
    double convergence () const;

    //##ModelId=3E560979034A
    //##Documentation
    //## Returns value of iteration counter
    int iterationNum () const;
    
    //##ModelId=3E56097903D4
    //##Documentation
    //## Returns value of domain counter
    int domainNum () const;
    
    //##ModelId=3E56097A008C
    //## Returns value of the end-of-data flag
    bool endOfData() const;
        

    //##ModelId=3E56097A0234
    //##Documentation
    //## Returns object mutex by reference
    Thread::Mutex & mutex () const;

    //##ModelId=3E560975038D
    //##Documentation
    //## Declares a local debug context for SolverControl
    LocalDebugContext;
    
    //##ModelId=3E56097A02C5
    string sdebug ( int detail = 1,const string &prefix = "",
                    const char *name = 0 ) const;
    

  protected:
    //##ModelId=3E01FD1D03DB
    //##Documentation
    //## inits various counters
    virtual bool init (const DataRecord &data);
  
    //##ModelId=3EB242F400B5
    virtual int processCommand (const HIID &id,const DataRecord::Ref &data,
                                const HIID &source);

    //##ModelId=3EB242F402AB
    int pauseSolution (const string &msg);
  
    //##ModelId=3E56097E00FD
    //##Documentation
    //## Initializes for a new solution. Clears everything and posts
    //## a StartSolutionEvent. Meant to be called from a child
    //## class's startSolution()
    void initSolution (const DataRecord::Ref &params);

    //##ModelId=3EB242F40376
    //##Documentation
    //## Processes parameters from the control record
    void processControlRecord (const DataRecord &rec);
  
    //##ModelId=3E70A2C703BA
    std::deque<DataRecord::Ref> & solveQueue ();
    
  private:
    //##ModelId=3E56097F03DD
    SolverControlAgent(const SolverControlAgent& right);
    //##ModelId=3E560980020D
    SolverControlAgent& operator=(const SolverControlAgent& right);

    //##ModelId=3E5F675C02FC
    int setEndSolutionState ();
    int setNextDomainState ();
    
    //##ModelId=3E56568200E6
    std::deque<DataRecord::Ref> solve_queue_;
    
    //##ModelId=3E56097602C8
    //##Documentation
    //## Current domain number
    int domainNum_;
    //##ModelId=3E56097603C9
    //##Documentation
    //## Current iteration number
    int iter_count_;
    //##ModelId=3EB242F30312
    int iter_step_;
    //##ModelId=3EB242F4001F
    int pause_at_iter_;
    //##ModelId=3E56097700E7
    //##Documentation
    //## Current convergence value
    double convergence_;
    //##ModelId=3E56097701D8
    //##Documentation
    //## Flag: end of input data has been reached
    bool endOfData_;
    
    //##ModelId=3E63610403B3
    //##Documentation
    //## Flag: next domain command has been received but not yet processed
    //## (this command only takes immediate effect only in IDLE state, in 
    //## all other states, the flag is raised for later processing)
    bool nextDomain_;
    
    //##ModelId=3E5CFA4E027B
    //##Documentation
    //## automaticallly go into STOPPED state when end of data and
    //## end of solution queue has been reached?
    bool stop_on_end_;
    
    //##ModelId=3E56097702E3
    //##Documentation
    //## ID of most recently received command
    HIID last_command_id_;

    //##ModelId=3E5609770337
    //##Documentation
    //## Data of most recently received command
    DataRecord::Ref last_command_data_;

    //##ModelId=3E5BA4CC017C
    //##Documentation
    //## When a solution has ended, this holds the end-record, until it is
    //## picked up by endSolution
    DataRecord::Ref endrec_;

    //##ModelId=3E5F641E01D6
    //##Documentation
    //## Stop criterion: max # of iterations
    int max_iterations_;
    //##ModelId=3E5F641F01CB
    //##Documentation
    //## Stop criterion: min convergence
    double conv_threshold_;
    //##ModelId=3E5F641F0327
    //##Documentation
    //## end-record to use when solution is stopped due to convergence
    DataRecord::Ref endrec_converged;
    //##ModelId=3E5F64200078
    //##Documentation
    //## end-record to use when solution is stopped due to max iter count reached
    DataRecord::Ref endrec_maxiter;
    
    //##ModelId=3E560978016B
    //##Documentation
    //## Mutex for thread-safety
    mutable Thread::Mutex mutex_;
    

};  

//##ModelId=3E56097902C1
inline double SolverControlAgent::convergence() const
{
    return convergence_;
}


//##ModelId=3E560979034A
inline int SolverControlAgent::iterationNum() const
{
    return iter_count_;
}

//##ModelId=3E56097903D4
inline int SolverControlAgent::domainNum() const
{
    return domainNum_;
}

//##ModelId=3E56097A0234
inline Thread::Mutex & SolverControlAgent::mutex() const
{
    return mutex_;
}

//##ModelId=3E56097A008C
inline bool SolverControlAgent::endOfData() const
{
  return endOfData_;
}

//##ModelId=3E70A2C703BA
inline std::deque<DataRecord::Ref> & SolverControlAgent::solveQueue ()
{
  return solve_queue_;
}

} // namespace SolverControl


#endif /* SOLVERCONTROL_SRC_SOLVERCONTROLAGENT_H_HEADER_INCLUDED_E43EA503 */
