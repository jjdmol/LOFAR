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

#include <BBSControl/CalSession.h>
#include <BBSControl/CommandVisitor.h>
#include <BBSControl/KernelConnection.h>
#include <BBSControl/SolveTask.h>

#include <PLC/ProcessControl.h>
#include <Common/lofar_smartptr.h>
#include <Common/Net/Socket.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    // Implementation of the ProcessControl and the CommandVisitor interface
    // for the BBS solver component.
    class SolverProcessControl: public ACC::PLC::ProcessControl,
                                public CommandVisitor
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

      // @name Implementation of CommandVisitor interface.
      // @{
      virtual CommandResult visit(const InitializeCommand &command);
      virtual CommandResult visit(const FinalizeCommand &command);
      virtual CommandResult visit(const NextChunkCommand &command);
      virtual CommandResult visit(const RecoverCommand &command);
      virtual CommandResult visit(const SynchronizeCommand &command);
      virtual CommandResult visit(const MultiStep &command);
      virtual CommandResult visit(const PredictStep &command);
      virtual CommandResult visit(const SubtractStep &command);
      virtual CommandResult visit(const AddStep &command);
      virtual CommandResult visit(const CorrectStep &command);
      virtual CommandResult visit(const SolveStep &command);
      virtual CommandResult visit(const ShiftStep &command);
      virtual CommandResult visit(const RefitStep &command);
      // @}

    private:
      enum State
      {
        UNDEFINED,
        WAIT,
        RUN,
        //# Insert new types here!
        N_State
      };

      // Returns a CommandResult with a descriptive message.
      CommandResult unsupported(const Command &command) const;

      // Set run state to \a state
      void setState(State state);

      // Return the current state as a string.
      const string& showState() const;

      // Set kernel groups. The argument \a groups should be interpreted as
      // follows:
      // - The size of the vector determines the number of groups.
      // - The sum of the elements determines the total number of kernels.
      // - Each element determines the number of kernels per group.
      void setSolveTasks(const vector<uint>& groups,
                         const SolverOptions& options);

      // State of the solver control process.
      State                     itsState;

      // Calibration session information.
      scoped_ptr<CalSession>    itsCalSession;

      // Listen socket.
      Socket                    itsSocket;

      // Vector of kernels.
      vector<KernelConnection>  itsKernels;

      // Container of solve tasks. Each task is executed by a different kernel
      // group.
      vector<SolveTask>         itsSolveTasks;
    };

    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
