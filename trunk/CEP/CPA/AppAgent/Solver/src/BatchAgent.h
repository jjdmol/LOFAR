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
    
#include <Solver/SolverControlAgent.h>
#include <vector>
class AppEventSink;

#pragma aidgroup Solver
#pragma aid Batch Solver Control Job Jobs 

namespace SolverControl {

const HIID
    // Constants for field names
    //    Subrecord of batch jobs
    FBatchJobs      = AidBatch|AidJobs;
    
//##ModelId=3DFF2C0700D8
class BatchAgent : public SolverControlAgent
{
  public:
    //##ModelId=3DFF2CCD01A1
    BatchAgent(const HIID &initf = AidControl)
      : SolverControlAgent(initf) {}
    //##ModelId=3E42792B02D8
    BatchAgent(AppEventSink &sink, const HIID &initf = AidControl)
      : SolverControlAgent(sink,initf) {}
    //##ModelId=3E50FBE500AE
    BatchAgent(AppEventSink *sink, int dmiflags, const HIID &initf = AidControl)
      : SolverControlAgent(sink,dmiflags,initf) {}

    //##ModelId=3E005A8403E5
    //##Documentation
    //## Initializes solution criteria.
    virtual bool init (const DataRecord &data);
    
    //##ModelId=3E01FA8D02FF
    //##Documentation
    //## Called by application to start solving for a new domain
    //## Resets to beginning of job queue
    virtual int startDomain (const DataRecord::Ref &data = DataRecord::Ref());
    
    //##ModelId=3E009B2E01DF
    //##Documentation
    //## Clears flags and solution parameters
    virtual void close();
    
    //##ModelId=3E00C7F3027E
    string sdebug ( int detail = 1,const string &prefix = "",
                    const char *name = 0 ) const;
    //##ModelId=3E70A1C501BB
    //##Documentation
    //## Called by the solver application to acknowledge end of solution.
    //## Posts an end-of-solution event.
    //## An end record will be attached to endrec upon return.
    //## Entry state must be ENDSOLVE (else if terminal, function will return
    //## immediately, or throw an exception in other states).
    //## Returns the current state, which will be one of:
    //##    IDLE (>0): proceed
    //##    terminal state (<=0): see class documentation above.
    virtual int endSolution(const DataRecord::Ref::Copy &data,DataRecord::Ref &endrec);


  private:
    //##ModelId=3E42781C03A6
    BatchAgent (const BatchAgent& right);
    //##ModelId=3DFF2CCD021E
    BatchAgent& operator=(const BatchAgent& right);


    //##ModelId=3E00990E03DD
    std::vector<DataRecord::Ref> jobs_;
    
};

} // namespace SolverControl



#endif /* SOLVERCONTROL_SRC_BATCHAGENT_H_HEADER_INCLUDED_FC8CF8CF */
