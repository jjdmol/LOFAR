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

//##ModelId=3DFF2CCD01A1
BatchAgent::BatchAgent()
{
}

//##ModelId=3E005A8403E5
bool BatchAgent::init (const DataRecord::Ref &data)
{
  if( !SolverControlAgent::init(data) )
    return False;
  
  const DataRecord &rec = *data;
  cdebug(3)<<"init: "<<data->sdebug(6)<<endl;
  // get the sub-record of solution jobs
  int nparams = rec[FBatchControlJobs].size();
  FailWhen( !nparams,"no job sun-records in in "+FBatchControlJobs.toString()+" field" );
  // copy all solution jobs to the queue
  jobs_.resize(nparams);
  for( int i=0; i<nparams; i++ )
  {
    // attach a ref to the parameter subrecord
    jobs_[i].attach( rec[FBatchControlJobs][i].as_DataRecord() );
  }
  current_job_ = 0;
  
  dprintf(1)("init: %d solve jobs initialized\n",nparams);  
  return True;
}

//##ModelId=3E0060C50000
int BatchAgent::endIteration (double conv)
{
  // call parent's endIteration
  int st = SolverControlAgent::endIteration(conv);
  if( st != RUNNING )
    return st;
  
  // do we have another job queued up?
  st = current_job_ >= jobs_.size() ? NEXT_DOMAIN : NEXT_SOLUTION;
  
  // check for max iteration count -- interrupt if reached
  if( iterationNum() >= max_iterations_ )
    endSolution("Iteration count exceeded",st);
  
  // check for convergence -- end if reached
  if( convergence() <= conv_threshold_ )
    endSolution("Converged",st);
  
  return state();
}

//##ModelId=3E01FA8D02FF
int BatchAgent::startDomain (const DataRecord::Ref &data)
{
  SolverControlAgent::startDomain();
  current_job_ = 0;
  setState(RUNNING);
  return state();
}

//##ModelId=3E0098E90136
int BatchAgent::startSolution (DataRecord::Ref &params)
{
  if( current_job_ >= jobs_.size() )
  {
    setState(NEXT_DOMAIN);
    return state();
  }
  
  // get next set of solution parameters from job queue
  params = jobs_[current_job_].copy();
  // advance pointer to next solve job
  current_job_++;  
  // get solve criteria
  conv_threshold_ = (*params)[FConvergence].as_double(DefaultConvergence);
  max_iterations_ = (*params)[FMaxIterations].as_int(DefaultMaxIterations);
  
  dprintf(1)("starting solution, niter=%d, convergence=%f\n",
              max_iterations_,conv_threshold_);
  
  // init the solution
  initSolution(params);
  
  setState(RUNNING);
  return state();
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
    appendf(out,"maxi:%d c0:%f job %d/%d",max_iterations_,conv_threshold_,current_job_,jobs_.size());

  return out;
}

} // namespace SolverControl
