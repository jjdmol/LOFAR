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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <BBSControl/SolverProcessControl.h>
#include <BBSControl/BlobStreamableConnection.h>
#include <BBSControl/CommandQueue.h>
#include <BBSControl/StreamUtil.h>

#include <APS/ParameterSet.h>
#include <Blob/BlobStreamable.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_smartptr.h>

#include <stdlib.h>

#define NONREADY        0
#define N_ReadyCode     999
#define NC              0
#define SUMLL           2

namespace LOFAR
{
  using namespace ACC::APS;

  namespace BBS
  {
    using LOFAR::operator<<;
//     using boost::scoped_ptr;

    //# Ensure classes are registered with the ObjectFactory.
    template class BlobStreamableVector<DomainRegistrationRequest>;
    template class BlobStreamableVector<IterationRequest>;
    template class BlobStreamableVector<IterationResult>;


    //##----   P u b l i c   m e t h o d s   ----##//

    SolverProcessControl::SolverProcessControl() :
      ProcessControl(),
      itsState(UNDEFINED),
      itsNrKernels(0)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    SolverProcessControl::~SolverProcessControl()
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW, "");
    }


    tribool SolverProcessControl::define()
    {
      LOG_INFO("SolverProcessControl::define()");
      try {
        // Get the number of kernels that will connect 
        itsNrKernels = globalParameterSet()->getUint32("Solver.nrKernels");
      }
      catch(Exception& e) {
        LOG_ERROR_STR(e);
        return false;
      }
      return true;
    }


    tribool SolverProcessControl::init()
    {
      LOG_INFO("SolverProcessControl::init()");
      try {
        // Socket acceptor. In the global case, it will become a TCP listen
        // socket; in the local case a Unix domain socket. The acceptor can be
        // a stack object, since we don't need it anymore, once all kernels
        // have connected.
        Socket acceptor("SolverProcessControl");

        // Socket connector. In the global case, this will become a TCP socket
        // accepted by the listening socket; in the local case, a Unix domain
        // socket. The connector is a heap object, since we need it in the
        // run() method.
        shared_ptr<BlobStreamableConnection> connector;

        if (isGlobal()) {
          LOG_DEBUG("This is a global solver.");

          // Create a new CommandQueue. This will open a connection to the
          // blackboard database.
          itsCommandQueue.reset
            (new CommandQueue(globalParameterSet()->getString("BBDB.DBName"),
                              globalParameterSet()->getString("BBDB.UserName"),
                              globalParameterSet()->getString("BBDB.Host"),
                              globalParameterSet()->getString("BBDB.Port")));

          // Register for the "command" trigger, which fires when a new
          // command is posted to the blackboard database.
          itsCommandQueue->registerTrigger(CommandQueue::Trigger::Command);

          // Create a TCP listen socket that will accept incoming kernel
          // connections.
          acceptor.initServer(globalParameterSet()->getString("Solver.Port"), 
                              Socket::TCP);
        }
        else {
          LOG_DEBUG("This is a local solver.");

          // Create a Unix domain socket to connect to "our" kernel.
          ostringstream oss;
          oss << "=Solver_" << getenv("USER") 
              << globalParameterSet()->getInt32("Solver.Id");
          acceptor.initServer(oss.str(), Socket::LOCAL);
        }

        //  Wait for all kernels to connect.
        LOG_DEBUG_STR("Waiting for " << itsNrKernels << 
                      " kernel(s) to connect ...");
        itsKernels.resize(itsNrKernels);
        for (uint i = 0; i < itsNrKernels; ++i) {
          connector.reset(new BlobStreamableConnection(acceptor.accept()));
          if (!connector->connect()) {
            THROW (IOException, "Failed to connect kernel #" << i); 
          }
          itsKernels[i] = connector;
          LOG_DEBUG_STR("Kernel #" << i << " connected.");
        }

        // Switch to IDLE state, indicating that we're ready to receive
        // commands. Note that we will only react to Solve commands; others
        // will be silently ignored.
        itsState = IDLE;
      }

      catch(Exception& e) {
        LOG_ERROR_STR(e);
        return false;
      }

      // All went well.
      return true;
    }


    tribool SolverProcessControl::run()
    {
      LOG_INFO("SolverProcessControl::run()");

      try {

        switch(itsState) {

        case UNDEFINED: {
          LOG_WARN("RunState is UNDEFINED; init() must be called first!");
          return false;
        }

        case IDLE: {
          LOG_TRACE_FLOW("RunState::IDLE");
          LOG_DEBUG("Waiting for command trigger ...");
          if (itsCommandQueue->waitForTrigger(CommandQueue::Trigger::Command)) {
            // Get the next command. If it's a SolveStep, we can change state.
            shared_ptr<const Command> cmd = 
              itsCommandQueue->getNextCommand().first;
            LOG_DEBUG_STR("Received a `" << cmd->type() << "' command");
            itsSolveStep = dynamic_pointer_cast<const SolveStep>(cmd);
            if (itsSolveStep) itsState = INDEXING;
          }
          break;
        }

        case INDEXING: {
          LOG_TRACE_FLOW("RunState::INDEXING");

          // Set-up map that maps kernel-id to kernel group-id
//           vector<uint> kernelGroups = globalParameterSet()->getVector<

          // Get the coefficient indices from our kernel(s).

          // Send indices back to all kernels.

          break;
        }

        case INITIALZING: {
          LOG_TRACE_FLOW("RunState::INITIALZING");

          // Set initial coefficients

          break;
        }

        case ITERATING: {
          LOG_TRACE_FLOW("RunState::ITERATING");

          // While not converged:
          //   Set equations
          //   iterate

          break;
        }

        default: {
          LOG_ERROR("RunState is UNKNOWN: this is a program logic error!");
          return false;
        }

        } // switch

#if 0
        // Receive the next message
        scoped_ptr<BlobStreamable> message(itsKernelConnection->recvObject());
        if(message)
          return dispatch(message.get());
        else
          return false;
#endif

      }
      catch(Exception& e)
      {
        LOG_ERROR_STR(e);
        return false;
      }
    }


    tribool SolverProcessControl::pause(const string& /*condition*/)
    {
      LOG_INFO("SolverProcessControl::pause()");
      LOG_WARN("Not supported");
      return false;
    }


    tribool SolverProcessControl::release()
    {
      LOG_INFO("SolverProcessControl::release()");
      LOG_WARN("Not supported");
      return false;
    }


    tribool SolverProcessControl::quit()
    {
      LOG_INFO("SolverProcessControl::quit()");
      return true;
    }


    tribool SolverProcessControl::snapshot(const string& /*destination*/)
    {
      LOG_INFO("SolverProcessControl::snapshot()");
      LOG_WARN("Not supported");
      return false;
    }


    tribool SolverProcessControl::recover(const string& /*source*/)
    {
      LOG_INFO("SolverProcessControl::recover()");
      LOG_WARN("Not supported");
      return false;
    }


    tribool SolverProcessControl::reinit(const string& /*configID*/)
    {
      LOG_INFO("SolverProcessControl::reinit()");
      LOG_WARN("Not supported");
      return false;
    }


    std::string SolverProcessControl::askInfo(const string& /*keylist*/)
    {
      LOG_INFO("SolverProcessControl::askInfo()");
      LOG_WARN("Not supported");
      return string();
    }


    //##--------   P r i v a t e   m e t h o d s   --------##//

    bool SolverProcessControl::isGlobal() const
    {
      return itsNrKernels > 1;
    }


    bool SolverProcessControl::dispatch(const BlobStreamable *message)
    {
      {
        const DomainRegistrationRequest *request =
          dynamic_cast<const DomainRegistrationRequest*>(message);
        if(request)
          return handle(request);
      }

      {
        const BlobStreamableVector<DomainRegistrationRequest> *request =
          dynamic_cast<const BlobStreamableVector<DomainRegistrationRequest>*>
          (message);
        if(request)
          return handle(request);
      }

      {
        const IterationRequest *request =
          dynamic_cast<const IterationRequest*>(message);
        if(request)
          return handle(request);
      }

      {
        const BlobStreamableVector<IterationRequest> *request =
          dynamic_cast<const BlobStreamableVector<IterationRequest>*>
          (message);
        if(request)
          return handle(request);
      }

      // We received a message we can't handle
      LOG_WARN_STR("Received message of unsupported type");
      return false;
    }


    bool SolverProcessControl::handle(const DomainRegistrationRequest *request)
    {
      return registerDomain(request);
    }


    bool SolverProcessControl::handle
    (const BlobStreamableVector<DomainRegistrationRequest> *request)
    {
      bool result = true;

      for(vector<DomainRegistrationRequest*>::const_iterator it =
            request->getVector().begin();
          it != request->getVector().end() && result;
          ++it)
      {
        result = result && registerDomain(*it);
      }
      return result;
    }


    bool SolverProcessControl::handle(const IterationRequest *request) const
    {
#if 0
      scoped_ptr<IterationResult> result(performIteration(request));
      return itsKernelConnection->sendObject(*result.get());
#endif
    }


    bool SolverProcessControl::handle
    (const BlobStreamableVector<IterationRequest> *request) const
    {
#if 0
      BlobStreamableVector<IterationResult> result;

      for(vector<IterationRequest*>::const_iterator it =
            request->getVector().begin();
          it != request->getVector().end();
          ++it)
      {
        result.getVector().push_back(performIteration(*it));
      }

      return itsKernelConnection->sendObject(result);
#endif
    }


    bool SolverProcessControl::registerDomain
    (const DomainRegistrationRequest *request)
    {
      LOG_DEBUG_STR("DomainRegistrationRequest: index: "
                    << request->getDomainIndex() << " #unknowns: "
                    << request->getUnknowns().size());
      LOG_DEBUG_STR("+ unknowns: " << request->getUnknowns());

      Domain &domain = itsRegisteredDomains[request->getDomainIndex()];
      domain.index = request->getDomainIndex();
      domain.unknowns = request->getUnknowns();
      domain.solver.set(casa::uInt(domain.unknowns.size()));
      // Set new value solution test
      domain.solver.setEpsValue(request->getEpsilon());
      // Set new 'derivative' test (actually, the inf norm of the known
      // vector is checked).
      domain.solver.setEpsDerivative(request->getEpsilon());
      // Set maximum number of iterations
      domain.solver.setMaxIter(request->getMaxIter());
      // Set new factors (collinearity factor, and Levenberg-Marquardt LMFactor)
      domain.solver.set(request->getColFactor(), request->getLMFactor());
      domain.solver.setBalanced(request->getBalancedEq());
      return true;
    }


    IterationResult* SolverProcessControl::performIteration
    (const IterationRequest *request)
    {
      map<uint32, Domain>::iterator it =
        itsRegisteredDomains.find(request->getDomainIndex());

      if(it == itsRegisteredDomains.end())
      {
        // Artifact of the current implementation: equations are still being
        // generated and send even if the solve domain has already
        // converged. So, we'll just return a result with resultCode N_ReadyCode
        // (i.e. already converged).
        return new IterationResult(request->getDomainIndex(),
                                   N_ReadyCode, "", vector<double>(), 0, -1.0, -1.0);
      }

      // Set the new normal equations.
      Domain &domain = it->second;
      domain.solver.merge(request->getNormalEquations());

      // Get some statistics from the solver. Note that the chi squared is
      // valid for the _previous_ iteration. The solver cannot compute the
      // chi squared directly after an iteration, because it needs the new
      // condition equations for that and these are computed by the kernel.
      casa::uInt rank, nun, np, ncon, ner, *piv;
      casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
      domain.solver.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er,
                            piv, sEq, sol, prec, nonlin);
      ASSERT(er && ner > SUMLL);

      double lmFactor = nonlin;
      double chiSquared = er[SUMLL] / std::max(er[NC] + nun, 1.0);

      // Solve the normal equations.
      domain.solver.solveLoop(rank, &(domain.unknowns[0]), true);

      // Construct a result.
      IterationResult *result = new IterationResult(domain.index,
                                                    domain.solver.isReady(),
                                                    domain.solver.readyText(),
                                                    domain.unknowns,
                                                    rank,
                                                    chiSquared,
                                                    lmFactor);

      // If we're done with this domain, unregister it.
      if(domain.solver.isReady() != NONREADY)
      {
        itsRegisteredDomains.erase(request->getDomainIndex());
      }

      return result;
    }

  } // namespace BBS

} // namespace LOFAR
