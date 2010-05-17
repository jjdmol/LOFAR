//# SolveTask.cc: Solve task for a given group of kernels.
//#
//# Copyright (C) 2002-2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <BBSControl/SolveTask.h>
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

    SolveTask::SolveTask(const vector<KernelConnection>& kernels,
      const SolverOptions& options) :
      itsKernels(kernels),
      itsState(IDLE)
    {
      itsSolver.reset(options);
    }


    bool SolveTask::run()
    {
      if(itsState != DONE) {
        // Receive messages from our kernel(s); for the time being we'll use
        // a round-robin "polling". Every message is handed over to the
        // kernel group that currently "holds" the kernel identified by the
        // kernel-index in the received message.
        // Note that the current implementation assumes blocking I/O.
        for (unsigned int i = 0; i < itsKernels.size(); ++i) {
          LOG_TRACE_STAT_STR("Kernel #" << i << " (index=" <<
                             itsKernels[i].index() << ") : recvObject()");
          shared_ptr<const KernelMessage> msg(itsKernels[i].recvMessage());
          if (msg) msg->passTo(*this);
        }
      }

      return (itsState == DONE);
    }


    //##--------  P r i v a t e   m e t h o d s  --------##//

    void SolveTask::handle(const ProcessIdMsg &message)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      error("Illegal", message);
    }


    void SolveTask::handle(const CoeffIndexMsg &message)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG_STR("SolveTask: Handling " << message.type() <<
                    ", kernel-index = " << message.getKernelIndex());

      switch (itsState) {
      default:
        error("Illegal", message);
        break;

      case IDLE:
        setState(INDEXING);
        // deliberate fall through

      case INDEXING:
        // Check to see whether this kernel has already sent a CoeffIndexMsg.
        if (!itsKernelMessageReceived.insert(message.getKernelIndex()).second) {
          error("Duplicate", message);
        }
        // Set the coefficient index in the solver
        itsSolver.setCoeffIndex(message.getKernelIndex(),
            message.getContents());

        // If all kernels have sent a CoeffIndexMsg, we should send a the
        // merged coefficient indices back to our kernels.
        if (itsKernelMessageReceived.size() == itsKernels.size()) {
          MergedCoeffIndexMsg msg;
          msg.getContents() = itsSolver.getCoeffIndex();
          for (unsigned int i = 0; i < itsKernels.size(); ++i) {
            itsKernels[i].sendMessage(msg);
          }
          itsKernelMessageReceived.clear();
          setState(INITIALIZING);
        }
        break;

      }
    }


    void SolveTask::handle(const CoeffMsg &message)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG_STR("SolveTask: Handling " << message.type() <<
                    ", kernel-index = " << message.getKernelIndex());

      switch (itsState) {
      default:
        error("Illegal", message);
        break;

      case INITIALIZING:
        // Check to see whether this kernel has already sent a CoeffMsg.
        if (!itsKernelMessageReceived.insert(message.getKernelIndex()).second) {
          error("Duplicate", message);
        }
        // Set initial coefficients in the solver.
        const vector<CellCoeff> &coeff = message.getContents();
        itsSolver.setCoeff(message.getKernelIndex(), coeff.begin(),
            coeff.end());

        // If all kernels have sent a CoeffMsg, we can start iterating.
        if (itsKernelMessageReceived.size() == itsKernels.size()) {
          itsKernelMessageReceived.clear();
          setState(ITERATING);
        }
        break;
      }
    }


    void SolveTask::handle(const EquationMsg &message)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG("SolveTask: Handling a EquationMsg");

      switch (itsState) {
      default:
        error("Illegal", message);
        break;

      case ITERATING:
        // Check to see whether this kernel has already sent a CoeffMsg.
        if (!itsKernelMessageReceived.insert(message.getKernelIndex()).second) {
          error("Duplicate", message);
        }
        // Set equations in the solver.
        const vector<CellEquation> &eq = message.getContents();
        itsSolver.setEquations(message.getKernelIndex(), eq.begin(), eq.end());

        // If all kernels have sent an EquationMsg, we can let the solver
        // perform one iteration.
        if (itsKernelMessageReceived.size() == itsKernels.size()) {
          SolutionMsg msg;

          // If stop criterion is reached, change to initializing state.
          if (itsSolver.iterate(back_inserter(msg.getContents()))) {
            setState(INITIALIZING);
          }
          itsKernelMessageReceived.clear();

          // Send the results back to "our" kernels.
          for (unsigned int i = 0; i < itsKernels.size(); ++i) {
            itsKernels[i].sendMessage(msg);
          }
        }
        break;
      }
    }


    void SolveTask::handle(const ChunkDoneMsg &message)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_COND, "");
      LOG_DEBUG("SolveTask: Handling a ChunkDoneMsg");

      switch (itsState) {
      default:
        error("Illegal", message);
        break;

      case INITIALIZING:
        // Check to see whether this kernel has already sent a ChunkDoneMsg.
        if (!itsKernelMessageReceived.insert(message.getKernelIndex()).second) {
          error("Duplicate", message);
        }
        if (itsKernelMessageReceived.size() == itsKernels.size()) {
          itsKernelMessageReceived.clear();
          setState(DONE);
        }
        break;
      }
    }


    void SolveTask::setState(State state)
    {
      itsState = state;
      LOG_DEBUG_STR("Switching to " << showState() << " state");
    }


    const string& SolveTask::showState() const
    {
      //# Caution: Always keep this array of strings in sync with the enum
      //#          State that is defined in the header file!
      static const string states[N_States+1] = {
        "IDLE",
        "INDEXING",
        "INITIALIZING",
        "ITERATING",
        "DONE",
        "<UNDEFINED>"  //# This should ALWAYS be last !!
      };
      if (IDLE <= itsState && itsState < N_States) return states[itsState];
      else return states[N_States];
    }


    void SolveTask::error(const string& prefix, const KernelMessage& message)
    {
      THROW (SolveTaskException, prefix << " message " << message.type() <<
             " received from kernel (index=" << message.getKernelIndex() <<
             ") while in " << showState() << " state");
    }

  } //# namespace BBS

} //# namespace LOFAR
