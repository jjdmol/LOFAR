//# MWStepTester.cc: Test class for the MWStep framework
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
#include "MWStepTester.h"
#include <MWControl/MWGlobalSpec.h>
#include <MWControl/MWLocalSpec.h>
#include <MWCommon/MasterControl.h>
#include <MWCommon/MWIos.h>
#include <Common/StreamUtil.h>

namespace std {
  using LOFAR::operator<<;
}
using namespace std;


namespace LOFAR { namespace CEP {

  MWStepTester::MWStepTester (int streamId, LOFAR::BlobOStream* out)
    : itsStreamId  (streamId),
      itsOperation (MasterControl::Step),
      itsOut       (out)
  {}

  MWStepTester::~MWStepTester()
  {}

  void MWStepTester::visitGlobal (const MWGlobalStep& step)
  {
    ParameterSet ps = step.getParms();
    MWCOUT << "  MWStepTester::visitGlobal,  streamId " << itsStreamId << endl;
    MWCOUT << "   Max nr. of iterations:  "
	   << ps.getInt32("Solve.MaxIter") << endl;
    MWCOUT << "   Convergence threshold:  "
	   << ps.getDouble("Solve.Epsilon") << endl;
    MWCOUT << "   Min fraction converged: "
	   << ps.getDouble("Solve.MinConverged") << endl;
    MWCOUT << "   Solvable parameters:    "
	   << ps.getStringVector("Solve.Parms") << endl;
    MWCOUT << "   Excluded parameters:    "
	   << ps.getStringVector("Solve.ExclParms") << endl;
    double bandWidth    = ps.getDouble("Solve.DomainSize.Freq");
    double timeInterval = ps.getDouble("Solve.DomainSize.Time");
    MWCOUT << "   Domain shape:           "
	   << DomainShape(bandWidth, timeInterval) << endl;
    itsOperation = MasterControl::GlobalInit;
    *itsOut << true;
  }

  void MWStepTester::visitLocal (const MWLocalStep& step)
  {
    MWCOUT << "  MWStepTester::visitLocal,  streamId "
           << itsStreamId << endl;
    MWCOUT << step.getParms();
    writeResult (true);
  }

  void MWStepTester::writeResult (bool result)
  {
    *itsOut << result;
  }

}} // end namespaces
