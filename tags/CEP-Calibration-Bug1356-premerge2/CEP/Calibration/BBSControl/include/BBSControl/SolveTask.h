//#  SolveTask.h: Solve task for a given group of kernels.
//#
//#  Copyright (C) 2002-2004
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
//#  $Id$

#ifndef LOFAR_BBSCONTROL_SOLVETASK_H
#define LOFAR_BBSCONTROL_SOLVETASK_H

// \file
// Solve task for a given group of kernels.

//# Includes
#include <BBSControl/MessageHandlers.h>
#include <BBSControl/KernelConnection.h>
#include <BBSKernel/Solver.h>
#include <BBSControl/Types.h>
#include <Common/lofar_set.h>
#include <Common/lofar_vector.h>
#include <Common/LofarTypes.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward declarations
//    class KernelIdMsg;
    class ProcessIdMsg;
    class CoeffIndexMsg;
    class CoefficientMsg;
    class EquationMsg;
    class SolutionMsg;
    class ChunkDoneMsg;

    // \addtogroup BBSControl
    // @{

    // Group of kernels that share one (global) solver. This class implements
    // the KernelMessageHandler interface in order to handle the different
    // kernel messages that it will receive.
    class SolveTask : KernelMessageHandler
    {
    private:
      // All states a kernel group can be in.
      enum State {
        IDLE = 0,
        INDEXING,
        INITIALIZING,
        ITERATING,
        DONE,
        //# Insert new states HERE !!
        N_States          ///< Number of states.
      };

    public:
      // Construct a kernel group that consists of \a nrKernels kernels.
      // \note This class does \e not contain any kernel objects; it merely
      // keeps track of the count.
      SolveTask(const vector<KernelConnection>& kernels,
        const SolverOptions& options);

      bool run();

    private:

//      virtual void handle(const KernelIdMsg &message);
      virtual void handle(const ProcessIdMsg &message);
      virtual void handle(const CoeffIndexMsg &message);
      virtual void handle(const CoeffMsg &message);
      virtual void handle(const EquationMsg &message);
      virtual void handle(const ChunkDoneMsg &message);

//       uint nrKernels() const { return itsNrKernels; }

      void setState(State s);
//       State state() const { return itsState; }
      const string& showState() const;

      // Compose error message and thow SolveTaskException.
      void error(const string& prefix, const KernelMessage& message);

    private:

      enum MsgType {
        NOMESSAGE = -1,
        COEFFINDEX,
        COEFF,
        EQUATION,
        CHUNKDONE,
        N_MsgTypes
      };

//       // Number of kernels in this group
//       uint itsNrKernels;
      vector<KernelConnection> itsKernels;
//       vector< pair<KernelConnection, MsgType> > itsKernels;

      // State that this kernel group is in.
      State itsState;

#if 0
      // Message counter. Counts the number of messages received of a certain
      // type. When we've received \c nrKernels messages we can change state.
      // The vector will contain \c N_MsgTypes elements.
      vector<uint> msgCount;
#endif

      // A set is used to "count" whether we've already received a message
      // from a given kernel. We use a set here, because a set provides us
      // with a means to detect whether a kernel has already sent a
      // message. If, by accident, a kernel sends a message twice, this will
      // be detected while inserting the kernel-id in the set.
      //
      // Once all kernels have sent a message of a certain type, we can change
      // state and we should clear the set.
      //
      // \note Because we do not keep track of the type of message, but only
      // whether a given kernel has sent a message, this approach will cease
      // to work correctly once we allow kernels to send message of different
      // types interleaved. For the time being this will certainly not be the
      // case, but it is something to keep in the back of our minds.
//      set<KernelId> itsKernelMessageReceived;
      set<KernelIndex> itsKernelMessageReceived;

      // The solver associated with this kernel group.
      Solver itsSolver;
      
    };

    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
