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

#include <APS/ParameterSet.h>
#include <Blob/BlobStreamable.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_smartptr.h>

#include <stdlib.h>

using namespace LOFAR::ACC::APS;

namespace LOFAR 
{
namespace BBS 
{
    using LOFAR::operator<<;
    using boost::scoped_ptr;


    //# Ensure classes are registered with the ObjectFactory.
    template class BlobStreamableVector<DomainRegistrationRequest>;
    template class BlobStreamableVector<IterationRequest>;
    template class BlobStreamableVector<IterationResult>;


    //##----   P u b l i c   m e t h o d s   ----##//
    SolverProcessControl::SolverProcessControl() :
        ProcessControl()
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    tribool SolverProcessControl::define()
    {
        LOG_INFO("SolverProcessControl::define()");
        
        try
        {
            char *user = getenv("USER");
            ASSERT(user);
            string userString(user);
            
            itsKernelConnection.reset(new BlobStreamableConnection(
                "=Solver_" + userString,
                Socket::LOCAL));
        }
        catch(Exception& e)
        {
            LOG_ERROR_STR(e);
            return false;
        }
        
        return true;
    }


    tribool SolverProcessControl::init()
    {
        LOG_INFO("SolverProcessControl::init()");
        try
        {
            LOG_INFO("Starting to listen at solver@localhost");
            if(!itsKernelConnection->connect())
            {
                LOG_ERROR("+ listen failed");
                return false;
            }
            LOG_INFO("+ ok");
        }
        catch(Exception& e)
        {
            LOG_ERROR_STR(e);
            return false;
        }
        
        // All went well.
        return true;
    }


    tribool SolverProcessControl::run()
    {
        LOG_INFO("SolverProcessControl::run()");

        try
        {
            // Receive the next message
            scoped_ptr<BlobStreamable> message(itsKernelConnection->recvObject());
            if(message)
                return dispatch(message.get());
            else
                return false;
        }
        catch(Exception& e)
        {
            LOG_ERROR_STR(e);
            return false;
        }
    }


    tribool SolverProcessControl::pause(const string& /*condition*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
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
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        LOG_WARN("Not supported");
        return false;
    }


    tribool SolverProcessControl::recover(const string& /*source*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        LOG_WARN("Not supported");
        return false;
    }


    tribool SolverProcessControl::reinit(const string& /*configID*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        LOG_WARN("Not supported");
        return false;
    }


    std::string SolverProcessControl::askInfo(const string& /*keylist*/)
    {
        LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
        return std::string("");
    }


    bool SolverProcessControl::dispatch(const BlobStreamable *message)
    {
        {
            const DomainRegistrationRequest *request = dynamic_cast<const DomainRegistrationRequest*>(message);
            if(request)
                return handle(request);
        }
        
        {
            const BlobStreamableVector<DomainRegistrationRequest> *request = dynamic_cast<const BlobStreamableVector<DomainRegistrationRequest>*>(message);
            if(request)
                return handle(request);
        }
        
        {
            const IterationRequest *request = dynamic_cast<const IterationRequest*>(message);
            if(request)
                return handle(request);
        }
        
        {
            const BlobStreamableVector<IterationRequest> *request = dynamic_cast<const BlobStreamableVector<IterationRequest>*>(message);
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

    
    bool SolverProcessControl::handle(const BlobStreamableVector<DomainRegistrationRequest> *request)
    {
        bool result = true;
        for(vector<DomainRegistrationRequest*>::const_iterator it = request->getVector().begin();
            it != request->getVector().end() && result;
            ++it)
        {
            result = result && registerDomain(*it);
        }
        return result;
    }
    
    
    bool SolverProcessControl::handle(const IterationRequest *request)
    {
        scoped_ptr<IterationResult> result(performIteration(request));
        return itsKernelConnection->sendObject(*result.get());
    }


    bool SolverProcessControl::handle(const BlobStreamableVector<IterationRequest> *request)
    {
        BlobStreamableVector<IterationResult> result;
        
        for(vector<IterationRequest*>::const_iterator it = request->getVector().begin();
            it != request->getVector().end();
            ++it)
        {
            result.getVector().push_back(performIteration(*it));
        }
        
        return itsKernelConnection->sendObject(result);
    }
    
    
    bool SolverProcessControl::registerDomain(const DomainRegistrationRequest *request)
    {
        LOG_DEBUG_STR("DomainRegistrationRequest: index: " << request->getDomainIndex() << " #unknowns: " << request->getUnknowns().size());
        LOG_DEBUG_STR("+ unknowns: " << request->getUnknowns());
        
        Domain &domain = itsRegisteredDomains[request->getDomainIndex()];
        domain.index = request->getDomainIndex();
        domain.unknowns = request->getUnknowns();
        domain.solver.set(casa::uInt(domain.unknowns.size()));
        domain.epsilon = request->getEpsilon();
        // Set new value solution test
        //domain.solver.setEpsValue(request->getEpsilon());
        // Set new 'derivative' test (actually, the inf norm of the known
        // vector is checked).
        //domain.solver.setEpsDerivative(request->getEpsilon());
        // Set maximum number of iterations
        //domain.solver.setMaxIter(request->getMaxIter());
        // Set new factors (collinearity factor, and Levenberg-Marquardt LMFactor)
        // void set(Double factor=1e-6, Double LMFactor=1e-3);
        return true;
    }

        
    IterationResult *SolverProcessControl::performIteration(const IterationRequest *request)
    {
        map<uint32, Domain>::iterator it = itsRegisteredDomains.find(request->getDomainIndex());
        //ASSERT(it != itsRegisteredDomains.end());
        if(it == itsRegisteredDomains.end())
        {
            // Artifact of the current implementation: equations are still being generated
            // and send even if the solve domain has already converged. So, we'll just return
            // a result with resultCode 2 (i.e. already converged).
            return new IterationResult(
                request->getDomainIndex(),
                2,
                "",
                vector<double>(),
                0,
                -1.0,
                -1.0);
        }

        // Set the new normal equations.
        Domain &domain = it->second;
        domain.solver.merge(request->getNormalEquations());

        // Get some statistics from the solver. Note that the chi squared is
        // valid for the _previous_ iteration. The solver cannot compute the
        // chi squared directly after an iteration, because it needs the new
        // condition equations for that and these are computed by the kernel.
        double chiSquared, lmFactor;
        casa::uInt rank, nun, np, ncon, ner, *piv;
//        casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
        casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin, fit;

        domain.solver.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er, piv, sEq, sol, prec, nonlin);
//        ASSERT(er && ner > casa::LSQFit::NC && ner > casa::LSQFit::SUMLL);
        ASSERT(er && ner > 2);
        //# It's weird that er[casa::LSQFit::SUMLL] contains chi squared while
        //# er[casa::LSQFit::CHI2] does not. ??
        //chiSquared = (er[casa::LSQFit::NC] > 0 ? er[casa::LSQFit::SUMLL] / er[casa::LSQFit::NC] : er[casa::LSQFit::SUMLL]);
//        chiSquared = er[casa::LSQFit::SUMLL];
        chiSquared = er[2];
        lmFactor = nonlin;

        //LOG_DEBUG_STR("before " << domain.unknowns);
        
        // Solve the normal equations.
//        domain.solver.solveLoop(rank, &(domain.unknowns[0]), true);
        domain.solver.solveLoop(fit, rank, &(domain.unknowns[0]), true);
        
        //LOG_DEBUG_STR("after " << domain.unknowns);

        uint32 resultCode = 0;
        if(fit < 0.0 && abs(fit) < domain.epsilon)
            //# Force convergence (BBSKernelProcessControl is already geared
            //# towards the new interface)
            resultCode = 1;
        
        // Send the result to the kernel.
        IterationResult *result = new IterationResult(
            request->getDomainIndex(),
//            domain.solver.isReady(),
//            domain.solver.readyText(),
            resultCode,
            "",
            domain.unknowns,
            rank,
            chiSquared,
            lmFactor);
        
        
        // If we're done with this domain, unregister it.
//        if(domain.solver.isReady() != casa::LSQFit::NONREADY)
        if(resultCode != 0)
        {
            LOG_DEBUG_STR("Unregistering domain " << request->getDomainIndex());
            itsRegisteredDomains.erase(request->getDomainIndex());
        }

        return result;
    }

} // namespace BBS
} // namespace LOFAR
