//#  KernelGroup.h: Group of kernels associated with a solver.
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

#ifndef LOFAR_BBS_KERNELGROUP_H
#define LOFAR_BBS_KERNELGROUP_H

// \file
// Group of kernels associated with a solver.

//# Includes
#include <BBSControl/MessageHandlers.h>
#include <BBSKernel/Solver.h>
#include <Common/lofar_vector.h>
#include <Common/LofarTypes.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward declarations
    class CoeffIndexMsg;
    class CoefficientMsg;
    class EquationMsg;
    class SolutionMsg;
    class ChunkDoneMsg;

    // \addtogroup BBS
    // @{

    // Group of kernels that share one (global) solver. This class implements
    // the KernelMessageHandler interface in order to handle the different
    // kernel messages that it will receive.
    class KernelGroup : public KernelMessageHandler
    {
    private:
      // All states a kernel group can be in.
      enum State {
        IDLE = 0,
        INDEXING,
        INITIALIZING,
        ITERATING,
        //# Insert new states HERE !!
        N_States          ///< Number of states.
      };

    public:
      // Construct a kernel group that consists of \a nrKernels kernels.
      // \note This class does \e not contain any kernel objects; it merely
      // keeps track of the count.
      KernelGroup(uint nrKernels) :
        itsNrKernels(nrKernels),
        itsState(IDLE),
        msgCount(N_MsgTypes)
      {}

      virtual void handle(const CoeffIndexMsg &message);
      virtual void handle(const CoeffMsg &message);
      virtual void handle(const EquationMsg &message);
      virtual void handle(const ChunkDoneMsg &message);

      uint nrKernels() const { return itsNrKernels; }

//       void state(State s) { itsState = s; }
//       State state() const { return itsState; }
      const string& showState() const;

    private:

      enum MsgType {
        COEFFINDEX = 0,
        COEFF,
        EQUATION,
        CHUNKDONE,
        N_MsgTypes
      };

      // Number of kernels in this group
      uint itsNrKernels;

      // State that this kernel group is in.
      State itsState;

      // Message counter. Counts the number of messages received of a certain
      // type. When we've received \c nrKernels messages we can change state.
      // The vector will contain \c N_MsgTypes elements.
      vector<uint> msgCount;

      // The solver associated with this kernel group.
      Solver itsSolver;
      
    };

    // @}

  } //# namespace BBS

} //# namespace LOFAR

#endif
