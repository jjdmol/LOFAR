//  BatchAgent.cc: agent for batch solver control
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

#include "BatchAgent.h"

namespace SolverControl {

using namespace AppState;

//##ModelId=3E005A8403E5
bool BatchAgent::init (const DataRecord &data)
{
  bool rethrow = data[FThrowError].as<bool>(False);
  try
  {
    FailWhen( !SolverControlAgent::init(data),"base init failed" );
    
    const DataRecord &rec = data[initfield()];
    cdebug(3)<<"init: "<<rec.sdebug(6)<<endl;
    
    // get the sub-record of solution jobs
    int nparams = rec[FBatchJobs].size();
    FailWhen( !nparams,"no job sub-records in in "+FBatchJobs.toString()+" field" );
    // copy all solution jobs to the queue
    jobs_.resize(nparams);
    for( int i=0; i<nparams; i++ )
    {
      // attach a ref to the parameter subrecord
      jobs_[i].attach(rec[FBatchJobs][i].as<DataRecord>());
    }
    dprintf(1)("init: %d solve jobs initialized\n",nparams);  
    return True;
  }
  catch( std::exception &exc )
  {
    // throw it on, if requested
    if( rethrow )
      throw(exc);
    setState(ERROR);
    return False;
  }
}

//##ModelId=3E01FA8D02FF
int BatchAgent::startDomain (const DataRecord::Ref &data)
{
  int res = SolverControlAgent::startDomain();
  if( res <= 0 )
    return res;
  // re-copy job queue for entire domain
  Thread::Mutex::Lock lock(mutex()); // lock to make it faster
  for( uint i=0; i < jobs_.size(); i++ )
    addSolution(jobs_[i]);
  return res;
}

//##ModelId=3E70A1C501BB
int BatchAgent::endSolution (DataRecord::Ref &endrec)
{
  int res = SolverControlAgent::endSolution(endrec);
  // no more solutions pending? Force NEXT_DOMAIN state
  if( res == IDLE && solveQueue().empty() )
    return setState(NEXT_DOMAIN);
  else
    return res;
}

//##ModelId=3E009B2E01DF
void BatchAgent::close ()
{
  SolverControlAgent::close();
  jobs_.clear();
}


//##ModelId=3E00C7F3027E
string BatchAgent::sdebug ( int detail,const string &prefix,
                                    const char *name ) const
{
  using Debug::append;
  using Debug::appendf;
  using Debug::ssprintf;

  string out = SolverControlAgent::sdebug(detail,prefix,name?name:"BatchAgent");
  if( detail >= 1 || detail == -1 )
    appendf(out,"%d jobs",jobs_.size());

  return out;
}

} // namespace SolverControl

