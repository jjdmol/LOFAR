//# MasterControl.cc: Master controller of distributed VDS processing
//#
//# Copyright (c) 2007
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

#include <MWCommon/MasterControl.h>
#include <MWCommon/MWGlobalStep.h>
#include <MWCommon/MWLocalStep.h>
#include <MWCommon/MWBlobIO.h>

using namespace std;


namespace LOFAR { namespace CEP {
    
  std::ostream& operator<<(std::ostream& os, MasterControl::Operation op)
  {
    switch(op) {
      case MasterControl::Init:
        os << string("Init: Initialize");
        break;
      case MasterControl::SetWd:
        os << string("SetWd: Set working domain");
        break;
      case MasterControl::Step:
        os << string("Step: Process a step");
        break;
      case MasterControl::GlobalInit:
        os << string("GlobalInit: Global initial step info");
        break;
      case MasterControl::GlobalInfo:
        os << string("GlobalInfo: Get all global iteration info");
        break;
      case MasterControl::GlobalExec:
        os << string("GlobalExec: Execute global iteration");
        break;
      case MasterControl::EndWd:
        os << string("EndWd: End processing working domain");
        break;
    }
    return os;
  };
  
  MasterControl::MasterControl (const MWConnectionSet::ShPtr& localWorkers,
				const MWConnectionSet::ShPtr& globalWorkers)
    : itsLocalWorkers  (localWorkers),
      itsGlobalWorkers (globalWorkers)
  {}

  MasterControl::~MasterControl()
  {}

  void MasterControl::setInitInfo (const ParameterSet& parms,
				   const vector<string>& dataPartNames,
				   const ObsDomain& fullDomain)
  {
    itsFullDomain = fullDomain;
    // Fill the DataHolder as much as possible.
    LOFAR::BlobString buf;
    int workerId = 0;
    for (int i=0; i<itsLocalWorkers->size(); ++i) {
      buf.resize (0);
      MWBlobOut out(buf, MasterControl::Init, 0, workerId);
      out.blobStream() << parms << dataPartNames[i];
      out.finish();
      itsLocalWorkers->write (i, buf);
      ++workerId;
    }
    for (int i=0; i<itsGlobalWorkers->size(); ++i) {
      buf.resize (0);
      MWBlobOut out(buf, MasterControl::Init, 0, workerId);
      out.blobStream() << parms << "";
      out.finish();
      itsGlobalWorkers->write (i, buf);
      ++workerId;
    }
    // Now read the replies back. They contain no info, but merely show
    // the worker is alive.
    readAllWorkers (true, true);
  }

  void MasterControl::setWorkDomainSpec (const WorkDomainSpec& wds)
  {
    itsWds = wds;
  }

  void MasterControl::processSteps (const MWStep& step)
  {
    // Iterate through the full observation domain.
    ObsDomain workDomain;
    while (itsFullDomain.getNextWorkDomain (workDomain, itsWds.getShape())) {
      // Send WorkDomain to all localWorkers and globalWorker.
      LOFAR::BlobString buf;
      MWBlobOut out(buf, MasterControl::SetWd, 0);
      out.blobStream() << workDomain;
      out.finish();
      itsLocalWorkers->writeAll (buf);
      itsGlobalWorkers->writeAll (buf);
      readAllWorkers (true, true);
      // Iterate through all steps and execute them.
      step.visit (*this);
    }
  }
   
  void MasterControl::quit()
  {
    // Send an end command.
    LOFAR::BlobString buf;
    MWBlobOut out(buf, -1, 0);
    out.finish();
    itsLocalWorkers->writeAll (buf);
    itsGlobalWorkers->writeAll (buf);
  }

  void MasterControl::visitGlobal (const MWGlobalStep& step)
  {
    // Send the global step info to all localWorkers and globalWorker.
    LOFAR::BlobString buf;
    {
      // Write command into buffer.
      MWBlobOut out (buf, MasterControl::Step, 0);
      step.toBlob (out.blobStream());
      out.finish();
    }
    itsLocalWorkers->writeAll (buf);
    itsGlobalWorkers->write (0, buf);
    // Read reply back from globalWorker.
    itsGlobalWorkers->read (0, buf);
    // Read the reply back from each localWorker and send that to the globalWorker.
    for (int i=0; i<itsLocalWorkers->size(); ++i) {
      itsLocalWorkers->read (i, buf);
      itsGlobalWorkers->write (0, buf);
    }
    // Iterate as long as the globalWorker has not converged.
    bool converged = false;
    while (!converged) {
      // Tell localWorkers to form the equations.
      buf.resize (0);
      {
	MWBlobOut out (buf, MasterControl::GlobalInfo, 0);
	out.finish();
      }
      itsLocalWorkers->writeAll (buf);
      // Read the reply back from each localWorker and send that to the globalWorker.
      for (int i=0; i<itsLocalWorkers->size(); ++i) {
	itsLocalWorkers->read (i, buf);
	itsGlobalWorkers->write (0, buf);
      }
      // Tell the globalWorker to do the solve, get the solution and send that
      // to each localWorker.
      buf.resize (0);
      {
	MWBlobOut out (buf, MasterControl::GlobalExec, 0);
	out.finish();
      }
      itsGlobalWorkers->write (0, buf);
      itsGlobalWorkers->read (0, buf);
      itsLocalWorkers->writeAll (buf);
      // Interpret the result to see if we have converged.
      MWBlobIn bin(buf);
      bin.blobStream() >> converged;
    }
  }

  void MasterControl::visitLocal (const MWLocalStep& step)
  {
    LOFAR::BlobString buf;
    MWBlobOut out (buf, MasterControl::Step, 0);
    step.toBlob (out.blobStream());
    out.finish();
    itsLocalWorkers->writeAll (buf);
    readAllWorkers (true, false);
  }

  void MasterControl::readAllWorkers (bool localWorkers, bool globalWorkers)
  {
    LOFAR::BlobString buf;
    if (localWorkers) {
      for (int i=0; i<itsLocalWorkers->size(); ++i) {
        itsLocalWorkers->read (i, buf);
      }    
    }
    if (globalWorkers) {
      for (int i=0; i<itsGlobalWorkers->size(); ++i) {
        itsGlobalWorkers->read (i, buf);
      }    
    }
  }

}} // end namespaces
