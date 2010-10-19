//# SolverTest.cc: Test class mimicking the SolverBBS behaviour
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include "SolverTest.h"
#include <MWControl/MWGlobalSpec.h>
#include <MWCommon/MasterControl.h>
#include <MWCommon/MWIos.h>
#include <MWCommon/MWError.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Common/LofarLogger.h>

using namespace std;


namespace LOFAR { namespace CEP {

  SolverTest::SolverTest()
  {}

  SolverTest::~SolverTest()
  {}

  WorkerProxy::ShPtr SolverTest::create()
  {
    return WorkerProxy::ShPtr (new SolverTest());
  }

  void SolverTest::setInitInfo (const ParameterSet&,
				const std::string&)
  {
    MWCOUT << "SolverTest::setInitInfo" << endl;
  }

  int SolverTest::process (int operation, int streamId,
			   LOFAR::BlobIStream& in,
			   LOFAR::BlobOStream& out)
  {
    int resOper = operation;
    MWCOUT << "SolverTest::doProcess" << endl;
    MWCOUT << "  Operation: " << operation << endl;
    MWCOUT << "  StreamId:  " << streamId << endl;
    switch (operation) {
    case MasterControl::SetWd:
    {
      ObsDomain workDomain;
      in >> workDomain;
      MWCOUT << "  Set work domain: " << workDomain << endl;
      break;
    }
    case MasterControl::Step:
    {
      // A step has to be processed.
      // Only a solve can be processed.
      ASSERTSTR (in.getNextType() == "MWGlobalSpec",
		 "SolverTest can only handle an MWGlobalSpec step");
      MWGlobalSpec step;
      // Fill it from the blobstream.
      step.fromBlob (in);
      itsMaxIter = step.getParms().getInt32("Solve.MaxIter");
      itsNrIter  = 0;
      MWCOUT << "  Solve maxiter " << itsMaxIter << endl;
      break;
    }
    case MasterControl::GlobalInit:
    {
      // ParmInfo has to be processed.
      bool result;
      in >> result;
      MWCOUT << "  ParmInfo " << result << endl;
      resOper = -1;     // no reply to be sent
      break;
    }
    case MasterControl::GlobalInfo:
    {
      // Equations have to be processed.
      bool result;
      in >> result;
      MWCOUT << "  GetEq " << result << endl;
      resOper = -1;     // no reply to be sent
      break;
    }
    case MasterControl::GlobalExec:
    {
      MWCOUT << "  Solve iteration: " << itsNrIter << endl;
      ++itsNrIter;
      bool converged = itsNrIter>=itsMaxIter;
      out << converged;
      break;
    }
    default:
      THROW (MWError, "SolverTest::process: operation "
	     << operation << " is unknown");
    }
    return resOper;
  }

}} // end namespaces
