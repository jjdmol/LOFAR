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
  
static int __dum = aidRegistry_SolverControl();

InitDebugContext(SolverControlAgent,"SolverControl");


//##ModelId=3E01FD1D03DB
bool SolverControlAgent::init (const DataRecord &data)
{
  if( !AppControlAgent::init(data) )
    return False;
  stop_on_end_ = data[initfield()][FStopWhenEnd].as_bool(False);
  status()[FEndData] = endOfData_ = False;
  domainNum_ = -1;
  setState(NEXT_DOMAIN);
  return True;
}


//##ModelId=3DFF2D300027
int SolverControlAgent::startDomain  (const DataRecord::Ref::Xfer &data)
{
  // in terminal state? Return immediately
  if( checkState() <= 0 )
    return state(); 
  FailWhen(state()!=IDLE && state()!=NEXT_DOMAIN,"unexpected state (IDLE or NEXT_DOMAIN wanted)");
  status()[FEndData] = endOfData_ = False;
  status()[FDomainNumber] = domainNum_++;
  nextDomain_ = False;
  dprintf(1)("starting solution for domain %d\n",domainNum_);
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
  status()[FEndData] = endOfData_ = True;
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
  status()[FQueueSize] = solve_queue_.size(); 
  initSolution(params);
  dprintf(2)("startSolution: %s\n",params->sdebug(2).c_str());
  // get convergence criterion, if any
  conv_threshold_ = (*params)[FConvergence].as_double(-1);
  if( conv_threshold_ >= 0 )
  {
    dprintf(2)("convergence threshold is %f\n",conv_threshold_);
    if( (*params)[FWhenConverged].exists() )
      endrec_converged.attach( (*params)[FWhenConverged].as_DataRecord() );
    else
      endrec_converged <<= new DataRecord;
  }
  else
    endrec_converged.detach();
  // get max iter criterion
  max_iterations_ = (*params)[FMaxIterations].as_int(-1);
  if( max_iterations_ >= 0 )
  {
    dprintf(2)("max iterations: %d\n",max_iterations_);
    if( (*params)[FWhenMaxIter].exists() )
      endrec_maxiter.attach( (*params)[FWhenMaxIter].as_DataRecord() );
    else
      endrec_maxiter <<= new DataRecord;
  }
  else
    endrec_maxiter.detach();
  return setState(RUNNING);
}

//##ModelId=3E005C9C0382
int SolverControlAgent::endIteration (double conv)
{
  // terminal state -- return immediately
  FailWhen(state()>0 && state()!=RUNNING && state()!=ENDSOLVE,"unexpected state (RUNNING or ENDSOLVE wanted)");
  dprintf(3)("end iteration %d, conv=%f\n",iterationNum_,convergence_);
  // update status
  status()[FIterationNumber] = ++iterationNum_;
  status()[FConvergence] = convergence_ = conv;
  // post end-of-iteration event
  DataRecord::Ref ref(new DataRecord,DMI::ANONWR);
  ref()[FIterationNumber] = iterationNum_;
  ref()[FConvergence] = convergence_;
  postEvent(EndIterationEvent);
  // check if state has changed
  if( checkState() == RUNNING )
  {
    // if still running, check for end-of-solution criteria
    if( conv_threshold_ >= 0 && conv <= conv_threshold_ )
      stopSolution("Converged",endrec_converged.copy(),ConvergedEvent);
    else if( max_iterations_ >= 0 && iterationNum_ >= max_iterations_ )
      stopSolution("Max iteration count reached",endrec_maxiter.copy(),MaxIterEvent);
  }
  return state();
}

//##ModelId=3E00650B036F
int SolverControlAgent::endSolution  (DataRecord::Ref &endrec)
{
  FailWhen(state()>0 && state()!=ENDSOLVE,"unexpected state (ENDSOLVE wanted)");
  dprintf(2)("endSolution\n");
  endrec = endrec_;
  // do we have a next domain command in the end record? Raise the flag
  if( endrec.valid() )
    nextDomain_ |= (*endrec)[FNextDomain].as_bool(False); 
  // post end event
  postEvent(EndSolutionEvent);
  return setEndSolutionState();
}

//##ModelId=3E5647EB0294
void SolverControlAgent::addSolution (const DataRecord::Ref &params)
{
  Thread::Mutex::Lock lock(mutex()); 
  solve_queue_.push_back(params.copy(DMI::PRESERVE_RW));
  status()[FQueueSize] = solve_queue_.size(); 
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
    if( endOfData() && solve_queue_.empty() )
    {
      dprintf(2)("no more data or solutions for now\n");
      sink().clearEventFlag(); // no more events for now
      if( stop_on_end_ )
      {
        dprintf(1)("end of data, no more solutions, stopping\n");
        setState(STOPPED);
      }
    }
    else
    {
      dprintf(2)("more data or solutions coming up\n");
      sink().raiseEventFlag(); // more solutions to come
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
  dprintf(1)("stopping solution: %s",msg.c_str());
  if( event.length() )
    postEvent(event,msg);
  endrec_ = endrec;
  setState(ENDSOLVE);
}
    
//##ModelId=3DFF2D6400EA
void SolverControlAgent::close ()
{
  dprintf(1)("closing\n");
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
  iterationNum_ = 0;
  setState(RUNNING);
  status()[FSolutionParams] <<= params.copy(DMI::READONLY);
  status()[FDomainNumber] = domainNum();
  postEvent(StartSolutionEvent);
  sink().clearEventFlag(); // no events until solution converges
}

//##ModelId=3E56097E031F
int SolverControlAgent::checkState (int wait)
{
  int res = getCommand(last_command_id_,last_command_data_,wait);
  // got a state-change command?
  if( res == AppEvent::NEWSTATE )
  {
    // are we being reinitialized?
    if( state() == INIT )
      initrec_.copy(last_command_data_,DMI::PRESERVE_RW);
  }
  // else check for solver-specific commands
  else if( res == AppEvent::SUCCESS )
  {
    // catch all errors during processing of commands
    try
    {
      if( last_command_id_ == NextDomainCommand )
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
      else if( last_command_id_ == EndSolutionCommand )
      {
        // if solution is ended w/o a data record, init an empty one
        if( !last_command_data_.valid() )
          last_command_data_ <<= new DataRecord;
        stopSolution("stop command received",last_command_data_.copy(DMI::PRESERVE_RW),StopSolutionEvent);
      }
      // add solution to queue
      else if( last_command_id_ == AddSolutionCommand )
      {
        FailWhen(!last_command_data_.valid(),
                  AddSolutionCommand.toString()+" must contain a DataRecord");
        addSolution(last_command_data_);
      }
    }
    catch( std::exception &exc )
    {
      DataRecord::Ref ref(new DataRecord,DMI::ANONWR);
      ref()[AidText] = "Error processing command " +
                        last_command_id_.toString() + ": " + exc.what();
      ref()[AidError] = exc.what();
      ref()[AidCommand] = last_command_id_;
      if( last_command_data_.valid() )
        ref()[AidData] <<= last_command_data_;
      postEvent(CommandErrorEvent,ref);
    }
  }
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

//##ModelId=3E560979013D
int SolverControlAgent::getInitRecord (DataRecord::Ref &initrec)
{
  // halted? return immediately
  if( state() == HALTED )
    return HALTED;
  // else wait for init or halt
  if( state() != INIT )
  {
    setState(STOPPED);
    // sit there checking (and ignoring) commands until an init/halt
    while( state() != INIT && state() != HALTED )
      checkState(AppEvent::BLOCK);
  }
  // return init record
  if( state() == INIT )
    initrec = initrec_;
  
  return state();
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



