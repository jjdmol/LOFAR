//  SolverControlAgent.cc: abstract base for solver control agents
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

#include "SolverControlAgent.h"

namespace SolverControl
{
  
using namespace AppState;
  
static int __dum = aidRegistry_Solver();

InitDebugContext(SolverControlAgent,"SolverControl");


//##ModelId=3E01FD1D03DB
bool SolverControlAgent::init (const DataRecord &data)
{
  if( !AppControlAgent::init(data) )
    return False;
  if( !data[initfield()].exists() )
    return False;
  stop_on_end_ = data[initfield()][FStopWhenEnd].as<bool>(False);
  setStatus(StEndData,endOfData_ = False);
  DataRecord::Ref empty(DMI::ANONWR);
  setStatus(StSolution,DataRecord::Ref(DMI::ANONWR));
  domainNum_ = -1;
  return True;
}

//##ModelId=3E8C1A5D0385
int SolverControlAgent::start (DataRecord::Ref &initrec)
{
  // instead of running state, go into NEXT_DOMAIN state.
  if( AppControlAgent::start(initrec) == RUNNING )
    setState(NEXT_DOMAIN);
  return state();
}


//##ModelId=3DFF2D300027
int SolverControlAgent::startDomain  (const DataRecord::Ref::Xfer &data)
{
  // in terminal state? Return immediately
  if( checkState() <= 0 )
    return state(); 
  FailWhen(state()!=IDLE && state()!=NEXT_DOMAIN,"unexpected state (IDLE or NEXT_DOMAIN wanted)");
  setState(IDLE);
  setStatus(StEndData,endOfData_ = False);
  setStatus(StDomainNumber,++domainNum_);
  nextDomain_ = False;
  dprintf(1)("starting domain %d\n",domainNum_);
  postEvent(StartDomainEvent,data);
  return state();
}

//##ModelId=3E01F9A302C5
int SolverControlAgent::endDomain (const DataRecord::Ref::Xfer &data)
{
  FailWhen(domainNum()<0,"illegal domain number -- perhaps startDomain() not called?");
  dprintf(1)("end of domain %d\n",domainNum_);
  postEvent(EndDomainEvent,data);
  return checkState();
}

//##ModelId=3E4BA06802E9
int SolverControlAgent::endData (const HIID &event)
{
  dprintf(1)("end of data\n");
  postEvent(event);
  setStatus(StEndData,endOfData_ = True);
  return checkState();
}

//##ModelId=3E01F9A203A4
int SolverControlAgent::startSolution (DataRecord::Ref &params)
{
  // in terminal state? Return immediately
  int st = checkState();
  if( st <= 0 || st == NEXT_DOMAIN )
    return state();
  FailWhen(state()!=IDLE,"unexpected state (IDLE expected)");
  // block until something is the solve queue
  Thread::Mutex::Lock lock(mutex()); 
  while( solve_queue_.empty() )
  {
    lock.release();
    int st = checkState(AppEvent::WAIT);
    if( st <= 0 || st == NEXT_DOMAIN )
      return st;
    lock.relock(mutex());
  }
  params = solve_queue_.front();
  solve_queue_.pop_front();
  setStatus(StQueueSize,int(solve_queue_.size())); 
  DataRecord::Ref solrec(DMI::ANONWR);
  solrec()[FIterationNumber] = 0;
  setStatus(StSolution,solrec);
  initSolution(params);
  dprintf(2)("startSolution: %s\n",params->sdebug(2).c_str());
  // set default solution controls...
  endrec_converged.detach();
  endrec_maxiter.detach();
  conv_threshold_ = -1;
  max_iterations_ = -1;
  iter_step_ = 0;
  pause_at_iter_ = -1;
  // ...and override them with anything present in the record
  processControlRecord(*params);
  
  return setState(RUNNING);
}

//##ModelId=3E005C9C0382
int SolverControlAgent::endIteration (double conv)
{
  // terminal state -- return immediately
  FailWhen(state()>0 && state()!=RUNNING && state()!=ENDSOLVE,"unexpected state (RUNNING or ENDSOLVE wanted)");
  // update status
  setStatus(StSolution,FIterationNumber,++iter_count_);
  setStatus(StSolution,FConvergence,convergence_=conv);
  dprintf(3)("end iteration %d, conv=%f\n",iter_count_,convergence_);
  // post end-of-iteration event
  DataRecord::Ref ref(new DataRecord,DMI::ANONWR);
  ref()[FIterationNumber] = iter_count_;
  ref()[FConvergence] = convergence_;
  postEvent(EndIterationEvent,ref);
  // check if state has been changed for us
  if( checkState() != RUNNING )
    return state();
  // still running? check for end conditions
  // NB: gotta pause here if no end-record supplied
  if( conv_threshold_ >= 0 && conv <= conv_threshold_ )
  {
    if( endrec_converged.valid() )
      stopSolution("Converged",endrec_converged.copy(),ConvergedEvent);
    else
      return pauseSolution("Converged, waiting for end-record");
  }
  else if( max_iterations_ >= 0 && iter_count_ >= max_iterations_ )
  {
    if( endrec_maxiter.valid() )
      stopSolution("Max iteration count reached",endrec_maxiter.copy(),MaxIterEvent);
    else
      return pauseSolution("Max iteration count reached, waiting for end-record");
  }
  // check for iteration stepping mode
  if( iter_step_ > 0 && iter_count_ >= pause_at_iter_ )
  {
    pause_at_iter_ = iter_count_ + iter_step_;
    return pauseSolution(
            ssprintf("Iter-stepping mode: paused at iteration %d",iter_count_) );
  }
  return state();
}

int SolverControlAgent::pauseSolution (const string &msg)
{
  if( pause() == AppEvent::PAUSED ) // this ought to go into a separate function
    postEvent(SolverPausedEvent,msg);
  // go into checkState(), since that'll keep us paused
  return checkState();
} 

//##ModelId=3E00650B036F
int SolverControlAgent::endSolution  (DataRecord::Ref &endrec)
{
  FailWhen(state()>0 && state()!=ENDSOLVE,"unexpected state (ENDSOLVE wanted)");
  endrec = endrec_;
  // do we have a next domain command in the end record? Raise the flag
  if( endrec.valid() )
    nextDomain_ |= (*endrec)[FNextDomain].as<bool>(False); 
  dprintf(2)("endSolution, nextDomain=%d\n",int(nextDomain_));
  setStatus(StSolution,DataRecord::Ref(DMI::ANONWR));
  setStatus(StSolutionParams,DataRecord::Ref(DMI::ANONWR));
  // post end event
  postEvent(EndSolutionEvent);
  return setEndSolutionState();
}

//##ModelId=3E5647EB0294
void SolverControlAgent::addSolution (const DataRecord::Ref &params)
{
  Thread::Mutex::Lock lock(mutex()); 
  solve_queue_.push_back(params.copy(DMI::PRESERVE_RW));
  setStatus(StQueueSize,int(solve_queue_.size())); 
  dprintf(2)("added solution %d, raising event flag\n",solve_queue_.size());
  sink().raiseEventFlag(); // more solutions to come
}

//##ModelId=3E5F675C02FC
int SolverControlAgent::setEndSolutionState ()
{
  if( checkState() == ENDSOLVE )
  {
    Thread::Mutex::Lock lock(mutex());
    setState(nextDomain_?NEXT_DOMAIN:IDLE);
    nextDomain_ = False;
    if( solve_queue_.empty() )
    {
      dprintf(2)("no more solutions for now, clearing event flag\n");
      sink().clearEventFlag(); // no more events for now
      if( endOfData() )
      {
        if( stop_on_end_ )
        {
          dprintf(1)("end of data, stopping\n");
          setState(STOPPED);
        }
      }
    }
  }
  return state();
}

//##ModelId=3E5B879E026D
void SolverControlAgent::stopSolution (const string &msg,
                            const DataRecord::Ref::Xfer &endrec,
                            const HIID &event)
{
  Thread::Mutex::Lock lock(mutex());
  FailWhen( state() != RUNNING,"unexpected state (RUNNING wanted)" );
  dprintf(1)("stopping solution: %s\n",msg.c_str());
  if( event.length() )
    postEvent(event,msg);
  endrec_ = endrec;
  bool unpause = True;
  if( endrec_.valid() && (*endrec_)[FPause].as<bool>(False) )
    unpause = False;
  setState(ENDSOLVE,unpause);
}
    
//##ModelId=3DFF2D6400EA
void SolverControlAgent::close ()
{
  dprintf(1)("closing\n");
  setStatus(StSolution,DataRecord::Ref(DMI::ANONWR));
  Thread::Mutex::Lock lock(mutex());
  postEvent(SolverEndEvent);
  AppControlAgent::close();
  sink().clearEventFlag(); // no more events 
}

//##ModelId=3E56097E00FD
void SolverControlAgent::initSolution (const DataRecord::Ref &params)
{
  FailWhen(domainNum()<0,"illegal domain number -- perhaps startDomain() not called?");
  convergence_ = 1e+1000; // hmm, definitely unconverged
  iter_count_ = 0;
  setState(RUNNING);
  setStatus(StSolutionParams,params.copy().privatize(DMI::WRITE|DMI::DEEP));
  setStatus(StDomainNumber,domainNum());
  postEvent(StartSolutionEvent);
}

int SolverControlAgent::processCommand (const HIID &id,
                          const DataRecord::Ref &data,const HIID &source)
{
  if( id == NextDomainCommand )
  {
    // in idle state, go into NEXT_DOMAIN immediately
    if( state() == IDLE )
    {
      setState(NEXT_DOMAIN);
      nextDomain_ = False;
    }
    else if( state() != NEXT_DOMAIN ) // else raise flag for later
      nextDomain_ = True;
  }
  // end-of-solution command
  else if( id == EndSolutionCommand )
  {
    stopSolution("stop command received",
        // if solution is ended w/o a data record, init an empty one
        data.valid() ? data.copy(DMI::PRESERVE_RW) : DataRecord::Ref(DMI::ANONWR),
        StopSolutionEvent);
  }
  // add solution to queue
  else if( id == AddSolutionCommand )
  {
    FailWhen(!data.valid(),
              AddSolutionCommand.toString()+" must contain a DataRecord");
    addSolution(data);
  }
  else if( id == ResumeEvent )
  {
    if( data.valid() )
      processControlRecord(*data);
    return resume();
  }
  else
    return AppControlAgent::processCommand(id,data,source);
  
  return AppEvent::SUCCESS;    
}

void SolverControlAgent::processControlRecord (const DataRecord &rec)
{
  // enable iteration stepping mode, if specified
  if( rec[FIterStep].exists() )
  {
    iter_step_ = rec[FIterStep].as<int>();
    setStatus(StSolutionParams,FIterStep,iter_step_);
    if( iter_step_ > 0 )
      pause_at_iter_ = iter_count_ + iter_step_;
    else
      pause_at_iter_ = -1;
  }
  // get convergence criterion, if any
  if( rec[FConvergence].exists() )
  {
    endrec_converged.detach();
    conv_threshold_ = rec[FConvergence].as<double>();
    setStatus(StSolutionParams,FConvergence,conv_threshold_);
    if( conv_threshold_ >= 0 )
    {
      dprintf(2)("convergence threshold is %f\n",conv_threshold_);
      if( rec[FWhenConverged].exists() )
      {
        endrec_converged.attach( rec[FWhenConverged].as<DataRecord>() );
        setStatus(StSolutionParams,FWhenConverged,endrec_converged.copy());
      }
    }
  }
  // get max iter criterion
  if( rec[FMaxIterations].exists() )
  {
    endrec_maxiter.detach();
    max_iterations_ = rec[FMaxIterations].as<int>();
    setStatus(StSolutionParams,FMaxIterations,max_iterations_);
    if( max_iterations_ >= 0 )
    {
      dprintf(2)("max iterations: %d\n",max_iterations_);
      if( rec[FWhenMaxIter].exists() )
      {
        endrec_maxiter.attach( rec[FWhenMaxIter].as<DataRecord>() );
        setStatus(StSolutionParams,FWhenMaxIter,endrec_maxiter.copy());
      }
    }
  }
}

//##ModelId=3E56097E031F
int SolverControlAgent::checkState (int wait)
{
  int res = getCommand(last_command_id_,last_command_data_,wait);
  return state();
}

//##ModelId=3E5609780194
bool SolverControlAgent::getLastCommand (HIID& id, DataRecord::Ref &data, bool flush)
{
  if( last_command_id_.empty() )
    return False;
  id = last_command_id_;
  if( flush )
  {
    data.xfer(last_command_data_);
    last_command_id_ = HIID();
  }
  else
    data.copy(last_command_data_,DMI::PRESERVE_RW);
  return True;
}

//##ModelId=3E56097A02C5
string SolverControlAgent::sdebug ( int detail,const string &prefix,
                                    const char *name ) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;
  
  string out;
  if( detail >= 0 ) // basic detail
  {
    appendf(out,"%s/%08x",name?name:"SolverControlAgent",(int)this);
  }
  if( detail >= 1 || detail == -1 )
  {
    appendf(out,"dom:%d st:%d i#%d c:%f",domainNum(),state(),iterationNum(),convergence());
  }
  if( detail >= 3 || detail == -3 )
  {
    append(out,status().sdebug(abs(detail)-1));
  }
  return out;
}

}; // end namespace SolverControl



