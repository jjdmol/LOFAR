//#  SolverProcessControl.h: Implementation of the ProcessControl
//#     interface for the BBS solver component.
//#
//#  Copyright (C) 2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_BBSCONTROL_SOLVERPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_SOLVERPROCESSCONTROL_H

// \file
// Implementation of the ProcessControl interface for the BBS solver component.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <BBSControl/CommandQueue.h>
#include <BBSControl/KernelConnection.h>
#include <BBSControl/SolveTask.h>
#include <BBSControl/SolveStep.h>

#include <PLC/ProcessControl.h>
#include <Common/lofar_smartptr.h>

namespace LOFAR
{
  namespace BBS
  {
    class SolverOptions;
    
    // \addtogroup BBSControl
    // @{

    // Implementation of the ProcessControl interface for the BBS solver
    // component.
    class SolverProcessControl: public ACC::PLC::ProcessControl
    {
    public:
      // Default constructor.
      SolverProcessControl();

      // Destructor.
      virtual ~SolverProcessControl();

      // @name Implementation of PLC interface.
      // @{
      virtual tribool define();
      virtual tribool init();
      virtual tribool run();
      virtual tribool pause(const string& condition);
      virtual tribool release();
      virtual tribool quit();
      virtual tribool snapshot(const string& destination);
      virtual tribool recover(const string& source);
      virtual tribool reinit(const string& configID);
      virtual string  askInfo(const string& keylist);
      // @}

    private:
      enum RunState {
        UNDEFINED = -1,
        WAIT,
        GET_COMMAND,
        SOLVE,
        QUIT,
        //# Insert new types HERE !!
        N_States
      };

      // Returns whether the current solver control process controls a global
      // solver or not. 
      // \note For the time being we will assume that, when the number of
      // kernels that will connect to the solver control process is more than
      // one, the solver control process controls a global solver.
      bool isGlobal() const;

      // Set run state to \a state
      void setState(RunState state);

      // Return the current state as a string.
      const string& showState() const;

      // Set kernel groups. The argument \a groups should be interpreted as
      // follows:
      // - The size of the vector determines the number of groups.
      // - The sum of the elements determines the total number of kernels.
      // - Each element determines the number of kernels per group.
      void setSolveTasks(const vector<uint>& groups,
                         const SolverOptions& options);

      // (Run) state of the solver control process
      RunState itsState;

      // Number of kernels that will connect to this solver control process.
      uint itsNrKernels;

      // Connection to the command queue.
      scoped_ptr<CommandQueue> itsCommandQueue;

      // Sender ID, used when posting results to the command queue.
      SenderId itsSenderId;

      // Command that should be executed next, or is currently being
      // executed.
      NextCommandType itsCommand;

      // Current solve command. We need to keep track of it, because we need
      // the information in it between different calls to the run() method.
      shared_ptr<const SolveStep> itsSolveStep;

      // Vector of kernels.
      vector<KernelConnection> itsKernels;
      
      // Container of solve tasks. Each task is executed by a different kernel
      // group.
      vector<SolveTask> itsSolveTasks;
    };

    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
