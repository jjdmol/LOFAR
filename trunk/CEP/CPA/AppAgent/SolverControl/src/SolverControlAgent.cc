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
  status_ <<= new DataRecord;
  status_()[FEndData] = endOfData_ = False;
  domainNum_ = -1;
  setState(NEXT_DOMAIN);
  return True;
}


//##ModelId=3DFF2D300027
int SolverControlAgent::startDomain  (const DataRecord::Ref &data)
{
  // in terminal state? Return immediately
  if( checkState() <= 0 )
    return state(); 
  // else must be in NEXT_DOMAIN state
  FailWhen(state()!=NEXT_DOMAIN,"unexpected state, must be NEXT_DOMAIN");
  status_()[FEndData] = endOfData_ = False;
  domainNum_++;
  dprintf(1)("starting solution for domain %d\n",domainNum_);
  postEvent(StartDomainEvent,data);
  return setState(NEXT_SOLUTION);
}

//##ModelId=3E01F9A302C5
int SolverControlAgent::endDomain (const DataRecord::Ref &data)
{
  FailWhen(domainNum()<0,"illegal domain number -- perhaps startDomain() not called?");
  dprintf(1)("end of domain %d\n",domainNum_);
  postEvent(EndDomainEvent,data);
  return checkState();
}

//##ModelId=3E4BA06802E9
int SolverControlAgent::endData (const HIID &event)
{
  postEvent(event);
  status_()[FEndData] = endOfData_ = True;
  return checkState();
}

//##ModelId=3E01F9A203A4
int SolverControlAgent::startSolution (DataRecord::Ref &)
{
  // in terminal state? Return immediately
  if( checkState() > 0 )
  {
    // if in NEXT_DOMAIN state, return immediately
    if( state() == NEXT_DOMAIN )
      return state();
    // else must be in NEXT_SOLUTION state
    FailWhen(state()!=NEXT_SOLUTION,"unexpected state, must be NEXT_SOLUTION");
    // default version does not drive any solutions; asks for next domain immediately
    return setState(NEXT_DOMAIN);
  }
  return state();
}

//##ModelId=3E005C9C0382
int SolverControlAgent::endIteration (double conv)
{
  // in terminal state? Return immediately
  if( checkState() > 0 )
  {
    DataRecord & stat = status();
    stat[FIterationNumber] = ++iterationNum_;
    stat[FConvergence] = convergence_ = conv;
    dprintf(2)("end iteration %d, conv=%f\n",iterationNum_,convergence_);
    postEvent(EndIterationEvent,status_);
  }
  return state();
}

//##ModelId=3DFF2D6400EA
void SolverControlAgent::close ()
{
  dprintf(1)("closing\n");
  Thread::Mutex::Lock lock(mutex());
  status_.detach();
  postEvent(SolverEndEvent);
  AppControlAgent::close();
  sink().clearEventFlag(); // no more events 
}

//##ModelId=3DFF5B240397
void SolverControlAgent::stopSolution (const string &msg,int newstate,const HIID &event)
{
  Thread::Mutex::Lock lock(mutex());
  dprintf(1)("stopping solution with state=%d (%s)\n",newstate,msg.c_str());
  setState(newstate);
  sink().raiseEventFlag(); // we now have an event
  // place message into status and generate event
  if( msg.length() )
    status_()[FMessage] = msg;
  postEvent(event,status_);
}

//##ModelId=3E0095CB0143
void SolverControlAgent::initSolution (const DataRecord::Ref &params)
{
  FailWhen(state() != NEXT_SOLUTION,"unexpected state, must be NEXT_SOLUTION");
  FailWhen(domainNum()<0,"illegal domain number -- perhaps startDomain() not called?");
  convergence_ = 1e+1000; // hmm, definitely unconverged
  iterationNum_ = 0;
  setState(RUNNING);
  // init status record and put a copy of the solution parameters into it
  status_ <<= new DataRecord;
  status_()[FSolutionParams] <<= params.copy(DMI::READONLY);
  status_()[FDomainNumber] = domainNum();
  postEvent(StartSolutionEvent,status_);
  sink().clearEventFlag(); // no events until solution converges
}

//##ModelId=3E4BB7C203B7
int SolverControlAgent::checkState (int wait)
{
  int res = getCommand(last_command_id_,last_command_data_,wait);
  if( res == AppEvent::NEWSTATE && state() == INIT )
    initrec_.copy(last_command_data_,DMI::PRESERVE_RW);
  return state();
}

//##ModelId=3E4BCA3300AA
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

//##ModelId=3E4BCA7003CA
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


//##ModelId=3E00C8540129
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
    append(out,status_->sdebug(abs(detail)-1));
  }
  return out;
}

}; // end namespace SolverControl



