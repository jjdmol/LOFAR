//#  KernelGroup.cc: minimal control wrapper around the kernel
//#
//#  Copyright (C) 2002-2007
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

#include <lofar_config.h>

#include <BBSControl/KernelGroup.h>
#include <BBSControl/Exceptions.h>
#include <BBSControl/Types.h>
#include <BBSControl/Messages.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>

namespace LOFAR
{
  namespace BBS
  {
    using LOFAR::operator<<;

    //##--------  P u b l i c   m e t h o d s  --------##//

    void KernelGroup::handle(const CoeffIndexMsg &message)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG_STR("KernelGroup: Handling CoeffIndexMsg, kernel-id = " 
                    << message.getKernelId());

      switch (itsState) {
      default:
        THROW (SolverControlException, "Illegal message `" << message.type() 
               << "' received while in " << showState() << " state");
        break;

      case IDLE:
        itsState = INDEXING;
        LOG_DEBUG("Changed state to INDEXING");
        // deliberate fall through

      case INDEXING:
        // This implementation is not robust againts one kernel sending
        // a message multiple times! This should probably be improved.

        // itsSolver.setCoeffIndex(message);
        LOG_INFO_STR("-msgCount[itsState] = " << msgCount[itsState]);
        LOG_INFO_STR("itsNrKernels = " << itsNrKernels);
        if (++msgCount[itsState] == itsNrKernels) {
          LOG_INFO_STR("+msgCount[itsState] = " << msgCount[itsState]);
          // Reset counter.
          msgCount[itsState] = 0;
          itsState = INITIALIZING;
          LOG_DEBUG("Changed state to INITIALIZING");
        }
        break;

      }
    }

    void KernelGroup::handle(const CoeffMsg &message)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG("KernelGroup: Handling a CoeffMsg");

      switch (itsState) {
      default:
        THROW (SolverControlException, "Illegal message `" << message.type()
               << "' received while in " << showState() << " state");
        break;

      case INITIALIZING:
        // This implementation is not robust againts one kernel sending
        // a message multiple times! This should probably be improved.

        // itsSolver.setCoeff(message);
        if (++msgCount[itsState] == itsNrKernels) {
          // Reset counter.
          msgCount[itsState] = 0;
          itsState = ITERATING;
          LOG_DEBUG("Changed state to ITERATING");
        }
        break;

      }
    }

    void KernelGroup::handle(const EquationMsg &message)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG("KernelGroup: Handling a EquationMsg");

      switch (itsState) {
      default:
        THROW (SolverControlException, "Illegal message `" << message.type()
               << "' received while in " << showState() << " state");
        break;

      case ITERATING:
        // This implementation is not robust againts one kernel sending
        // a message multiple times! This should probably be improved.

        // itsSolver.setEquations(message);
        if (++msgCount[itsState] == itsNrKernels) {
          // Reset counter.
          msgCount[itsState] = 0;
          // Let the solver perform another iteration
          // if (itsSolver.iterate(solutionMsg)) {
          //   // send the solution back to all kernels
          //   itsState = INITIALIZING;
          //   LOG_DEBUG("Changed state to INITIALIZING");
          // }
        }
        break;
      }
    }


    void KernelGroup::handle(const ChunkDoneMsg &message)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG("KernelGroup: Handling a ChunkDoneMsg");

      switch (itsState) {
      default:
        THROW (SolverControlException, "Illegal message `" << message.type()
               << "' received while in " << showState() << " state");
        break;

      case INITIALIZING:
        // This implementation is not robust againts one kernel sending
        // a message multiple times! This should probably be improved.

        if (++msgCount[itsState] == itsNrKernels) {
          // Reset counter.
          msgCount[itsState] = 0;
          itsState = INDEXING;
          LOG_DEBUG("Changed state to INDEXING");
        }
        break;
      }
    }


    const string& KernelGroup::showState() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          State that is defined in the header file!
      static const string states[N_States+1] = {
        "IDLE",
        "INDEXING",
        "INITIALIZING",
        "ITERATING",
        "<UNDEFINED>"  //# This should ALWAYS be last !!
      };
      if (IDLE <= itsState && itsState < N_States) return states[itsState];
      else return states[N_States];
    }

  } //# namespace BBS

} //# namespace LOFAR
