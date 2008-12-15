//#  MWStepTester.cc: 
//#
//#  Copyright (C) 2007
//#
//#  $Id$

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
