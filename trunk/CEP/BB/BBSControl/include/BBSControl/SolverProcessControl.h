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

#include <BBSControl/DomainRegistrationRequest.h>
#include <BBSControl/IterationRequest.h>
#include <BBSControl/IterationResult.h>
#include <BBSControl/BlobStreamableVector.h>
#include <BBSControl/BlobStreamableConnection.h>

#include <Common/lofar_smartptr.h>
#include <PLC/ProcessControl.h>
#include <APS/ParameterSet.h>

#include <scimath/Fitting/LSQFit.h>

namespace LOFAR
{

namespace BBS
{
        // \addtogroup BBSControl
        // @{

        // Implementation of the ProcessControl interface for the BBS solver component.
        class SolverProcessControl: public ACC::PLC::ProcessControl
        {
        public:
            // Default constructor.
            SolverProcessControl();

            // @name Implementation of PLC interface.
            // @{
            virtual tribool define();
            virtual tribool init();
            virtual tribool run();
            virtual tribool pause(const string& condition);
            virtual tribool quit();
            virtual tribool snapshot(const string& destination);
            virtual tribool recover(const string& source);
            virtual tribool reinit(const string& configID);
            virtual string  askInfo(const string& keylist);
            // @}

        private:
            struct Domain
            {
                uint32          index;
                vector<double>  unknowns;
                casa::LSQFit    solver;
                //# this attribute is only temporary, until the problems with
                //# the latest version of LSQFit have been solved.
                double          epsilon;
            };
            
            bool dispatch(const BlobStreamable *message);
            bool handle(const DomainRegistrationRequest *request);
            bool handle(const BlobStreamableVector<DomainRegistrationRequest> *request);
            bool handle(const IterationRequest *request);
            bool handle(const BlobStreamableVector<IterationRequest> *request);

            bool registerDomain(const DomainRegistrationRequest *request);
            IterationResult *performIteration(const IterationRequest *request);
            
            // Parameter set for this process controller.
            ACC::APS::ParameterSet itsParameterSet;

            scoped_ptr<BlobStreamableConnection> itsKernelConnection;
            
            map<uint32, Domain> itsRegisteredDomains;
        };
    // @}
} //# namespace BBS
} //# namespace LOFAR

#endif
