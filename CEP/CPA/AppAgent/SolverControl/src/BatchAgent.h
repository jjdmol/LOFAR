//  BatchAgent.h: agent for batch solver control
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
#ifndef SOLVERCONTROL_SRC_BATCHAGENT_H_HEADER_INCLUDED_FC8CF8CF
#define SOLVERCONTROL_SRC_BATCHAGENT_H_HEADER_INCLUDED_FC8CF8CF
    
#include <SolverControl/SolverControlAgent.h>
#include <vector>

#pragma aid Batch Solver Control Job Jobs Max

namespace SolverControl {

const HIID
    // Constants for field names
    //    Subrecord of batch control parameters
    FBatchControlJobs      = AidBatch|AidSolver|AidControl|AidJobs,
    //    Max number of iterations
    FMaxIterations         = AidMax|AidIteration;
    // Also use:
    // FSolutionParams
    // FConvergence
        
// default arguments
const double DefaultConvergence = 1e-3;
const int DefaultMaxIterations  = 100;    

//##ModelId=3DFF2C0700D8
class BatchAgent : public SolverControlAgent
{
  public:
    //##ModelId=3DFF2CCD01A1
    BatchAgent();

    //##ModelId=3E005A8403E5
    //##Documentation
    //## Initializes solution criteria.
    virtual bool init (const DataRecord::Ref &data);
    
    //##ModelId=3E01FA8D02FF
    //##Documentation
    //## Called by application to start solving for a new domain
    //## Resets to beginning of job queue
    virtual int startDomain (const DataRecord::Ref &data = DataRecord::Ref());
    
    //##ModelId=3E0098E90136
    //##Documentation
    //## Gets next solution from job queue
    virtual int startSolution (DataRecord::Ref &params);
    
    //##ModelId=3E0060C50000
    //##Documentation
    //## Keeps track of solution status, and compares it against the
    //## initialized criteria. Once a criteria is hit, posts an EndSolveEvent
    //## to advertise this, calls stopSolution(), and returns False to wind
    //## things down.
    virtual int endIteration(double conv);
    
    //##ModelId=3E009B2E01DF
    //##Documentation
    //## Clears flags and solution parameters
    virtual void close();
    
    //##ModelId=3E00C7F3027E
    string sdebug ( int detail = 1,const string &prefix = "",
                    const char *name = 0 ) const;


  private:
    //##ModelId=3DFF2CCD01B4
    BatchAgent (const BatchAgent& right);

    //##ModelId=3DFF2CCD021E
    BatchAgent& operator=(const BatchAgent& right);
    
    //##ModelId=3E0060D20249
    //##Documentation
    //## Criterion: maximum number of solve iterations
    int max_iterations_;

    //##ModelId=3E0060D901DE
    //##Documentation
    //## Criterion: required convergence
    double conv_threshold_;
    
    //##ModelId=3E00990E03DD
    std::vector<DataRecord::Ref> jobs_;
    
    //##ModelId=3E3E55820037
    uint current_job_;

};

} // namespace SolverControl



#endif /* SOLVERCONTROL_SRC_BATCHAGENT_H_HEADER_INCLUDED_FC8CF8CF */
